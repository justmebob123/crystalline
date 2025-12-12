#!/bin/bash
# Script to debug PHP extension segfault with gdb

echo "=== Installing GDB and Valgrind ==="
sudo dnf install -y gdb valgrind || sudo yum install -y gdb valgrind

echo ""
echo "=== Creating GDB commands file ==="
cat > /tmp/gdb_commands.txt << 'EOF'
set pagination off
set logging file /tmp/gdb_output.txt
set logging on
run examples/php/prime_generation.php
bt full
info registers
info locals
quit
EOF

echo ""
echo "=== Running PHP under GDB ==="
gdb -batch -x /tmp/gdb_commands.txt php

echo ""
echo "=== GDB Output ==="
cat /tmp/gdb_output.txt

echo ""
echo "=== Running Valgrind ==="
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose php examples/php/prime_generation.php 2>&1 | tee /tmp/valgrind_output.txt

echo ""
echo "=== Analysis Complete ==="
echo "GDB output saved to: /tmp/gdb_output.txt"
echo "Valgrind output saved to: /tmp/valgrind_output.txt"