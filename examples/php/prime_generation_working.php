<?php
/**
 * Crystalline Math - Prime Generation Example (Working Version)
 * 
 * This example demonstrates the working features of the PHP extension.
 * Note: crystalline_prime_nth() has a known issue and is excluded.
 */

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension is not loaded.\n" .
        "Please install it with: sudo scripts/install_php_centos.sh\n");
}

echo "=== Crystalline Math - Prime Generation (Working Features) ===\n\n";

// Display loaded extension info
echo "Extension loaded: crystalline_math\n\n";

// Example 1: Generate primes using O(1) formula
echo "1. O(1) Prime Generation:\n";
echo "   Position 3, Magnitude 0: " . crystalline_prime_generate_o1(3, 0) . "\n";
echo "   Position 3, Magnitude 1: " . crystalline_prime_generate_o1(3, 1) . "\n";
echo "   Position 3, Magnitude 2: " . crystalline_prime_generate_o1(3, 2) . "\n";
echo "   Position 6, Magnitude 0: " . crystalline_prime_generate_o1(6, 0) . "\n";
echo "   Position 6, Magnitude 1: " . crystalline_prime_generate_o1(6, 1) . "\n";
echo "   Position 9, Magnitude 0: " . crystalline_prime_generate_o1(9, 0) . "\n";
echo "   Position 9, Magnitude 1: " . crystalline_prime_generate_o1(9, 1) . "\n\n";

// Example 2: Check if numbers are prime
echo "2. Primality Testing:\n";
$test_numbers = [2, 3, 4, 5, 17, 100, 157, 997, 1009, 10007];
foreach ($test_numbers as $n) {
    $is_prime = is_prime($n);
    echo "   $n is " . ($is_prime ? "PRIME" : "composite") . "\n";
}
echo "\n";

// Example 3: Generate first N primes (workaround for prime_nth)
echo "3. First 20 Primes (using is_prime):\n";
$primes = [];
$candidate = 2;
while (count($primes) < 20) {
    if (is_prime($candidate)) {
        $primes[] = $candidate;
    }
    $candidate++;
}
echo "   " . implode(", ", $primes) . "\n\n";

// Example 4: Generate primes in all 12 clock positions
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
echo "5. Performance Test (Primality Testing):\n";
$start = microtime(true);
$count = 1000;
for ($i = 0; $i < $count; $i++) {
    is_prime(rand(1, 10000));
}
$elapsed = microtime(true) - $start;
$per_second = $count / $elapsed;
echo "   Checked $count numbers in " . number_format($elapsed, 4) . " seconds\n";
echo "   Rate: " . number_format($per_second, 0) . " checks/second\n\n";

// Example 6: Find twin primes
echo "6. Twin Primes (p and p+2 both prime):\n";
$twin_primes = [];
for ($p = 3; $p < 100 && count($twin_primes) < 10; $p += 2) {
    if (is_prime($p) && is_prime($p + 2)) {
        $twin_primes[] = "($p, " . ($p + 2) . ")";
    }
}
echo "   " . implode(", ", $twin_primes) . "\n\n";

echo "=== Example Complete ===\n";
echo "\nNote: crystalline_prime_nth() has a known issue and is not used in this example.\n";
echo "Use is_prime() in a loop as shown above to find the nth prime.\n";
?>