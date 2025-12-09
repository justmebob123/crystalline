/**
 * @file recovery_crypto.h
 * @brief Cryptographic Recovery Library
 * 
 * Specialized recovery for cryptographic data:
 * - ECDSA key recovery (Bitcoin, Ethereum, etc.)
 * - SSH key recovery (Ed25519, RSA)
 * - Nonce recovery from signatures
 * - Partial key reconstruction
 */

#ifndef RECOVERY_CRYPTO_H
#define RECOVERY_CRYPTO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Crypto types
typedef enum {
    CRYPTO_TYPE_ECDSA_SECP256K1,  // Bitcoin, Ethereum
    CRYPTO_TYPE_ECDSA_SECP256R1,  // NIST P-256
    CRYPTO_TYPE_ED25519,          // Modern SSH
    CRYPTO_TYPE_RSA_2048,         // Traditional SSH
    CRYPTO_TYPE_RSA_4096,
    CRYPTO_TYPE_UNKNOWN
} crypto_type_t;

// ECDSA signature
typedef struct {
    uint8_t r[32];
    uint8_t s[32];
    uint8_t v;  // Recovery ID
} ecdsa_signature_t;

// Key pair
typedef struct {
    uint8_t* private_key;
    size_t private_len;
    uint8_t* public_key;
    size_t public_len;
    crypto_type_t type;
} crypto_keypair_t;

/**
 * Detect crypto type from data
 */
crypto_type_t crypto_detect_type(const uint8_t* data, size_t len);

/**
 * Recover ECDSA private key from signatures with reused nonce
 * 
 * @param sig1 First signature
 * @param sig2 Second signature (same k)
 * @param msg1 First message hash
 * @param msg2 Second message hash
 * @param privkey Output private key (32 bytes)
 * @return true on success
 */
bool ecdsa_recover_key_from_nonce_reuse(
    const ecdsa_signature_t* sig1,
    const ecdsa_signature_t* sig2,
    const uint8_t* msg1,
    const uint8_t* msg2,
    uint8_t* privkey
);

/**
 * Recover ECDSA nonce from signature and private key
 */
bool ecdsa_recover_nonce(
    const ecdsa_signature_t* sig,
    const uint8_t* msg,
    const uint8_t* privkey,
    uint8_t* nonce
);

/**
 * Recover partial private key using samples as anchors
 */
bool crypto_recover_partial_key(
    const uint8_t* public_key,
    size_t pubkey_len,
    const uint8_t* partial_key,
    size_t partial_len,
    crypto_type_t type,
    uint8_t* recovered_key,
    size_t* recovered_len
);

/**
 * Validate recovered key against public key
 */
bool crypto_validate_keypair(
    const uint8_t* private_key,
    size_t privkey_len,
    const uint8_t* public_key,
    size_t pubkey_len,
    crypto_type_t type
);

/**
 * Free keypair
 */
void crypto_free_keypair(crypto_keypair_t* keypair);

#ifdef __cplusplus
}
#endif

#endif // RECOVERY_CRYPTO_H