# Recovery System Implementation Plan

## Overview
Create a unified, production-ready recovery system with comprehensive format support, proper organization, and all necessary dependencies.

## Current State Analysis

### Existing Infrastructure
1. **Libraries** (11 total):
   - `librecovery_common.a` - Common recovery utilities
   - `librecovery_core.a/.so` - Core recovery algorithms
   - `librecovery_crypto.a/.so` - Crypto recovery (EXISTS!)
   - `librecovery_signal.a/.so` - Signal recovery (EXISTS!)
   - `librecovery_network.a/.so` - Network recovery
   - `libgeometric_recovery.a` - Geometric recovery (Objective 28)

2. **Tools** (11 recovery tools):
   - `bitcoin-recovery` - Bitcoin key recovery (EXISTS!)
   - `crypto-recovery` - General crypto recovery
   - `signal-recovery` - Signal recovery (WORKING!)
   - `geometric-recovery` - Geometric recovery
   - `image-recovery` - Image recovery
   - `network-recovery` - Network recovery
   - `scientific-recovery` - Scientific recovery
   - `ml-recovery` - ML recovery
   - `platonic-demo` - Platonic solid demo
   - `universal-recovery` - Universal recovery
   - `test_blind_recovery` - Blind recovery test

3. **Existing Capabilities**:
   - ECDSA sample loader/generator (algorithms/src/geometric_recovery/)
   - Bitcoin transaction handling (tools/bitcoin-recovery.c)
   - WAV loader (just implemented)
   - Comprehensive geometric recovery (Objective 28)
   - GNURadio integration (gnuradio/gr-recovery/)

### Missing Components
1. **JSON loader** - For crypto data
2. **CSV loader** - For I/Q and signal data
3. **HackRF support** - Native SDR device support
4. **Unified CLI** - Single tool with format specification
5. **Dependency management** - Install scripts for Ubuntu/CentOS
6. **Production organization** - Clean directory structure

## Implementation Plan

### Phase 1: Dependency Management (30 minutes)
- [ ] Create `install_dependencies_ubuntu.sh`
- [ ] Create `install_dependencies_centos.sh`
- [ ] Install: jansson, libhackrf, librtlsdr, libusb, OpenSSL, pkg-config
- [ ] Test all dependencies

### Phase 2: Format Loaders (2 hours)
- [ ] Implement JSON loader (jansson-based)
- [ ] Implement CSV loader (for I/Q data)
- [ ] Implement binary I/Q loader
- [ ] Implement HackRF native reader
- [ ] Update recovery_common library

### Phase 3: Unified CLI Tool (2 hours)
- [ ] Create `recovery-cli` unified tool
- [ ] Support format specification: --format json|csv|wav|iq|hackrf
- [ ] Support device specification: --device /dev/hackrf0
- [ ] Support sample rate: --sample-rate 2.4e6
- [ ] Integrate all recovery types

### Phase 4: Bitcoin/Crypto Integration (1 hour)
- [ ] Fix bitcoin-recovery library linking
- [ ] Integrate ECDSA sample loader
- [ ] Add transaction ID loading
- [ ] Single-command k recovery from txid

### Phase 5: Production Organization (2 hours)
- [ ] Create `/recovery` directory in repo root
- [ ] Audit and copy all relevant files
- [ ] Organize by category:
  - `/recovery/lib` - All libraries
  - `/recovery/include` - All headers
  - `/recovery/src` - All source files
  - `/recovery/tools` - All CLI tools
  - `/recovery/tests` - All tests
  - `/recovery/docs` - Documentation
- [ ] Update all Makefiles
- [ ] Create master Makefile

### Phase 6: Testing & Validation (1 hour)
- [ ] Test all format loaders
- [ ] Test unified CLI
- [ ] Test Bitcoin recovery end-to-end
- [ ] Test HackRF integration
- [ ] Generate comprehensive test report

## File Organization Structure

```
/recovery/
├── lib/
│   ├── recovery_common/      # Common utilities (JSON, CSV, WAV loaders)
│   ├── recovery_core/         # Core algorithms
│   ├── recovery_crypto/       # Crypto/Bitcoin recovery
│   ├── recovery_signal/       # Signal/I/Q recovery
│   ├── recovery_network/      # Network recovery
│   ├── recovery_geometric/    # Geometric recovery (Objective 28)
│   └── Makefile
├── include/
│   ├── recovery_common.h
│   ├── recovery_core.h
│   ├── recovery_crypto.h
│   ├── recovery_signal.h
│   ├── recovery_network.h
│   └── recovery_geometric.h
├── src/
│   ├── loaders/              # Format loaders
│   │   ├── json_loader.c
│   │   ├── csv_loader.c
│   │   ├── wav_loader.c
│   │   ├── iq_loader.c
│   │   └── hackrf_reader.c
│   ├── crypto/               # Crypto recovery
│   ├── signal/               # Signal recovery
│   └── geometric/            # Geometric recovery
├── tools/
│   ├── recovery-cli          # Unified CLI tool
│   ├── bitcoin-recovery      # Bitcoin-specific
│   ├── signal-recovery       # Signal-specific
│   └── Makefile
├── tests/
│   ├── test_loaders.c
│   ├── test_crypto_recovery.c
│   ├── test_signal_recovery.c
│   └── Makefile
├── docs/
│   ├── API.md
│   ├── FORMATS.md
│   ├── EXAMPLES.md
│   └── ARCHITECTURE.md
├── scripts/
│   ├── install_dependencies_ubuntu.sh
│   ├── install_dependencies_centos.sh
│   └── build_all.sh
├── Makefile                  # Master Makefile
└── README.md
```

## Dependencies

### Ubuntu/Debian
```bash
apt-get install -y \
  build-essential \
  pkg-config \
  libjansson-dev \
  libhackrf-dev \
  librtlsdr-dev \
  libusb-1.0-0-dev \
  libssl-dev \
  libopenssl-dev \
  cmake \
  git
```

### CentOS/RHEL
```bash
yum install -y \
  gcc gcc-c++ make \
  pkgconfig \
  jansson-devel \
  hackrf-devel \
  rtl-sdr-devel \
  libusb-devel \
  openssl-devel \
  cmake \
  git
```

## Timeline
- **Phase 1**: 30 minutes (Dependencies)
- **Phase 2**: 2 hours (Format loaders)
- **Phase 3**: 2 hours (Unified CLI)
- **Phase 4**: 1 hour (Bitcoin integration)
- **Phase 5**: 2 hours (Organization)
- **Phase 6**: 1 hour (Testing)
- **Total**: ~9 hours

## Success Criteria
1. ✅ All dependencies installed
2. ✅ JSON, CSV, WAV, I/Q, HackRF loaders working
3. ✅ Unified CLI tool functional
4. ✅ Bitcoin recovery: single command from txid
5. ✅ Signal recovery: HackRF native support
6. ✅ Clean production organization
7. ✅ All tests passing
8. ✅ Comprehensive documentation

## Next Steps
1. Start with Phase 1: Install dependencies
2. Implement Phase 2: Format loaders
3. Continue through phases sequentially