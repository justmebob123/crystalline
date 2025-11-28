# Crystalline CLLM - Dependencies Documentation

## Overview

This document lists all dependencies required to build and run the Crystalline CLLM system, including version requirements, installation instructions for different platforms, and troubleshooting tips.

## Quick Start

### Ubuntu/Debian
```bash
./install_deps_ubuntu.sh
```

### CentOS/RHEL
```bash
./install_deps_centos.sh
```

## Core Dependencies

### 1. Build Tools

| Package | Version | Purpose |
|---------|---------|---------|
| gcc | ≥ 7.0 | C compiler |
| g++ | ≥ 7.0 | C++ compiler (optional) |
| make | ≥ 4.0 | Build automation |
| cmake | ≥ 3.10 | Build system (optional) |
| git | ≥ 2.0 | Version control |
| pkg-config | Any | Library configuration |

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential gcc g++ make cmake git pkg-config

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install gcc gcc-c++ make cmake git pkgconfig
```

### 2. Core Libraries

| Library | Version | Purpose | Required |
|---------|---------|---------|----------|
| libcurl | ≥ 7.50 | HTTP client for web crawling | Yes |
| libxml2 | ≥ 2.9 | XML parsing for documents | Yes |
| libzip | ≥ 1.5 | ZIP file handling | Yes |
| zlib | ≥ 1.2 | Compression | Yes |

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev libxml2-dev libzip-dev zlib1g-dev

# CentOS/RHEL
sudo yum install libcurl-devel libxml2-devel libzip-devel zlib-devel
```

**Version Check:**
```bash
pkg-config --modversion libcurl
pkg-config --modversion libxml-2.0
pkg-config --modversion libzip
```

### 3. SDL2 Libraries (GUI)

| Library | Version | Purpose | Required |
|---------|---------|---------|----------|
| SDL2 | ≥ 2.0.8 | Graphics and windowing | Yes (for GUI) |
| SDL2_ttf | ≥ 2.0.14 | TrueType font rendering | Yes (for GUI) |

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev libsdl2-ttf-dev

# CentOS/RHEL
sudo yum install SDL2-devel SDL2_ttf-devel
```

**Version Check:**
```bash
pkg-config --modversion sdl2
pkg-config --modversion SDL2_ttf
```

**Note:** The GUI application requires SDL2. If you only need the CLI tools, SDL2 is optional.

## Document Processing Tools

### 4. PDF Processing

| Tool | Purpose | Required |
|------|---------|----------|
| poppler-utils | PDF text extraction (pdftotext, pdfinfo) | Yes |
| wkhtmltopdf | HTML to PDF conversion | Optional |

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install poppler-utils wkhtmltopdf

# CentOS/RHEL
sudo yum install poppler-utils wkhtmltopdf
```

### 5. Office Document Processing

| Tool | Purpose | Required |
|------|---------|----------|
| antiword | MS Word (.doc) text extraction | Optional |
| unrtf | RTF to text conversion | Optional |
| catdoc | MS Word (.doc) text extraction | Optional |

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install antiword unrtf catdoc

# CentOS/RHEL
sudo yum install antiword catdoc
# unrtf may not be available
```

**Note:** The system uses libdocproc for DOCX/XLSX/PPTX processing (pure C implementation), so these tools are optional fallbacks.

### 6. OCR (Optical Character Recognition)

| Tool | Purpose | Required |
|------|---------|----------|
| tesseract-ocr | Image text extraction | Optional |
| tesseract-ocr-eng | English language data | Optional |

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install tesseract-ocr tesseract-ocr-eng

# CentOS/RHEL
sudo yum install tesseract tesseract-langpack-eng
```

## Text Processing Utilities

### 7. Command-Line Tools

| Tool | Purpose | Required |
|------|---------|----------|
| grep | Pattern matching | Yes |
| gawk | Text processing | Yes |
| sed | Stream editing | Yes |
| jq | JSON processing | Optional |
| csvkit | CSV processing | Optional |
| xmlstarlet | XML processing | Optional |

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install grep gawk sed jq csvkit xmlstarlet

# CentOS/RHEL
sudo yum install grep gawk sed jq xmlstarlet
sudo pip3 install csvkit  # csvkit via pip
```

## Additional Utilities

### 8. General Utilities

| Tool | Purpose | Required |
|------|---------|----------|
| wget | File downloading | Optional |
| curl | HTTP client | Yes |
| zip/unzip | Archive handling | Optional |
| tmux | Terminal multiplexer | Optional |
| vim | Text editor | Optional |
| tree | Directory visualization | Optional |
| rsync | File synchronization | Optional |

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install wget curl zip unzip tmux vim tree rsync

# CentOS/RHEL
sudo yum install wget curl zip unzip tmux vim tree rsync
```

### 9. Node.js (Web Tools)

| Tool | Version | Purpose | Required |
|------|---------|---------|----------|
| Node.js | ≥ 18.0 | JavaScript runtime | Optional |
| npm | ≥ 8.0 | Package manager | Optional |

**Installation:**
```bash
# Ubuntu/Debian
curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
sudo apt-get install -y nodejs

# CentOS/RHEL
curl -fsSL https://rpm.nodesource.com/setup_20.x | sudo bash -
sudo yum install -y nodejs
```

**Version Check:**
```bash
node --version
npm --version
```

## Platform-Specific Notes

### Ubuntu/Debian

**Tested Versions:**
- Ubuntu 20.04 LTS (Focal Fossa)
- Ubuntu 22.04 LTS (Jammy Jellyfish)
- Ubuntu 24.04 LTS (Noble Numbat)
- Debian 11 (Bullseye)
- Debian 12 (Bookworm)

