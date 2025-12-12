<?php
/**
 * Crystalline Math - Rainbow Table Example
 * 
 * This example demonstrates rainbow table operations for
 * fast prime lookups.
 */

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension is not loaded.\n" .
        "Please install it with: sudo make install-php\n");
}

echo "=== Crystalline Math - Rainbow Table ===\n\n";

// Example 1: Initialize rainbow table
echo "1. Initialize Rainbow Table:\n";
$capacity = crystalline_rainbow_init(10000);
if ($capacity !== false) {
    echo "   Rainbow table initialized with capacity: $capacity\n";
} else {
    echo "   Failed to initialize rainbow table\n";
}
echo "\n";

// Example 2: Populate with primes
echo "2. Populate Rainbow Table:\n";
$count = 1000;
$result = crystalline_rainbow_populate($count);
if ($result) {
    echo "   Populated with $count primes\n";
} else {
    echo "   Failed to populate rainbow table\n";
}
echo "\n";

// Example 3: Lookup primes by index
echo "3. Lookup Primes by Index:\n";
for ($i = 1; $i <= 20; $i++) {
    $prime = crystalline_rainbow_lookup($i);
    echo sprintf("   Index %3d: %5d\n", $i, $prime);
}
echo "\n";

// Example 4: Performance comparison
echo "4. Performance Comparison:\n";

// Using rainbow table (via prime_nth)
$start = microtime(true);
for ($i = 1; $i <= 1000; $i++) {
    crystalline_prime_nth($i);
}
$rainbow_time = microtime(true) - $start;

// Using O(1) generation
$start = microtime(true);
for ($i = 0; $i < 1000; $i++) {
    crystalline_prime_generate_o1($i % 12, $i / 12);
}
$o1_time = microtime(true) - $start;

echo sprintf("   Rainbow lookup (1000 primes): %.4f seconds\n", $rainbow_time);
echo sprintf("   O(1) generation (1000 primes): %.4f seconds\n", $o1_time);
echo sprintf("   Speedup: %.2fx\n", $rainbow_time / $o1_time);
echo "\n";

// Example 5: Large prime lookups
echo "5. Large Prime Lookups:\n";
$indices = [100, 500, 1000, 5000, 10000];
foreach ($indices as $index) {
    $prime = crystalline_prime_nth($index);
    echo sprintf("   Prime #%5d: %d\n", $index, $prime);
}
echo "\n";

echo "=== Example Complete ===\n";
?>