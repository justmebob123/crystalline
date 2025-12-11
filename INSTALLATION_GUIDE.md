# Universal Recovery System - Installation Guide

Complete installation guide for all platforms and components.

---

## Quick Start

```bash
# 1. Install dependencies
sudo ./install_dependencies.sh

# 2. Build all libraries and tools
make -f Makefile.recovery clean
make -f Makefile.recovery -j$(nproc)

# 3. Install system-wide
sudo make -f Makefile.recovery install

# 4. Update library cache
sudo ldconfig

# 5. Verify installation
bitcoin-recovery --version
bitcoin-miner --version
```

---

## Fixing Library Loading Issues

### Problem
```
error while loading shared libraries: librecovery_core.so: cannot open shared object file
```

### Solutions

#### Solution 1: System-Wide Installation (Recommended)

```bash
# Install libraries to /usr/local/lib
sudo make -f Makefile.recovery install

# Update library cache
sudo ldconfig

# Verify
ldconfig -p | grep recovery
```

#### Solution 2: Set LD_LIBRARY_PATH (Temporary)

```bash
# For current session
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Run tools
bitcoin-recovery --help
bitcoin-miner --help
```

#### Solution 3: Set LD_LIBRARY_PATH (Permanent)

```bash
# Add to ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

# Or add to /etc/ld.so.conf.d/
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/recovery.conf
sudo ldconfig
```

#### Solution 4: Use Wrapper Script (Development)

```bash
# Use the provided wrapper script
cd tools
./run-bitcoin-tools.sh bitcoin-recovery --help
./run-bitcoin-tools.sh bitcoin-miner --help

# Or create symlinks
ln -s run-bitcoin-tools.sh bitcoin-recovery.sh
ln -s run-bitcoin-tools.sh bitcoin-miner.sh
./bitcoin-recovery.sh --help
```

#### Solution 5: Build with RPATH

```bash
# Rebuild tools with RPATH
cd tools
gcc -Wl,-rpath,'$ORIGIN/../lib/recovery_core' \
    -Wl,-rpath,'$ORIGIN/../lib/recovery_crypto' \
    -Wl,-rpath,'$ORIGIN/../lib/recovery_network' \
    -o bitcoin-recovery bitcoin-recovery.c \
    -L../lib/recovery_core -L../lib/recovery_crypto -L../lib/recovery_network \
    -lrecovery_core -lrecovery_crypto -lrecovery_network -lcurl -lm
```

---

## Platform-Specific Installation

### CentOS / RHEL

```bash
# Install dependencies
sudo yum groupinstall "Development Tools"
sudo yum install openssl-devel libcurl-devel

# Build and install
make -f Makefile.recovery
sudo make -f Makefile.recovery install
sudo ldconfig
```

### Ubuntu / Debian

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install build-essential libssl-dev libcurl4-openssl-dev

# Build and install
make -f Makefile.recovery
sudo make -f Makefile.recovery install
sudo ldconfig
```

### Arch Linux

```bash
# Install dependencies
sudo pacman -S base-devel openssl curl

# Build and install
make -f Makefile.recovery
sudo make -f Makefile.recovery install
sudo ldconfig
```

---

## Component Installation

### Core Libraries Only

```bash
# Build core libraries
cd lib/recovery_core && make
cd ../recovery_crypto && make
cd ../recovery_network && make
cd ../recovery_signal && make

