<?php
/**
 * Crystalline Math - Prime Generation Example
 * 
 * This example demonstrates O(1) deterministic prime generation
 * using the Crystalline Math PHP extension.
 */

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension is not loaded.\n" .
        "Please install it with: sudo make install-php\n");
}

echo "=== Crystalline Math - Prime Generation ===\n\n";

// Display version
echo "Extension Version: " . crystalline_version() . "\n\n";

// Example 1: Generate primes using O(1) formula
echo "1. O(1) Prime Generation:\n";
echo "   Position 3, Magnitude 0: " . crystalline_prime_generate_o1(3, 0) . "\n";
echo "   Position 3, Magnitude 1: " . crystalline_prime_generate_o1(3, 1) . "\n";
echo "   Position 3, Magnitude 2: " . crystalline_prime_generate_o1(3, 2) . "\n";
echo "   Position 5, Magnitude 0: " . crystalline_prime_generate_o1(5, 0) . "\n";
echo "   Position 5, Magnitude 1: " . crystalline_prime_generate_o1(5, 1) . "\n\n";

// Example 2: Check if numbers are prime
echo "2. Primality Testing:\n";
$test_numbers = [2, 3, 4, 5, 17, 100, 157, 997];
foreach ($test_numbers as $n) {
    $is_prime = crystalline_prime_is_prime($n);
    echo "   $n is " . ($is_prime ? "PRIME" : "composite") . "\n";
}
echo "\n";

// Example 3: Get nth prime
echo "3. Nth Prime:\n";
for ($i = 1; $i <= 10; $i++) {
    $prime = crystalline_prime_nth($i);
    echo "   Prime #$i: $prime\n";
}
echo "\n";

// Example 4: Generate primes in all 12 positions
echo "4. Primes in All 12 Clock Positions (Magnitude 0):\n";
for ($pos = 0; $pos < 12; $pos++) {
    $prime = crystalline_prime_generate_o1($pos, 0);
    if ($prime > 0) {
        echo "   Position $pos: $prime\n";
    } else {
        echo "   Position $pos: composite\n";
    }
}
echo "\n";

// Example 5: Performance test
echo "5. Performance Test:\n";
$start = microtime(true);
$count = 1000;
for ($i = 0; $i < $count; $i++) {
    crystalline_prime_is_prime(rand(1, 10000));
}
$elapsed = microtime(true) - $start;
$per_second = $count / $elapsed;
echo "   Checked $count numbers in " . number_format($elapsed, 4) . " seconds\n";
echo "   Rate: " . number_format($per_second, 0) . " checks/second\n\n";

echo "=== Example Complete ===\n";
?>