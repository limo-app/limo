/*!
 * \file cryptography.h
 * \brief Header for the cryptography namespace.
 */

#pragma once

#include <stdexcept>
#include <string>


/*!
 * \brief Exception indicating an error during a cryptographic operation.
 */
class CryptographyError : public std::runtime_error
{
public:
  /*!
   * \brief Constructor.
   * \param message Message for the exception.
   */
  CryptographyError(const char* message) : std::runtime_error(message) {}
  /*!
   * \brief Constructor.
   * \param message Message for the exception.
   */
  CryptographyError(const std::string& message) : std::runtime_error(message) {}
};


namespace cryptography
{
/*!
 * \brief Encrypts the given string using AES-GCM with the given key.
 * \param plain_text Text to be encrapted.
 * \param key Key to use for encryption.
 * \return The cipher text, the random nonce(IV) used, the authentication tag.
 * \throws CryptographyError When an OpenSSL internal error occurs.
 */
std::tuple<std::string, std::string, std::string> encrypt(const std::string& plain_text,
                                                          const std::string& key);
/*!
 * \brief Decrypts the given cipher text using AES-GCM.
 * \param cipher_text Text to be decrypted.
 * \param key Key used for decryption.
 * \param nonce Nonce (IV) used during enryption.
 * \param tag Authentication tag.
 * \return The plain text.
 * \throws CryptographyError When an OpenSSL internal error occurs.
 */
std::string decrypt(const std::string& cipher_text,
                    const std::string& key,
                    const std::string& nonce,
                    const std::string& tag);

/*! \brief A default encryption key used in case no key was specified. */
constexpr char default_key[] = "rWnYJVdtxz8Iu62GSJy0OPlOat7imMb8";
};
