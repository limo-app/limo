#include "cryptography.h"
#include <cmath>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rand.h>


void throwError(const std::string& step)
{
  std::string error = "Error during " + step + ".\n";
  auto code = ERR_get_error();
  char buffer[256];
  while(code)
  {
    ERR_error_string(code, buffer);
    error.append(std::string(buffer));
    error.append("\n");
    code = ERR_get_error();
  }
  ERR_free_strings();
  throw CryptographyError(error);
}

namespace cryptography
{
std::tuple<std::string, std::string, std::string> encrypt(const std::string& plain_text,
                                                          const std::string& key)
{
  auto ctx = EVP_CIPHER_CTX_new();
  if(!ctx)
    throwError("encryption");

  if(EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
    throwError("encryption");

  constexpr int nonce_size = 12;
  unsigned char nonce[nonce_size];
  if(RAND_bytes(nonce, nonce_size) != 1)
    throwError("encryption");

  std::string actual_key = key.empty() ? default_key : key;
  constexpr int key_size = 32;
  unsigned char key_padded[key_size];
  for(int i = 0; i < key_size; i++)
    key_padded[i] = actual_key[i % actual_key.size()];
  if(EVP_EncryptInit_ex(ctx, NULL, NULL, key_padded, nonce) != 1)
    throwError("encryption");

  const int buffer_size = exp2((int)(log(plain_text.size() + 16) / log(2)) + 1);
  unsigned char cipher_text[buffer_size];
  int cur_length = 0;
  unsigned char plain_array[plain_text.size()];
  for(int i = 0; i < plain_text.size(); i++)
    plain_array[i] = plain_text[i];
  if(EVP_EncryptUpdate(ctx, cipher_text, &cur_length, plain_array, plain_text.size()) != 1)
    throwError("encryption");

  int cipher_length = cur_length;
  if(EVP_EncryptFinal_ex(ctx, cipher_text + cur_length, &cur_length) != 1)
    throwError("encryption");
  cipher_length += cur_length;

  unsigned char tag[16];
  if(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) != 1)
    throwError("encryption");

  EVP_CIPHER_CTX_free(ctx);

  const std::string cipher_str(reinterpret_cast<const char*>(cipher_text), cipher_length);
  const std::string nonce_str(reinterpret_cast<const char*>(nonce), nonce_size);
  const std::string tag_str(reinterpret_cast<const char*>(tag), 16);

  return { cipher_str, nonce_str, tag_str };
}

std::string decrypt(const std::string& cipher_text,
                    const std::string& key,
                    const std::string& nonce,
                    const std::string& tag)
{
  auto ctx = EVP_CIPHER_CTX_new();
  if(!ctx)
    throwError("decryption");

  if(EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1)
    throwError("decryption");

  std::string actual_key = key.empty() ? default_key : key;
  constexpr int key_size = 32;
  unsigned char key_arr[key_size];
  for(int i = 0; i < key_size; i++)
    key_arr[i] = actual_key[i % actual_key.size()];
  unsigned char nonce_arr[nonce.size()];
  for(int i = 0; i < nonce.size(); i++)
    nonce_arr[i] = nonce[i];
  if(EVP_DecryptInit_ex(ctx, NULL, NULL, key_arr, nonce_arr) != 1)
    throwError("decryption");

  unsigned char cipher_arr[cipher_text.size()];
  for(int i = 0; i < cipher_text.size(); i++)
    cipher_arr[i] = cipher_text[i];
  unsigned char plain_text[(int)exp2((int)(log(cipher_text.size()) / log(2)) + 1)];
  int cur_length = 0;
  if(EVP_DecryptUpdate(ctx, plain_text, &cur_length, cipher_arr, cipher_text.size()) != 1)
    throwError("decryption");
  int plain_text_length = cur_length;

  unsigned char tag_arr[tag.size()];
  for(int i = 0; i < tag.size(); i++)
    tag_arr[i] = tag[i];
  if(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag_arr) != 1)
    throwError("decryption");

  if(EVP_DecryptFinal_ex(ctx, plain_text + cur_length, &cur_length) <= 0)
    throwError("decryption");
  plain_text_length += cur_length;

  return std::string(reinterpret_cast<const char*>(plain_text), plain_text_length);
}
}