**Known Issues:**
- On Ubuntu 18.04, you may need to install libzip from source (version too old)
- SDL2_ttf may require manual font installation on minimal systems

### CentOS/RHEL

**Tested Versions:**
- CentOS 7
- CentOS 8 Stream
- RHEL 8
- RHEL 9

**Known Issues:**
- EPEL repository required for many packages
- Some tools (unrtf, csvkit) may not be available in default repos
- SDL2 may require PowerTools/CodeReady repository on RHEL 8+

**Enable PowerTools (RHEL 8+):**
```bash
sudo dnf config-manager --set-enabled powertools  # RHEL 8
sudo dnf config-manager --set-enabled crb         # RHEL 9
```

### macOS

**Installation via Homebrew:**
```bash
brew install gcc make cmake git pkg-config
brew install curl libxml2 libzip
brew install sdl2 sdl2_ttf
brew install poppler tesseract
brew install jq xmlstarlet
brew install node
```

**Note:** macOS support is experimental. Some features may not work correctly.

## Building from Source

If your distribution doesn't have recent enough versions, you can build from source:

### libzip (if version < 1.5)

```bash
wget https://libzip.org/download/libzip-1.10.1.tar.gz
tar xzf libzip-1.10.1.tar.gz
cd libzip-1.10.1
mkdir build && cd build
cmake ..
make
sudo make install
sudo ldconfig
```

### SDL2 (if version < 2.0.8)

```bash
wget https://www.libsdl.org/release/SDL2-2.28.5.tar.gz
tar xzf SDL2-2.28.5.tar.gz
cd SDL2-2.28.5
./configure
make
sudo make install
sudo ldconfig
```

### SDL2_ttf (if version < 2.0.14)

```bash
wget https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-2.20.2.tar.gz
tar xzf SDL2_ttf-2.20.2.tar.gz
cd SDL2_ttf-2.20.2
./configure
make
sudo make install
sudo ldconfig
```

## Verification

After installing dependencies, verify your setup:

```bash
# Check compiler
gcc --version

# Check libraries
pkg-config --modversion libcurl
pkg-config --modversion libxml-2.0
pkg-config --modversion libzip
pkg-config --modversion sdl2

# Check tools
pdftotext -v
tesseract --version
jq --version
node --version

# Try building
make clean
make
```

## Troubleshooting

### Common Issues

#### 1. "Package not found" errors

**Problem:** pkg-config cannot find a library

**Solution:**
```bash
# Check if library is installed
dpkg -l | grep libcurl  # Ubuntu/Debian
rpm -qa | grep libcurl  # CentOS/RHEL

# Check pkg-config path
echo $PKG_CONFIG_PATH

# Add custom path if needed
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

#### 2. SDL2 not found

**Problem:** SDL2 headers or libraries not found

**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install libsdl2-dev libsdl2-ttf-dev

# Check installation
ls /usr/include/SDL2/
ls /usr/lib/x86_64-linux-gnu/libSDL2*

# If installed in custom location
export CFLAGS="-I/usr/local/include/SDL2"
export LDFLAGS="-L/usr/local/lib"
```

#### 3. libzip version too old

**Problem:** libzip version < 1.5

**Solution:** Build from source (see "Building from Source" section above)

#### 4. Missing fonts for SDL2_ttf

**Problem:** Application crashes with "Cannot load font"

**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install fonts-dejavu-core

# CentOS/RHEL
sudo yum install dejavu-sans-fonts

# Or specify custom font path in application
```

#### 5. EPEL repository issues (CentOS/RHEL)

**Problem:** Cannot install packages from EPEL

**Solution:**
```bash
# CentOS 7
sudo yum install epel-release

# CentOS 8/RHEL 8
sudo dnf install epel-release
sudo dnf config-manager --set-enabled powertools

# RHEL 9
sudo dnf install epel-release
sudo dnf config-manager --set-enabled crb
```

## Minimal Dependencies

If you only need the core functionality (no GUI, no document processing):

**Required:**
- gcc, make, git
- libcurl
- libxml2
- libzip

**Installation:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential libcurl4-openssl-dev libxml2-dev libzip-dev

# CentOS/RHEL
sudo yum groupinstall "Development Tools"
sudo yum install libcurl-devel libxml2-devel libzip-devel
```

## Optional Features

| Feature | Dependencies | Notes |
|---------|--------------|-------|
| GUI Application | SDL2, SDL2_ttf | Required for visual interface |
| PDF Processing | poppler-utils | For PDF text extraction |
| Office Documents | libdocproc (built-in) | Pure C implementation |
| OCR | tesseract-ocr | For image text extraction |
| Web Crawler | libcurl | Already required |
| JSON Processing | jq | Optional CLI tool |
| CSV Processing | csvkit | Optional CLI tool |

## Docker Support

For a containerized environment with all dependencies:

```dockerfile
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential gcc g++ make cmake git pkg-config \
    libcurl4-openssl-dev libxml2-dev libzip-dev zlib1g-dev \
    libsdl2-dev libsdl2-ttf-dev \
    poppler-utils tesseract-ocr \
    jq xmlstarstar \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
```

## Support

For dependency-related issues:
1. Check this documentation
2. Review the troubleshooting section
3. Check the GitHub issues
4. Contact the development team

## Version History

- **v1.0** (2024-11): Initial dependency documentation
- Added Ubuntu/Debian support
- Added CentOS/RHEL support
- Added troubleshooting guide