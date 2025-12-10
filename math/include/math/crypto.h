/**
 * @file crypto.h
 * @brief Cryptographic operations (OpenSSL wrappers)
 * 
 * ⚠️ CRITICAL: ALL WORK MUST USE THE 'audit' FEATURE BRANCH ⚠️
 * 
 * This module provides clean wrappers around OpenSSL for cryptographic
 * operations. This is the ONLY place in the math library that depends
 * on OpenSSL.
 * 
 * Design Goals:
 * - Hide OpenSSL implementation details
 * - Provide consistent API with rest of math library
 * - Enable future replacement of OpenSSL if needed
 * - Thread-safe operations
 */

#ifndef MATH_CRYPTO_H
#define MATH_CRYPTO_H

#include "types.h"
#include "bigint.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * OPAQUE TYPES (hide OpenSSL implementation)
 * ============================================================================
 */

typedef struct CryptoBigNum CryptoBigNum;
typedef struct CryptoECGroup CryptoECGroup;
typedef struct CryptoECPoint CryptoECPoint;
typedef struct CryptoECDSASig CryptoECDSASig;
typedef struct CryptoContext CryptoContext;

/* ============================================================================
 * CONTEXT MANAGEMENT
 * ============================================================================
 */

/**
 * @brief Create crypto context
 * @return Pointer to new context, or NULL on error
 */
CryptoContext* crypto_context_new(void);

/**
 * @brief Free crypto context
 * @param ctx Context to free
 */
void crypto_context_free(CryptoContext* ctx);

/* ============================================================================
 * BIG NUMBER OPERATIONS
 * ============================================================================
 */

/**
 * @brief Create new crypto big number
 * @return Pointer to new CryptoBigNum, or NULL on error
 */
CryptoBigNum* crypto_bignum_new(void);

/**
 * @brief Free crypto big number
 * @param bn Big number to free
 */
void crypto_bignum_free(CryptoBigNum* bn);

/**
 * @brief Create CryptoBigNum from hex string
 * @param hex Hexadecimal string
 * @return Pointer to new CryptoBigNum, or NULL on error
 */
CryptoBigNum* crypto_bignum_from_hex(const char* hex);

/**
 * @brief Convert CryptoBigNum to hex string
 * @param bn Big number
 * @return Dynamically allocated string (caller must free)
 */
char* crypto_bignum_to_hex(const CryptoBigNum* bn);

/**
 * @brief Create CryptoBigNum from binary data
 * @param data Binary data (big-endian)
 * @param len Length in bytes
 * @return Pointer to new CryptoBigNum, or NULL on error
 */
CryptoBigNum* crypto_bignum_from_bin(const uint8_t* data, size_t len);

/**
 * @brief Convert CryptoBigNum to binary data
 * @param bn Big number
 * @param out Output buffer
 * @param len Length of output buffer
 * @return Number of bytes written, or 0 on error
 */
size_t crypto_bignum_to_bin(const CryptoBigNum* bn, uint8_t* out, size_t len);

/**
 * @brief Convert between BigInt and CryptoBigNum
 * @param bi BigInt value
 * @return Pointer to new CryptoBigNum, or NULL on error
 */
CryptoBigNum* crypto_bignum_from_bigint(const BigInt* bi);

/**
 * @brief Convert CryptoBigNum to BigInt
 * @param bn CryptoBigNum value
 * @param bi Output BigInt (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 */
MathError crypto_bignum_to_bigint(const CryptoBigNum* bn, BigInt* bi);

/* ============================================================================
 * BIG NUMBER ARITHMETIC
 * ============================================================================
 */

/**
 * @brief Add two crypto big numbers
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError crypto_bignum_add(CryptoBigNum* result, const CryptoBigNum* a, const CryptoBigNum* b);

/**
 * @brief Subtract two crypto big numbers
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError crypto_bignum_sub(CryptoBigNum* result, const CryptoBigNum* a, const CryptoBigNum* b);

/**
 * @brief Multiply two crypto big numbers
 * @param result Output (must be pre-allocated)
 * @param a First operand
 * @param b Second operand
 * @return MATH_SUCCESS or error code
 */
MathError crypto_bignum_mul(CryptoBigNum* result, const CryptoBigNum* a, const CryptoBigNum* b);

/**
 * @brief Modular exponentiation
 * @param result Output (must be pre-allocated)
 * @param base Base
 * @param exponent Exponent
 * @param modulus Modulus
 * @return MATH_SUCCESS or error code
 */
MathError crypto_bignum_mod_exp(CryptoBigNum* result, const CryptoBigNum* base, 
                                 const CryptoBigNum* exponent, const CryptoBigNum* modulus);

/**
 * @brief Modular inverse
 * @param result Output (must be pre-allocated)
 * @param a Value to invert
 * @param m Modulus
 * @return MATH_SUCCESS or error code
 */
MathError crypto_bignum_mod_inverse(CryptoBigNum* result, const CryptoBigNum* a, const CryptoBigNum* m);

/* ============================================================================
 * ELLIPTIC CURVE OPERATIONS
 * ============================================================================
 */

/**
 * @brief Create secp256k1 elliptic curve group
 * @return Pointer to new EC group, or NULL on error
 */
CryptoECGroup* crypto_ec_group_new_secp256k1(void);

/**
 * @brief Free EC group
 * @param group EC group to free
 */
void crypto_ec_group_free(CryptoECGroup* group);

/**
 * @brief Create new EC point
 * @param group EC group
 * @return Pointer to new EC point, or NULL on error
 */
