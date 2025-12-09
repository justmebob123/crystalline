/**
 * @file recovery_network.h
 * @brief Bitcoin Network Interface Library
 * 
 * Support for ALL Bitcoin networks:
 * - Bitcoin mainnet
 * - Bitcoin testnet
 * - Bitcoin regtest
 * - Custom altcoins
 * - Private networks
 * 
 * NO Bitcoin software required - pure HTTP/RPC interface
 */

#ifndef RECOVERY_NETWORK_H
#define RECOVERY_NETWORK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Network types
typedef enum {
    NETWORK_BITCOIN_MAINNET,
    NETWORK_BITCOIN_TESTNET,
    NETWORK_BITCOIN_REGTEST,
    NETWORK_CUSTOM
} network_type_t;

// Network configuration
typedef struct {
    network_type_t type;
    char* rpc_url;           // RPC endpoint (NULL for default)
    char* rpc_user;          // RPC username (NULL if not needed)
    char* rpc_pass;          // RPC password (NULL if not needed)
    int timeout_seconds;     // Request timeout
} network_config_t;

// Network context (opaque)
typedef struct bitcoin_network bitcoin_network_t;

// Transaction
typedef struct {
    char* txid;
    uint8_t* raw_data;
    size_t raw_len;
    void* parsed;  // Internal parsed structure
} transaction_t;

// Block
typedef struct {
    char* hash;
    uint32_t height;
    uint32_t nonce;
    uint8_t* raw_data;
    size_t raw_len;
    void* parsed;  // Internal parsed structure
} block_t;

/**
 * Connect to Bitcoin network
 * 
 * @param config Network configuration
 * @return Network context or NULL on error
 */
bitcoin_network_t* bitcoin_connect(const network_config_t* config);

/**
 * Fetch transaction by ID
 * 
 * @param net Network context
 * @param txid Transaction ID (hex string)
 * @return Transaction or NULL on error
 */
transaction_t* bitcoin_fetch_transaction(bitcoin_network_t* net, const char* txid);

/**
 * Fetch block by hash
 * 
 * @param net Network context
 * @param hash Block hash (hex string)
 * @return Block or NULL on error
 */
block_t* bitcoin_fetch_block(bitcoin_network_t* net, const char* hash);

/**
 * Fetch block by height
 * 
 * @param net Network context
 * @param height Block height
 * @return Block or NULL on error
 */
block_t* bitcoin_fetch_block_by_height(bitcoin_network_t* net, uint32_t height);

/**
 * Broadcast transaction
 * 
 * @param net Network context
 * @param tx Transaction to broadcast
 * @return true on success
 */
bool bitcoin_broadcast_transaction(bitcoin_network_t* net, transaction_t* tx);

/**
 * Create new transaction
 * 
 * @param net Network context
 * @param inputs Input UTXOs
 * @param num_inputs Number of inputs
 * @param outputs Output addresses and amounts
 * @param num_outputs Number of outputs
 * @return Transaction or NULL on error
 */
transaction_t* bitcoin_create_transaction(
    bitcoin_network_t* net,
    const char** inputs,
    size_t num_inputs,
    const char** outputs,
    size_t num_outputs
);

/**
 * Sign transaction
 * 
 * @param tx Transaction to sign
 * @param privkey Private key (32 bytes for ECDSA)
 * @return true on success
 */
bool bitcoin_sign_transaction(transaction_t* tx, const uint8_t* privkey);

/**
 * Validate block nonce
 * 
 * @param block Block to validate
 * @return true if nonce is valid
 */
bool bitcoin_validate_nonce(const block_t* block);

/**
 * Mine block (find valid nonce)
 * 
 * @param block Block to mine (nonce will be updated)
 * @param max_iterations Maximum iterations (0 = unlimited)
 * @return true if valid nonce found
 */
bool bitcoin_mine_block(block_t* block, uint64_t max_iterations);

/**
 * Get current difficulty
 * 
 * @param net Network context
 * @return Current difficulty target
 */
uint32_t bitcoin_get_difficulty(bitcoin_network_t* net);

/**
 * Get block height
 * 
 * @param net Network context
 * @return Current block height
 */
uint32_t bitcoin_get_block_height(bitcoin_network_t* net);

/**
 * Free transaction
 */
void bitcoin_free_transaction(transaction_t* tx);

/**
 * Free block
 */
void bitcoin_free_block(block_t* block);

/**
 * Disconnect from network
 */
void bitcoin_disconnect(bitcoin_network_t* net);

#ifdef __cplusplus
}
#endif

#endif // RECOVERY_NETWORK_H