#!/bin/bash

echo "# Complete File Inventory"
echo ""
echo "## Source Files (.c)"
echo ""
find src -name "*.c" -type f | wc -l | xargs echo "Total .c files:"
echo ""

echo "## Header Files (.h)"
echo ""
find include -name "*.h" -type f | wc -l | xargs echo "Total .h files:"
echo ""

echo "## Tools"
echo ""
find tools -type f -executable | wc -l | xargs echo "Total tools:"
echo ""

echo "## Demos"
echo ""
find demos -name "*.c" -type f | wc -l | xargs echo "Total demos:"
echo ""

echo "## Tests"
echo ""
find tests -name "*.c" -type f | wc -l | xargs echo "Total tests:"
echo ""

echo "## Total Code Files"
echo ""
total_c=$(find src -name "*.c" -type f | wc -l)
total_h=$(find include -name "*.h" -type f | wc -l)
total_tools=$(find tools -type f -executable | wc -l)
total_demos=$(find demos -name "*.c" -type f | wc -l)
total_tests=$(find tests -name "*.c" -type f | wc -l)
total=$((total_c + total_h + total_tools + total_demos + total_tests))
echo "Total: $total files"