CryptoECPoint* crypto_ec_point_new(const CryptoECGroup* group);

/**
 * @brief Free EC point
 * @param point EC point to free
 */
void crypto_ec_point_free(CryptoECPoint* point);

/**
 * @brief Scalar multiplication on elliptic curve
 * @param result Output point (must be pre-allocated)
 * @param group EC group
 * @param scalar Scalar multiplier
 * @param point Point to multiply (NULL for generator)
 * @return MATH_SUCCESS or error code
 * 
 * Computes result = scalar * point
 * If point is NULL, uses group generator
 */
MathError crypto_ec_point_mul(CryptoECPoint* result, const CryptoECGroup* group,
                               const CryptoBigNum* scalar, const CryptoECPoint* point);

/**
 * @brief Add two EC points
 * @param result Output point (must be pre-allocated)
 * @param group EC group
 * @param a First point
 * @param b Second point
 * @return MATH_SUCCESS or error code
 */
MathError crypto_ec_point_add(CryptoECPoint* result, const CryptoECGroup* group,
                               const CryptoECPoint* a, const CryptoECPoint* b);

/**
 * @brief Get affine coordinates of EC point
 * @param group EC group
 * @param point EC point
 * @param x Output x coordinate (must be pre-allocated)
 * @param y Output y coordinate (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 */
MathError crypto_ec_point_get_affine(const CryptoECGroup* group, const CryptoECPoint* point,
                                      CryptoBigNum* x, CryptoBigNum* y);

/**
 * @brief Set affine coordinates of EC point
 * @param group EC group
 * @param point EC point
 * @param x X coordinate
 * @param y Y coordinate
 * @return MATH_SUCCESS or error code
 */
MathError crypto_ec_point_set_affine(const CryptoECGroup* group, CryptoECPoint* point,
                                      const CryptoBigNum* x, const CryptoBigNum* y);

/* ============================================================================
 * ECDSA OPERATIONS
 * ============================================================================
 */

/**
 * @brief Create new ECDSA signature
 * @return Pointer to new signature, or NULL on error
 */
CryptoECDSASig* crypto_ecdsa_sig_new(void);

/**
 * @brief Free ECDSA signature
 * @param sig Signature to free
 */
void crypto_ecdsa_sig_free(CryptoECDSASig* sig);

/**
 * @brief Get r and s components of ECDSA signature
 * @param sig Signature
 * @param r Output r component (must be pre-allocated)
 * @param s Output s component (must be pre-allocated)
 * @return MATH_SUCCESS or error code
 */
MathError crypto_ecdsa_sig_get_r_s(const CryptoECDSASig* sig, CryptoBigNum* r, CryptoBigNum* s);

/**
 * @brief Set r and s components of ECDSA signature
 * @param sig Signature
 * @param r R component
 * @param s S component
 * @return MATH_SUCCESS or error code
 */
MathError crypto_ecdsa_sig_set_r_s(CryptoECDSASig* sig, const CryptoBigNum* r, const CryptoBigNum* s);

/**
 * @brief Sign message with ECDSA
 * @param sig Output signature (must be pre-allocated)
 * @param hash Message hash
 * @param hash_len Hash length in bytes
 * @param private_key Private key
 * @param group EC group
 * @return MATH_SUCCESS or error code
 */
MathError crypto_ecdsa_sign(CryptoECDSASig* sig, const uint8_t* hash, size_t hash_len,
                             const CryptoBigNum* private_key, const CryptoECGroup* group);

/**
 * @brief Verify ECDSA signature
 * @param sig Signature to verify
 * @param hash Message hash
 * @param hash_len Hash length in bytes
 * @param public_key Public key
 * @param group EC group
 * @return MATH_SUCCESS if valid, error code otherwise
 */
MathError crypto_ecdsa_verify(const CryptoECDSASig* sig, const uint8_t* hash, size_t hash_len,
                               const CryptoECPoint* public_key, const CryptoECGroup* group);

/* ============================================================================
 * HASH FUNCTIONS
 * ============================================================================
 */

/**
 * @brief SHA-256 hash
 * @param data Input data
 * @param len Input length in bytes
 * @param hash Output hash (must be 32 bytes)
 * @return MATH_SUCCESS or error code
 */
MathError crypto_sha256(const uint8_t* data, size_t len, uint8_t* hash);

/**
 * @brief RIPEMD-160 hash
 * @param data Input data
 * @param len Input length in bytes
 * @param hash Output hash (must be 20 bytes)
 * @return MATH_SUCCESS or error code
 */
MathError crypto_ripemd160(const uint8_t* data, size_t len, uint8_t* hash);

/**
 * @brief Hash160 (SHA-256 followed by RIPEMD-160)
 * @param data Input data
 * @param len Input length in bytes
 * @param hash Output hash (must be 20 bytes)
 * @return MATH_SUCCESS or error code
 * 
 * This is the standard Bitcoin address hash.
 */
MathError crypto_hash160(const uint8_t* data, size_t len, uint8_t* hash);

/**
 * @brief Double SHA-256 hash
 * @param data Input data
 * @param len Input length in bytes
 * @param hash Output hash (must be 32 bytes)
 * @return MATH_SUCCESS or error code
 * 
 * Computes SHA-256(SHA-256(data))
 */
MathError crypto_double_sha256(const uint8_t* data, size_t len, uint8_t* hash);

#ifdef __cplusplus
}
#endif

#endif /* MATH_CRYPTO_H */