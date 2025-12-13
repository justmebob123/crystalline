# Algorithms PHP Extension Makefile

PHPIZE = phpize
PHP_CONFIG = php-config
EXTENSION_DIR = $(shell $(PHP_CONFIG) --extension-dir 2>/dev/null || echo "/usr/lib/php/modules")
PHP_INI_DIR = $(shell $(PHP_CONFIG) --ini-dir 2>/dev/null || echo "/etc/php/conf.d")

.PHONY: all build install clean test

all: build

build:
	@echo "Building Algorithms PHP Extension..."
	$(PHPIZE)
	./configure --with-php-config=$(PHP_CONFIG) --enable-algorithms
	$(MAKE)
	@echo "✓ Build complete"

install: build
	@echo "Installing Algorithms PHP Extension..."
	sudo $(MAKE) install
	@echo "extension=algorithms.so" | sudo tee $(PHP_INI_DIR)/algorithms.ini
	@echo "✓ Installation complete"
	@echo "✓ Extension enabled in $(PHP_INI_DIR)/algorithms.ini"

clean:
	@echo "Cleaning build files..."
	-$(MAKE) clean 2>/dev/null || true
	-$(PHPIZE) --clean 2>/dev/null || true
	rm -rf .libs *.lo *.la modules/ autom4te.cache/ build/ config.* configure* install-sh ltmain.sh missing mkinstalldirs run-tests.php
	@echo "✓ Clean complete"

test:
	@echo "Testing Algorithms PHP Extension..."
	php test.php
	@echo "✓ Tests complete"

help:
	@echo "Algorithms PHP Extension Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make build    - Build the extension"
	@echo "  make install  - Build and install the extension"
	@echo "  make clean    - Clean build files"
	@echo "  make test     - Run tests"
	@echo "  make help     - Show this help"