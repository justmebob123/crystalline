#!/bin/bash
# Install PHP Extension on CentOS/RHEL

set -e

PHP_EXT_NAME="crystalline_math"

echo "Configuring PHP extension for CentOS/RHEL..."

# Create PHP configuration
PHP_CONF="/etc/php.d/crystalline.ini"
echo "extension=$PHP_EXT_NAME.so" | sudo tee "$PHP_CONF"
echo "✓ PHP configuration created: $PHP_CONF"

# Restart Apache (httpd) if running
if systemctl is-active --quiet httpd; then
    echo "Restarting Apache (httpd)..."
    sudo systemctl restart httpd
    echo "✓ Apache restarted"
fi

echo ""
echo "✓ PHP extension configured for CentOS/RHEL"
echo ""
echo "Test with: php -m | grep crystalline"
echo "Or run: php examples/php/test_extension.php"