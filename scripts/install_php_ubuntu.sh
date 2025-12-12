#!/bin/bash
# Install PHP Extension on Ubuntu

set -e

PHP_EXT_NAME="crystalline_math"

echo "Configuring PHP extension for Ubuntu..."

# Detect PHP version
PHP_VERSION=$(php -r 'echo PHP_MAJOR_VERSION.".".PHP_MINOR_VERSION;' 2>/dev/null)

if [ -z "$PHP_VERSION" ]; then
    echo "ERROR: PHP not found"
    exit 1
fi

echo "Detected PHP version: $PHP_VERSION"

# Create CLI configuration
CLI_CONF="/etc/php/$PHP_VERSION/cli/conf.d/20-crystalline.ini"
echo "extension=$PHP_EXT_NAME.so" | sudo tee "$CLI_CONF"
echo "✓ CLI configuration created: $CLI_CONF"

# Create Apache configuration if Apache module exists
if [ -d "/etc/php/$PHP_VERSION/apache2/conf.d" ]; then
    APACHE_CONF="/etc/php/$PHP_VERSION/apache2/conf.d/20-crystalline.ini"
    echo "extension=$PHP_EXT_NAME.so" | sudo tee "$APACHE_CONF"
    echo "✓ Apache configuration created: $APACHE_CONF"
    
    # Restart Apache
    if systemctl is-active --quiet apache2; then
        echo "Restarting Apache..."
        sudo systemctl restart apache2
        echo "✓ Apache restarted"
    elif service apache2 status >/dev/null 2>&1; then
        echo "Restarting Apache..."
        sudo service apache2 restart
        echo "✓ Apache restarted"
    fi
fi

echo ""
echo "✓ PHP extension configured for Ubuntu"
echo ""
echo "Test with: php -m | grep crystalline"
echo "Or run: php examples/php/test_extension.php"