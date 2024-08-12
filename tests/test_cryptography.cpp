#include "../src/core/cryptography.h"
#include "test_utils.h"
#include <catch2/catch_test_macros.hpp>
#include <random>
#include <string>


std::string generateRandomString(std::default_random_engine& e)
{
  std::uniform_int_distribution<int> length_dist(1, 100);
  std::uniform_int_distribution<char> char_dist(0, 255);
  std::string str;
  const int str_len = length_dist(e);
  for(int j = 0; j < str_len; j++)
    str += char_dist(e);
  return str;
}

TEST_CASE("String are encrypted", "[.crypto]")
{
  std::random_device r;
  std::default_random_engine e(r());
  std::vector<std::pair<std::string, std::string>> text_key_pairs{ { "this is a super secret text",
                                                                     "some key" } };

  for(int i = 0; i < 10; i++)
    text_key_pairs.emplace_back(generateRandomString(e), generateRandomString(e));

  for(const auto& [plain_text, key] : text_key_pairs)
  {
    const auto [cipher, nonce, tag] = cryptography::encrypt(plain_text, key);
    REQUIRE(!cipher.empty());
    REQUIRE(!nonce.empty());
    REQUIRE(!tag.empty());
    REQUIRE(cipher != plain_text);
    std::string decrypted_text = cryptography::decrypt(cipher, key, nonce, tag);
    REQUIRE(decrypted_text == plain_text);
  }

  const std::string key = "my key";
  const std::string plain_text = "some text";
  const auto [cipher, nonce, tag] = cryptography::encrypt(plain_text, key);
  REQUIRE_THROWS_AS(cryptography::decrypt(cipher + "a", key, nonce, tag), CryptographyError);
  REQUIRE_THROWS_AS(cryptography::decrypt(cipher, key + "a", nonce, tag), CryptographyError);
  REQUIRE_THROWS_AS(cryptography::decrypt(cipher, key, nonce == "a" ? "b" : "a", tag),
                    CryptographyError);
  REQUIRE_THROWS_AS(cryptography::decrypt(cipher, key, nonce, tag == "a" ? "b" : "a"),
                    CryptographyError);
}
