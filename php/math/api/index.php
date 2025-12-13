<?php
/**
 * Crystalline Math REST API
 * 
 * Complete REST API for all Crystalline Math functions
 */

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle OPTIONS request
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit();
}

// Get request path
$path = parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH);
$path = str_replace('/api/math/', '', $path);
$path = trim($path, '/');

// Get request body
$input = json_decode(file_get_contents('php://input'), true);

// Response helper
function respond($data, $status = 200) {
    http_response_code($status);
    echo json_encode($data, JSON_PRETTY_PRINT);
    exit();
}

// Error helper
function error($message, $status = 400) {
    respond(['error' => $message], $status);
}

// ============================================================================
// ARITHMETIC ENDPOINTS
// ============================================================================

if ($path === 'add') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required');
    }
    $result = math_add($input['a'], $input['b']);
    respond(['result' => $result, 'operation' => 'addition']);
}

if ($path === 'subtract') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required');
    }
    $result = math_sub($input['a'], $input['b']);
    respond(['result' => $result, 'operation' => 'subtraction']);
}

if ($path === 'multiply') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required');
    }
    $result = math_mul($input['a'], $input['b']);
    respond(['result' => $result, 'operation' => 'multiplication']);
}

if ($path === 'divide') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required');
    }
    if ($input['b'] == 0) {
        error('Division by zero');
    }
    $result = math_div($input['a'], $input['b']);
    respond(['result' => $result, 'operation' => 'division']);
}

if ($path === 'mod') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required');
    }
    $result = math_mod($input['a'], $input['b']);
    respond(['result' => $result, 'operation' => 'modulo']);
}

if ($path === 'abs') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_abs($input['x']);
    respond(['result' => $result, 'operation' => 'absolute_value']);
}

if ($path === 'min') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required');
    }
    $result = math_min($input['a'], $input['b']);
    respond(['result' => $result, 'operation' => 'minimum']);
}

if ($path === 'max') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required');
    }
    $result = math_max($input['a'], $input['b']);
    respond(['result' => $result, 'operation' => 'maximum']);
}

if ($path === 'clamp') {
    if (!isset($input['x']) || !isset($input['min']) || !isset($input['max'])) {
        error('Missing parameters: x, min, max required');
    }
    $result = math_clamp($input['x'], $input['min'], $input['max']);
    respond(['result' => $result, 'operation' => 'clamp']);
}

if ($path === 'floor') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_floor($input['x']);
    respond(['result' => $result, 'operation' => 'floor']);
}

if ($path === 'ceil') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_ceil($input['x']);
    respond(['result' => $result, 'operation' => 'ceiling']);
}

if ($path === 'round') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_round($input['x']);
    respond(['result' => $result, 'operation' => 'round']);
}

// ============================================================================
// TRANSCENDENTAL ENDPOINTS
// ============================================================================

if ($path === 'sqrt') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    if ($input['x'] < 0) {
        error('Square root of negative number');
    }
    $result = math_sqrt($input['x']);
    respond(['result' => $result, 'operation' => 'square_root']);
}

if ($path === 'cbrt') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_cbrt($input['x']);
    respond(['result' => $result, 'operation' => 'cube_root']);
}

if ($path === 'pow') {
    if (!isset($input['x']) || !isset($input['y'])) {
        error('Missing parameters: x, y required');
    }
    $result = math_pow($input['x'], $input['y']);
    respond(['result' => $result, 'operation' => 'power']);
}

if ($path === 'exp') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_exp($input['x']);
    respond(['result' => $result, 'operation' => 'exponential']);
}

if ($path === 'log') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    if ($input['x'] <= 0) {
        error('Logarithm of non-positive number');
    }
    $result = math_log($input['x']);
    respond(['result' => $result, 'operation' => 'natural_logarithm']);
}

if ($path === 'log10') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    if ($input['x'] <= 0) {
        error('Logarithm of non-positive number');
    }
    $result = math_log10($input['x']);
    respond(['result' => $result, 'operation' => 'log_base_10']);
}

if ($path === 'log2') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    if ($input['x'] <= 0) {
        error('Logarithm of non-positive number');
    }
    $result = math_log2($input['x']);
    respond(['result' => $result, 'operation' => 'log_base_2']);
}

if ($path === 'sin') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_sin($input['x']);
    respond(['result' => $result, 'operation' => 'sine']);
}

if ($path === 'cos') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_cos($input['x']);
    respond(['result' => $result, 'operation' => 'cosine']);
}

if ($path === 'tan') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_tan($input['x']);
    respond(['result' => $result, 'operation' => 'tangent']);
}

if ($path === 'asin') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    if ($input['x'] < -1 || $input['x'] > 1) {
        error('Arcsine input must be in [-1, 1]');
    }
    $result = math_asin($input['x']);
    respond(['result' => $result, 'operation' => 'arcsine']);
}