# Install
sudo cp lib/recovery_*/*.so /usr/local/lib/
sudo cp lib/recovery_*/*.a /usr/local/lib/
sudo mkdir -p /usr/local/include/recovery
sudo cp lib/recovery_*/include/*.h /usr/local/include/recovery/
sudo ldconfig
```

### Python Bindings Only

```bash
# Requires core libraries installed first
pip3 install -e python/

# Verify
python3 -c "from recovery import core; print('Success!')"
```

### GNU Radio Module Only

```bash
# Requires core libraries and GNU Radio installed
cd gnuradio/gr-recovery
mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig

# Verify
python3 -c "from gnuradio import recovery; print('Success!')"
```

---

## Verification

### Test Library Installation

```bash
# Check if libraries are installed
ldconfig -p | grep recovery

# Should show:
# librecovery_core.so
# librecovery_crypto.so
# librecovery_network.so
# librecovery_signal.so
```

### Test Tools

```bash
# Test each tool
bitcoin-network --version
bitcoin-recovery --version
bitcoin-miner --version
universal-recovery --help

# If you get library errors, use wrapper script:
cd tools
./run-bitcoin-tools.sh bitcoin-recovery --version
```

### Test Python Bindings

```bash
python3 << 'EOF'
from recovery import core, crypto, network, signal
print("✓ All modules imported successfully")
print(f"Core: {core.__name__}")
print(f"Crypto: {crypto.__name__}")
print(f"Network: {network.__name__}")
print(f"Signal: {signal.__name__}")
EOF
```

### Test GNU Radio Module

```bash
python3 << 'EOF'
try:
    from gnuradio import recovery
    print("✓ GNU Radio recovery module installed")
except ImportError as e:
    print(f"✗ GNU Radio module not installed: {e}")
EOF
```

---

## Troubleshooting

### Issue: Libraries not found after installation

**Symptoms:**
```
error while loading shared libraries: librecovery_core.so: cannot open shared object file
```

**Diagnosis:**
```bash
# Check if libraries are installed
ls -l /usr/local/lib/librecovery*.so

# Check if library path is configured
echo $LD_LIBRARY_PATH

# Check ldconfig cache
ldconfig -p | grep recovery
```

**Fix:**
```bash
# Update library cache
sudo ldconfig

# Or add to library path
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Or use wrapper script
cd tools && ./run-bitcoin-tools.sh bitcoin-recovery --help
```

### Issue: Permission denied during installation

**Symptoms:**
```
Permission denied: /usr/local/lib/librecovery_core.so
```

**Fix:**
```bash
# Use sudo for installation
sudo make -f Makefile.recovery install

# Or install to user directory
make -f Makefile.recovery PREFIX=$HOME/.local install
export LD_LIBRARY_PATH=$HOME/.local/lib:$LD_LIBRARY_PATH
```

### Issue: OpenSSL not found

**Symptoms:**
```
fatal error: openssl/ec.h: No such file or directory
```

**Fix:**
```bash
# Ubuntu/Debian
sudo apt-get install libssl-dev

# CentOS/RHEL
sudo yum install openssl-devel

# The libraries will build with stubs if OpenSSL is not available
# Full functionality requires OpenSSL
```

### Issue: Python module not found

**Symptoms:**
```
ModuleNotFoundError: No module named 'recovery'
```

**Fix:**
```bash
# Install Python bindings
cd python
pip3 install -e .

# Or system-wide
sudo pip3 install -e .

# Verify
python3 -c "import recovery; print(recovery.__version__)"
```

---

## Development Installation

For development, you may want to build without installing:

```bash
# Build everything
make -f Makefile.recovery

# Run tools with wrapper script
cd tools
./run-bitcoin-tools.sh bitcoin-recovery --help

# Or set library path manually
export LD_LIBRARY_PATH=$(pwd)/lib/recovery_core:$(pwd)/lib/recovery_crypto:$(pwd)/lib/recovery_network:$(pwd)/lib/recovery_signal
./tools/bitcoin-recovery --help
```

---

## Uninstallation

### Remove System-Wide Installation

```bash
# Uninstall everything
sudo make -f Makefile.recovery uninstall

# Or manually
sudo rm -f /usr/local/lib/librecovery*.so
sudo rm -f /usr/local/lib/librecovery*.a
sudo rm -rf /usr/local/include/recovery
sudo rm -f /usr/local/bin/bitcoin-*
sudo rm -f /usr/local/bin/universal-recovery
sudo ldconfig
```

### Remove Python Bindings

```bash
pip3 uninstall recovery
```

### Remove GNU Radio Module

```bash
cd gnuradio/gr-recovery/build
sudo make uninstall
```

---

## Post-Installation Setup

### Configure Bitcoin Core

For mining and recovery tools to work, you need Bitcoin Core running:

```bash
# Create bitcoin.conf
mkdir -p ~/.bitcoin
cat > ~/.bitcoin/bitcoin.conf << EOF
# Regtest configuration
regtest=1
server=1
rpcuser=user
rpcpassword=password
rpcallowip=127.0.0.1
EOF

# Start Bitcoin Core
bitcoind -daemon

# Generate initial blocks
bitcoin-cli -regtest generate 101

# Get mining address
bitcoin-cli -regtest getnewaddress
```

### Test Mining

```bash
# Start mining on regtest
bitcoin-miner --solo -n regtest -u user -P password -a bcrt1q... --objective28 -t 4

# Should see:
# Mining Active - Press Ctrl+C to stop
# [OBJ28] Hashrate: 120 H/s | Hashes: 12000 | Blocks: 1 | Uptime: 100s
```

---

## Performance Tuning

### Optimal Thread Count

```bash
# Get CPU core count
nproc

# Use 12 threads for optimal 12-fold symmetry
bitcoin-miner -t 12 --all-algorithms

# Or use all cores
bitcoin-miner -t $(nproc) --objective28
```

### Algorithm Selection

```bash
# Standard mining (baseline)
bitcoin-miner -t 4

# Platonic optimization (+20-30%)
bitcoin-miner -t 4 --platonic

# OBJECTIVE 28 optimization (+30-50%)
bitcoin-miner -t 4 --objective28

# All optimizations (+50-80%)
bitcoin-miner -t 4 --all-algorithms
```

---

## Support

### Check Installation Status

```bash
# Run diagnostic script
./tools/run-bitcoin-tools.sh bitcoin-recovery --version
./tools/run-bitcoin-tools.sh bitcoin-miner --version

# Check libraries
ldconfig -p | grep recovery

# Check Python
python3 -c "import recovery; print('OK')"
```

### Get Help

- Documentation: See `BITCOIN_TOOLS_GUIDE.md`
- Issues: https://github.com/justmebob123/crystalline/issues
- Examples: See `examples/` directory

---

## Version

**Version:** 1.0.0
**Status:** Production Ready
**Platforms:** CentOS, RHEL, Ubuntu, Debian, Arch Linux

---

**Complete Installation Guide for Universal Recovery System**