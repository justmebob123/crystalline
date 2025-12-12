<?php
/**
 * Crystalline Math - Clock Lattice Example
 * 
 * This example demonstrates the Babylonian clock lattice structure
 * with 12-fold symmetry.
 */

// Check if extension is loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension is not loaded.\n" .
        "Please install it with: sudo make install-php\n");
}

echo "=== Crystalline Math - Clock Lattice ===\n\n";

// Example 1: Map primes to clock positions
echo "1. Prime to Clock Position Mapping:\n";
$primes = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47];
foreach ($primes as $prime) {
    $pos = crystalline_clock_position($prime);
    if ($pos !== false) {
        echo sprintf("   Prime %3d: Ring %2d, Position %2d, Angle %.2f°, Radius %.2f\n",
            $prime, $pos['ring'], $pos['position'], $pos['angle'], $pos['radius']);
    }
}
echo "\n";

// Example 2: Validate clock positions
echo "2. Clock Position Validation:\n";
$test_positions = [
    [0, 0], [0, 5], [0, 11], [0, 12],  // Ring 0 (12 positions)
    [1, 0], [1, 30], [1, 59], [1, 60], // Ring 1 (60 positions)
    [2, 0], [2, 30], [2, 59], [2, 60], // Ring 2 (60 positions)
];
foreach ($test_positions as list($ring, $position)) {
    $valid = crystalline_clock_validate($ring, $position);
    echo sprintf("   Ring %d, Position %2d: %s\n",
        $ring, $position, $valid ? "VALID" : "invalid");
}
echo "\n";

// Example 3: 12-fold symmetry demonstration
echo "3. 12-Fold Symmetry (First 12 Primes):\n";
$symmetry_groups = array_fill(0, 12, []);
for ($i = 1; $i <= 100; $i++) {
    $prime = crystalline_prime_nth($i);
    $pos = crystalline_clock_position($prime);
    if ($pos !== false) {
        $group = $pos['position'] % 12;
        $symmetry_groups[$group][] = $prime;
    }
}

for ($i = 0; $i < 12; $i++) {
    echo "   Group $i: " . implode(', ', array_slice($symmetry_groups[$i], 0, 5));
    if (count($symmetry_groups[$i]) > 5) {
        echo " ... (" . count($symmetry_groups[$i]) . " total)";
    }
    echo "\n";
}
echo "\n";

// Example 4: Clock lattice structure
echo "4. Clock Lattice Structure:\n";
echo "   Ring 0: 12 positions (base clock)\n";
echo "   Ring 1: 60 positions (first expansion)\n";
echo "   Ring 2: 60 positions (second expansion)\n";
echo "   Ring 3: 100 positions (magnitude expansion)\n";
echo "   Total: 232 positions in Babylonian structure\n\n";

echo "=== Example Complete ===\n";
?>