if ($path === 'acos') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    if ($input['x'] < -1 || $input['x'] > 1) {
        error('Arccosine input must be in [-1, 1]');
    }
    $result = math_acos($input['x']);
    respond(['result' => $result, 'operation' => 'arccosine']);
}

if ($path === 'atan') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_atan($input['x']);
    respond(['result' => $result, 'operation' => 'arctangent']);
}

if ($path === 'atan2') {
    if (!isset($input['y']) || !isset($input['x'])) {
        error('Missing parameters: y, x required');
    }
    $result = math_atan2($input['y'], $input['x']);
    respond(['result' => $result, 'operation' => 'atan2']);
}

if ($path === 'sinh') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_sinh($input['x']);
    respond(['result' => $result, 'operation' => 'hyperbolic_sine']);
}

if ($path === 'cosh') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_cosh($input['x']);
    respond(['result' => $result, 'operation' => 'hyperbolic_cosine']);
}

if ($path === 'tanh') {
    if (!isset($input['x'])) {
        error('Missing parameter: x required');
    }
    $result = math_tanh($input['x']);
    respond(['result' => $result, 'operation' => 'hyperbolic_tangent']);
}

// ============================================================================
// PRIME NUMBER ENDPOINTS
// ============================================================================

if ($path === 'prime/nth') {
    if (!isset($input['n'])) {
        error('Missing parameter: n required');
    }
    $result = prime_nth($input['n']);
    respond(['result' => $result, 'operation' => 'nth_prime', 'n' => $input['n']]);
}

if ($path === 'prime/next') {
    if (!isset($input['n'])) {
        error('Missing parameter: n required');
    }
    $result = prime_next($input['n']);
    respond(['result' => $result, 'operation' => 'next_prime', 'after' => $input['n']]);
}

if ($path === 'prime/prev') {
    if (!isset($input['n'])) {
        error('Missing parameter: n required');
    }
    $result = prime_prev($input['n']);
    respond(['result' => $result, 'operation' => 'previous_prime', 'before' => $input['n']]);
}

if ($path === 'prime/is_prime') {
    if (!isset($input['n'])) {
        error('Missing parameter: n required');
    }
    $result = prime_is_prime($input['n']);
    respond(['result' => $result, 'operation' => 'primality_test', 'n' => $input['n']]);
}

if ($path === 'prime/count') {
    if (!isset($input['n'])) {
        error('Missing parameter: n required');
    }
    $result = prime_count_below($input['n']);
    respond(['result' => $result, 'operation' => 'count_primes_below', 'n' => $input['n']]);
}

if ($path === 'prime/factor') {
    if (!isset($input['n'])) {
        error('Missing parameter: n required');
    }
    $result = prime_factor($input['n']);
    respond(['result' => $result, 'operation' => 'prime_factorization', 'n' => $input['n']]);
}

// ============================================================================
// COMPLEX NUMBER ENDPOINTS
// ============================================================================

if ($path === 'complex/create') {
    if (!isset($input['real']) || !isset($input['imag'])) {
        error('Missing parameters: real, imag required');
    }
    $result = complex_create($input['real'], $input['imag']);
    respond(['result' => $result, 'operation' => 'create_complex']);
}

if ($path === 'complex/add') {
    if (!isset($input['z1']) || !isset($input['z2'])) {
        error('Missing parameters: z1, z2 required (each with real and imag)');
    }
    $z1 = complex_create($input['z1']['real'], $input['z1']['imag']);
    $z2 = complex_create($input['z2']['real'], $input['z2']['imag']);
    $result = complex_add($z1, $z2);
    respond(['result' => $result, 'operation' => 'complex_addition']);
}

if ($path === 'complex/subtract') {
    if (!isset($input['z1']) || !isset($input['z2'])) {
        error('Missing parameters: z1, z2 required (each with real and imag)');
    }
    $z1 = complex_create($input['z1']['real'], $input['z1']['imag']);
    $z2 = complex_create($input['z2']['real'], $input['z2']['imag']);
    $result = complex_sub($z1, $z2);
    respond(['result' => $result, 'operation' => 'complex_subtraction']);
}

if ($path === 'complex/multiply') {
    if (!isset($input['z1']) || !isset($input['z2'])) {
        error('Missing parameters: z1, z2 required (each with real and imag)');
    }
    $z1 = complex_create($input['z1']['real'], $input['z1']['imag']);
    $z2 = complex_create($input['z2']['real'], $input['z2']['imag']);
    $result = complex_mul($z1, $z2);
    respond(['result' => $result, 'operation' => 'complex_multiplication']);
}

if ($path === 'complex/divide') {
    if (!isset($input['z1']) || !isset($input['z2'])) {
        error('Missing parameters: z1, z2 required (each with real and imag)');
    }
    $z1 = complex_create($input['z1']['real'], $input['z1']['imag']);
    $z2 = complex_create($input['z2']['real'], $input['z2']['imag']);
    $result = complex_div($z1, $z2);
    respond(['result' => $result, 'operation' => 'complex_division']);
}

