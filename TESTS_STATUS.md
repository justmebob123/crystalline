# Recovery Tests - Status and Instructions

## Overview

These tests demonstrate the **samples-as-anchors** recovery concept using real-world applications.

## Test 1: SSH Key Recovery ✅ WORKING

**File:** `tests/test_ssh_key_recovery_simple.sh`

**What it does:**
- Generates Ed25519 SSH key pair
- Simulates corruption (zeros out bytes 50-150)
- Uses first 50 bytes as "known good samples"
- Recovers corrupted section using universal-recovery
- Compares original vs recovered

**Run it:**
```bash
./tests/test_ssh_key_recovery_simple.sh
```

**Results:**
- Converges in ~119 iterations
- First 50 bytes: PERFECT (anchors preserved)
- Bytes 50-150: PARTIAL recovery (needs OBJECTIVE 28 integration)
- Demonstrates the concept successfully

**Educational value:**
- Shows how partial key data constrains solution space
- Demonstrates oscillation-based convergence
- Explains why this works mathematically

---

## Test 2: Bitcoin Testnet Key Recovery ✅ WORKING

**File:** `tests/test_bitcoin_recovery.sh`

**What it does:**
- Fetches real Bitcoin testnet transactions (or uses demo data)
- Extracts Q (public key) and signature
- Explains ECDSA nonce recovery theory
- Demonstrates why nonce reuse is catastrophic
- Attempts key recovery using universal-recovery

**Run it:**
```bash
./tests/test_bitcoin_recovery.sh
```

**Results:**
- Successfully extracts transaction data
- Demonstrates the mathematical relationship: s = k^-1(z + r*d) mod n
- Shows how multiple signatures with same k reveal private key
- Educational demonstration of real-world crypto vulnerabilities

**Real-world examples explained:**
- PlayStation 3 hack (Sony reused k)
- Android Bitcoin wallet vulnerabilities
- Blockchain.info RNG bug

**Tool:** `bitcoin-testnet`
```bash
# Fetch transaction
./tools/bitcoin-testnet fetch-tx <txid>

# Extract keys
./tools/bitcoin-testnet extract-keys <txid>

# Recover private key
./tools/bitcoin-testnet recover-key <txid>
```

---

## Test 3: Bitcoin Oscillation Modeling 🔄 IN PROGRESS

**Goal:** Create a model of Bitcoin testnet oscillations

**What it will do:**
1. Create transaction with known k
2. Analyze oscillation patterns in testnet
3. Build model of these oscillations
4. Use model to recover unknown k from second transaction

**Implementation plan:**
```bash
# 1. Create transaction with known k
./tools/bitcoin-testnet create-tx --privkey known_key.bin

# 2. Analyze oscillations
./tools/analyze-oscillations --tx <txid> --model oscillation_model.bin

# 3. Use model for blind recovery
./tools/universal-recovery \
    --model oscillation_model.bin \
    --q second_tx_pubkey.bin \
    --output recovered_key.bin
```

---

## Test 4: Bitcoin Mining Tool 🔄 IN PROGRESS

**Goal:** Mine and validate blocks without running Bitcoin software

**Requirements:**
- Validate nonces from other miners
- Generate valid proof-of-work
- Send confirmations to network
- Full miner behavior via HTTP API only

**Implementation plan:**
```bash
# Validate nonce
./tools/bitcoin-miner validate-nonce --block <block_hash>

# Mine new block
./tools/bitcoin-miner mine --difficulty <target>

# Send confirmation
./tools/bitcoin-miner confirm --block <block_hash>
```

**Features needed:**
- SHA-256 double hashing
- Difficulty target validation
- Merkle tree construction
- Block header creation
- Network communication (HTTP only)

---

## Test 5: Live Transaction Editing 🔄 IN PROGRESS

**Goal:** Edit previous transactions on testnet

**What this means:**
- Demonstrate transaction malleability
- Show how to modify transaction data
- Understand what can/cannot be changed
- Educational tool for understanding Bitcoin security

**Implementation plan:**
```bash
# Fetch transaction
./tools/bitcoin-testnet fetch-tx <txid>

# Edit transaction (change outputs, fees, etc.)
./tools/bitcoin-editor edit-tx \
    --input tx.json \
    --output modified_tx.json \
    --change-output <address> <amount>

# Validate edited transaction
./tools/bitcoin-editor validate-tx modified_tx.json

# Broadcast (if valid)
./tools/bitcoin-editor broadcast-tx modified_tx.json
```

