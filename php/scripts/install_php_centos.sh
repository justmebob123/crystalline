#!/bin/bash
# PHP Extension Installation Script for CentOS/RHEL
# Installs crystalline_math and algorithms PHP extensions

set -e

echo "=== Crystalline PHP Extension Installer for CentOS/RHEL ==="
echo ""

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Error: This script must be run as root (use sudo)"
    exit 1
fi

# Get PHP extension directory
PHP_EXT_DIR=$(php-config --extension-dir 2>/dev/null)
if [ -z "$PHP_EXT_DIR" ]; then
    echo "Error: PHP not found. Please install PHP first:"
    echo "  sudo yum install php php-devel"
    exit 1
fi

echo "PHP Extension Directory: $PHP_EXT_DIR"
echo ""

# Build crystalline_math extension
echo "Building crystalline_math extension..."
cd /workspace/php
make clean
make

if [ ! -f "modules/crystalline_math.so" ]; then
    echo "Error: Failed to build crystalline_math.so"
    exit 1
fi

# Install crystalline_math
echo "Installing crystalline_math extension..."
cp modules/crystalline_math.so "$PHP_EXT_DIR/"
chmod 755 "$PHP_EXT_DIR/crystalline_math.so"

# Create PHP configuration
PHP_VERSION=$(php -r "echo PHP_MAJOR_VERSION.'.'.PHP_MINOR_VERSION;")
INI_DIR="/etc/php.d"

if [ ! -d "$INI_DIR" ]; then
    INI_DIR="/etc/php/$PHP_VERSION/mods-available"
    mkdir -p "$INI_DIR"
fi

echo "Creating configuration in $INI_DIR..."
cat > "$INI_DIR/20-crystalline_math.ini" << 'EOF'
; configuration for Crystalline Math extension
; priority=20
extension=crystalline_math.so
EOF

echo ""
echo "✓ Installation complete!"
echo ""
echo "Verify installation:"
echo "  php -m | grep crystalline"
echo ""
echo "Test the extension:"
echo "  php /workspace/php/examples/stock_trading_analysis.php"
echo ""