if ($path === 'complex/magnitude') {
    if (!isset($input['z'])) {
        error('Missing parameter: z required (with real and imag)');
    }
    $z = complex_create($input['z']['real'], $input['z']['imag']);
    $result = complex_magnitude($z);
    respond(['result' => $result, 'operation' => 'complex_magnitude']);
}

if ($path === 'complex/phase') {
    if (!isset($input['z'])) {
        error('Missing parameter: z required (with real and imag)');
    }
    $z = complex_create($input['z']['real'], $input['z']['imag']);
    $result = complex_phase($z);
    respond(['result' => $result, 'operation' => 'complex_phase']);
}

if ($path === 'complex/conjugate') {
    if (!isset($input['z'])) {
        error('Missing parameter: z required (with real and imag)');
    }
    $z = complex_create($input['z']['real'], $input['z']['imag']);
    $result = complex_conjugate($z);
    respond(['result' => $result, 'operation' => 'complex_conjugate']);
}

// ============================================================================
// ABACUS (ARBITRARY PRECISION) ENDPOINTS
// ============================================================================

if ($path === 'abacus/add') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required (as strings)');
    }
    $a = abacus_from_string($input['a']);
    $b = abacus_from_string($input['b']);
    $result = abacus_create();
    abacus_add($result, $a, $b);
    $result_str = abacus_to_string($result);
    respond(['result' => $result_str, 'operation' => 'abacus_addition']);
}

if ($path === 'abacus/subtract') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required (as strings)');
    }
    $a = abacus_from_string($input['a']);
    $b = abacus_from_string($input['b']);
    $result = abacus_create();
    abacus_sub($result, $a, $b);
    $result_str = abacus_to_string($result);
    respond(['result' => $result_str, 'operation' => 'abacus_subtraction']);
}

if ($path === 'abacus/multiply') {
    if (!isset($input['a']) || !isset($input['b'])) {
        error('Missing parameters: a, b required (as strings)');
    }
    $a = abacus_from_string($input['a']);
    $b = abacus_from_string($input['b']);
    $result = abacus_create();
    abacus_mul($result, $a, $b);
    $result_str = abacus_to_string($result);
    respond(['result' => $result_str, 'operation' => 'abacus_multiplication']);
}

// ============================================================================
// CLOCK LATTICE ENDPOINTS
// ============================================================================

if ($path === 'clock/map_prime') {
    if (!isset($input['prime'])) {
        error('Missing parameter: prime required');
    }
    $result = clock_map_prime($input['prime']);
    respond(['result' => $result, 'operation' => 'map_prime_to_clock']);
}

if ($path === 'clock/position_to_prime') {
    if (!isset($input['ring']) || !isset($input['position'])) {
        error('Missing parameters: ring, position required');
    }
    $pos = ['ring' => $input['ring'], 'position' => $input['position']];
    $result = clock_position_to_prime($pos);
    respond(['result' => $result, 'operation' => 'clock_position_to_prime']);
}

if ($path === 'clock/angular_distance') {
    if (!isset($input['prime1']) || !isset($input['prime2'])) {
        error('Missing parameters: prime1, prime2 required');
    }
    $pos1 = clock_map_prime($input['prime1']);
    $pos2 = clock_map_prime($input['prime2']);
    $result = clock_angular_distance($pos1, $pos2);
    respond(['result' => $result, 'operation' => 'angular_distance']);
}

// ============================================================================
// API DOCUMENTATION ENDPOINT
// ============================================================================

if ($path === '' || $path === 'help') {
    $endpoints = [
        'arithmetic' => [
            'add', 'subtract', 'multiply', 'divide', 'mod', 'abs',
            'min', 'max', 'clamp', 'floor', 'ceil', 'round'
        ],
        'transcendental' => [
            'sqrt', 'cbrt', 'pow', 'exp', 'log', 'log10', 'log2',
            'sin', 'cos', 'tan', 'asin', 'acos', 'atan', 'atan2',
            'sinh', 'cosh', 'tanh'
        ],
        'prime' => [
            'prime/nth', 'prime/next', 'prime/prev', 'prime/is_prime',
            'prime/count', 'prime/factor'
        ],
        'complex' => [
            'complex/create', 'complex/add', 'complex/subtract',
            'complex/multiply', 'complex/divide', 'complex/magnitude',
            'complex/phase', 'complex/conjugate'
        ],
        'abacus' => [
            'abacus/add', 'abacus/subtract', 'abacus/multiply'
        ],
        'clock' => [
            'clock/map_prime', 'clock/position_to_prime', 'clock/angular_distance'
        ]
    ];
    
    respond([
        'name' => 'Crystalline Math REST API',
        'version' => '1.0.0',
        'endpoints' => $endpoints,
        'usage' => 'POST to /api/math/{endpoint} with JSON body'
    ]);
}

// 404 - Endpoint not found
error('Endpoint not found: ' . $path, 404);
?>