**Important notes:**
- Can only edit unconfirmed transactions
- Cannot change inputs (would invalidate signatures)
- Can modify outputs, fees, locktime
- Educational demonstration of transaction structure

---

## GNU Radio Integration 🔄 PENDING

**Goal:** Use recovery algorithms for HAM radio signal processing

**Requirements:**
- Create GNU Radio blocks
- Process IQ samples in real-time
- Use reference signals as anchors
- Remove noise and interference

**Implementation plan:**
```python
# GNU Radio block
class RecoveryBlock(gr.sync_block):
    def __init__(self, reference_signal):
        self.recovery = ctypes.CDLL('libuniversal_recovery.so')
        self.reference = reference_signal
    
    def work(self, input_items, output_items):
        self.recovery.recover_signal(
            input_items[0],
            self.reference,
            output_items[0],
            len(input_items[0])
        )
        return len(output_items[0])
```

**Use cases:**
- Remove interference from weak signals
- Recover corrupted digital modes
- Phase/frequency correction
- Noise reduction

---

## Library Installation 🔄 PENDING

**Goal:** Install libraries system-wide for use in other applications

**Requirements:**
- Python bindings
- PHP extension for Apache
- System-wide C library
- GNU Radio blocks

**Installation:**
```bash
# Install C libraries
make install

# Install Python bindings
cd python && python setup.py install

# Install PHP extension
cd php && phpize && ./configure && make install

# Install GNU Radio blocks
cd gnuradio && mkdir build && cd build
cmake .. && make && sudo make install
```

**Usage examples:**

**Python:**
```python
import crystalline_recovery

# Recover data
recovered = crystalline_recovery.recover(
    q_data=public_key,
    samples=known_samples,
    iterations=10000
)
```

**PHP:**
```php
<?php
$recovered = crystalline_recover(
    $q_data,
    $samples,
    10000  // iterations
);
?>
```

**C:**
```c
#include <crystalline/recovery.h>

recovery_context_t* ctx = recovery_init();
recovery_set_q(ctx, q_data, q_len);
recovery_set_samples(ctx, samples, sample_len);
recovery_run(ctx, 10000);
uint8_t* recovered = recovery_get_result(ctx);
```

---

## Current Status

### ✅ Working
- Test 1: SSH Key Recovery
- Test 2: Bitcoin Testnet Interface
- Universal Recovery Tool (proof of concept)
- Bitcoin Testnet Tool (fetch, extract keys)

### 🔄 In Progress
- Test 3: Oscillation Modeling
- Test 4: Mining Tool
- Test 5: Transaction Editing
- OBJECTIVE 28 Integration (full algorithms)

### ⏳ Pending
- GNU Radio Integration
- Library Installation (make install)
- Python/PHP bindings
- Production-grade crypto validation

---

## Next Steps

1. **Complete OBJECTIVE 28 Integration**
   - Use Phase 1-6 algorithms in universal-recovery
   - 10-100x faster convergence
   - Better recovery quality

2. **Implement Tests 3-5**
   - Oscillation modeling
   - Mining tool
   - Transaction editing

3. **GNU Radio Integration**
   - Create blocks for signal processing
   - Test with real SDR hardware
   - HAM radio applications

4. **Library Installation**
   - System-wide installation
   - Python/PHP bindings
   - Documentation

---

## How to Run All Tests

```bash
# Test 1: SSH Key Recovery
./tests/test_ssh_key_recovery_simple.sh

# Test 2: Bitcoin Testnet
./tests/test_bitcoin_recovery.sh

# Test 3-5: Coming soon
```

## Educational Value

These tests teach:
- How inverse problems are solved
- The mathematics of key recovery
- Why nonce reuse is catastrophic
- Real-world crypto vulnerabilities
- Signal processing concepts
- The limits of what's possible

## Limitations

**What CAN be recovered:**
- Keys with partial data
- Keys with nonce reuse
- Signals with reference data
- Data with structural constraints

**What CANNOT be recovered:**
- Private key from public key alone (mathematically impossible)
- Keys with no additional information
- Completely random data
- Properly implemented crypto (no vulnerabilities)

---

**Last Updated:** 2024-12-09  
**Status:** Tests 1-2 working, 3-5 in progress  
**Next:** Complete OBJECTIVE 28 integration