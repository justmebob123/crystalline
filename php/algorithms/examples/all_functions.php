<?php
/**
 * Complete Examples for All Crystalline Algorithms Functions
 * 
 * This file demonstrates every available function in the Crystalline Algorithms library.
 */

echo "=== CRYSTALLINE ALGORITHMS LIBRARY - COMPLETE FUNCTION EXAMPLES ===\n\n";

// ============================================================================
// NUMERICAL OPERATIONS
// ============================================================================
echo "--- NUMERICAL OPERATIONS ---\n";

// Softmax
$logits = [1.0, 2.0, 3.0, 4.0, 5.0];
$probs = numerical_softmax($logits);
echo "Softmax of [1,2,3,4,5]:\n";
print_r($probs);

// Log-softmax
$log_probs = numerical_log_softmax($logits);
echo "Log-softmax:\n";
print_r($log_probs);

// Safe operations
$safe_log = numerical_safe_log(0.0001, 1e-10);
echo "Safe log(0.0001) = $safe_log\n";

$safe_div = numerical_safe_divide(10.0, 0.0, 0.0);
echo "Safe divide(10/0) with default 0 = $safe_div\n";

// Clipping
$clipped = numerical_clip(150.0, 0.0, 100.0);
echo "Clip 150 to [0,100] = $clipped\n";

echo "\n";

// ============================================================================
// STATISTICS OPERATIONS
// ============================================================================
echo "--- STATISTICS OPERATIONS ---\n";

$data = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0];

// Mean
$mean = stats_mean($data);
echo "Mean of data: $mean\n";

// Variance
$variance = stats_variance($data, $mean);
echo "Variance: $variance\n";

// Standard deviation
$std_dev = stats_std_dev($data, $mean);
echo "Standard deviation: $std_dev\n";

// Median
$median = stats_median($data);
echo "Median: $median\n";

// Percentile
$p75 = stats_percentile($data, 75.0);
echo "75th percentile: $p75\n";

// Min/Max
$min = stats_min($data);
$max = stats_max($data);
echo "Min: $min, Max: $max\n";

// Correlation
$data2 = [2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0, 18.0, 20.0];
$corr = stats_correlation($data, $data2);
echo "Correlation between data and data2: $corr\n";

echo "\n";

// ============================================================================
// OPTIMIZER OPERATIONS
// ============================================================================
echo "--- OPTIMIZER OPERATIONS ---\n";

// Create optimizer configurations
$sgd_config = optimizer_config_create('sgd');
$sgd_config['learning_rate'] = 0.01;
echo "Created SGD optimizer with lr=0.01\n";

$adam_config = optimizer_config_create('adam');
$adam_config['learning_rate'] = 0.001;
$adam_config['beta1'] = 0.9;
$adam_config['beta2'] = 0.999;
echo "Created Adam optimizer with lr=0.001\n";

// Learning rate schedules
$lr_config = lr_schedule_config_create('cosine_annealing');
$lr_config['initial_lr'] = 0.1;
$lr_config['min_lr'] = 0.001;
$lr_config['T_max'] = 100;
echo "Created cosine annealing LR schedule\n";

// Get learning rate at step 50
$lr_at_50 = lr_schedule_get_lr($lr_config, 50);
echo "Learning rate at step 50: $lr_at_50\n";

echo "\n";

// ============================================================================
// GRADIENT OPERATIONS
// ============================================================================
echo "--- GRADIENT OPERATIONS ---\n";

// Create gradient buffer
$grad_buffer = gradient_buffer_create(100);
echo "Created gradient buffer with capacity 100\n";

// Accumulate gradients
$gradients = array_fill(0, 100, 0.1);
gradient_buffer_accumulate($grad_buffer, $gradients, 1.0);
echo "Accumulated gradients\n";

// Compute gradient statistics
$grad_stats = gradient_compute_stats($gradients);
echo "Gradient statistics:\n";
echo "  Mean: " . $grad_stats['mean'] . "\n";
echo "  Std: " . $grad_stats['std'] . "\n";
echo "  Max: " . $grad_stats['max'] . "\n";
echo "  Min: " . $grad_stats['min'] . "\n";

// Clip gradients
$clipped_count = gradient_clip_by_value($gradients, -1.0, 1.0);
echo "Clipped $clipped_count gradients to [-1, 1]\n";

echo "\n";

// ============================================================================
// BATCH PROCESSING OPERATIONS
// ============================================================================
echo "--- BATCH PROCESSING OPERATIONS ---\n";

// Create batch queue
$queue = batch_queue_create(10);
echo "Created batch queue with capacity 10\n";

// Create and enqueue batches
for ($i = 0; $i < 5; $i++) {
    $batch = batch_create($i, 32);
    batch_queue_enqueue($queue, $batch);
}
echo "Enqueued 5 batches\n";

// Get queue size
$size = batch_queue_size($queue);
echo "Queue size: $size\n";

// Dequeue batch
$batch = batch_queue_dequeue($queue);
echo "Dequeued batch with ID: " . $batch['id'] . "\n";

echo "\n";

// ============================================================================
// THREADING OPERATIONS
// ============================================================================
echo "--- THREADING OPERATIONS ---\n";

// Detect CPU cores
$num_cores = detect_num_cpu_cores();
echo "Detected $num_cores CPU cores\n";

// Get optimal thread count
$optimal_threads = get_optimal_thread_count(12);
echo "Optimal thread count for 12 work groups: $optimal_threads\n";

// Create thread allocation strategy
$strategy = thread_allocation_create($optimal_threads, 12);
echo "Created thread allocation strategy\n";

// Get work groups for thread
$work_groups = thread_allocation_get_work_groups($strategy, 0);
echo "Work groups for thread 0:\n";
print_r($work_groups);

// Calculate balance
$balance = thread_allocation_calculate_balance($strategy);
echo "Thread allocation balance: $balance\n";

echo "\n";

// ============================================================================
// MEMORY MANAGEMENT OPERATIONS
// ============================================================================
echo "--- MEMORY MANAGEMENT OPERATIONS ---\n";

// Create memory manager
$mem_manager = memory_create(1024 * 1024); // 1MB
echo "Created memory manager with 1MB capacity\n";

// Create memory pool
$pool_idx = memory_create_pool($mem_manager, 'embeddings', 64, 1000);
echo "Created memory pool 'embeddings' with 1000 blocks of 64 bytes\n";

// Allocate memory
$ptr = memory_alloc($mem_manager, $pool_idx, 64);
echo "Allocated 64 bytes from pool\n";

// Get statistics
$stats = memory_get_statistics($mem_manager);
echo "Memory statistics:\n";
echo "  Total allocated: " . $stats['total_allocated'] . " bytes\n";
echo "  Total freed: " . $stats['total_freed'] . " bytes\n";
echo "  Active allocations: " . $stats['active_allocations'] . "\n";

echo "\n";

// ============================================================================
// SPHERE PACKING OPERATIONS
// ============================================================================
echo "--- SPHERE PACKING OPERATIONS ---\n";

// Create spheres
$sphere1 = ['x' => 0.0, 'y' => 0.0, 'z' => 0.0, 'radius' => 1.0];
$sphere2 = ['x' => 2.0, 'y' => 0.0, 'z' => 0.0, 'radius' => 1.0];

// Check if kissing
$are_kissing = spheres_are_kissing($sphere1, $sphere2, 0.01);
echo "Spheres are kissing: " . ($are_kissing ? "yes" : "no") . "\n";

// Calculate distance
$distance = sphere_distance($sphere1, $sphere2);
echo "Distance between sphere centers: $distance\n";

// Calculate gap
$gap = sphere_gap($sphere1, $sphere2);
echo "Gap between spheres: $gap\n";

// Calculate volume
$volume = sphere_volume($sphere1);
echo "Volume of sphere1: $volume\n";

// Calculate surface area
$surface_area = sphere_surface_area($sphere1);
echo "Surface area of sphere1: $surface_area\n";

echo "\n";

// ============================================================================
// SPHERE THREADING OPERATIONS
// ============================================================================
echo "--- SPHERE THREADING OPERATIONS ---\n";

// Create sphere threading model
$model = sphere_threading_create(13); // 13 kissing spheres
echo "Created sphere threading model with 13 spheres\n";

// Get neighbors
$neighbors = sphere_get_neighbors($model, 0);
echo "Neighbors of sphere 0:\n";
print_r($neighbors);

// Optimize cache locality
$optimized = sphere_optimize_cache_locality($model);
echo "Optimized cache locality: " . ($optimized ? "success" : "failed") . "\n";

// Get statistics
$stats = sphere_get_statistics($model, 0);
echo "Statistics for sphere 0:\n";
echo "  Work assigned: " . $stats['work_assigned'] . "\n";
echo "  Work completed: " . $stats['work_completed'] . "\n";

echo "\n";

// ============================================================================
// NTT ATTENTION OPERATIONS
// ============================================================================
echo "--- NTT ATTENTION OPERATIONS ---\n";

// Create NTT attention config
$ntt_config = ntt_attention_config_init();
$ntt_config['threshold'] = 512;
$ntt_config['use_ntt'] = true;
echo "Created NTT attention config with threshold 512\n";

// Check if should use NTT
$should_use = should_use_ntt_attention(1024, $ntt_config);
echo "Should use NTT for seq_len=1024: " . ($should_use ? "yes" : "no") . "\n";

// Estimate speedup
$speedup = ntt_attention_estimate_speedup(1024);
echo "Estimated speedup for seq_len=1024: {$speedup}x\n";

echo "\n";

// ============================================================================
// VALIDATION OPERATIONS
// ============================================================================
echo "--- VALIDATION OPERATIONS ---\n";

// Check residual
$is_valid_residual = validation_check_residual(0.001, 0.01);
echo "Residual 0.001 < 0.01: " . ($is_valid_residual ? "valid" : "invalid") . "\n";

// Check entropy
$is_valid_entropy = validation_check_entropy(0.5, 0.1);
echo "Entropy 0.5 > 0.1: " . ($is_valid_entropy ? "valid" : "invalid") . "\n";

// Check range
$in_range = validation_check_range(5.0, 0.0, 10.0);
echo "Value 5.0 in [0, 10]: " . ($in_range ? "yes" : "no") . "\n";

// Calculate harm
$harm = validation_calculate_harm(0.1, 0.5, 0.8);
echo "Calculated harm: $harm\n";

// Safety probability
$safety_prob = validation_safety_probability(0.95, 0.05);
echo "Safety probability: $safety_prob\n";

echo "\n";

// ============================================================================
// VISUALIZATION OPERATIONS
// ============================================================================
echo "--- VISUALIZATION OPERATIONS ---\n";

// Create visualization data
$viz_data = viz_create(100, 13);
echo "Created visualization data for 100 points in 13D\n";

// Set point positions
for ($i = 0; $i < 10; $i++) {
    $position = array_fill(0, 13, $i * 0.1);
    viz_set_point_position($viz_data, $i, $position);
}
echo "Set positions for 10 points\n";

// Project to 2D
viz_project_2d($viz_data);
echo "Projected to 2D\n";

// Calculate statistics
$stats = viz_calculate_statistics($viz_data);
echo "Visualization statistics:\n";
echo "  Mean: " . $stats['mean'] . "\n";
echo "  Std: " . $stats['std'] . "\n";

// Export to JSON
viz_export_json($viz_data, 'visualization.json');
echo "Exported to visualization.json\n";

echo "\n";

// ============================================================================
// LOCK-FREE QUEUE OPERATIONS
// ============================================================================
echo "--- LOCK-FREE QUEUE OPERATIONS ---\n";

// Create lock-free queue
$lf_queue = lock_free_queue_create(100);
echo "Created lock-free queue with capacity 100\n";

// Enqueue items
for ($i = 0; $i < 10; $i++) {
    lock_free_queue_enqueue($lf_queue, ['id' => $i, 'data' => "item_$i"]);
}
echo "Enqueued 10 items\n";

// Get size
$lf_size = lock_free_queue_size($lf_queue);
echo "Queue size: $lf_size\n";

// Dequeue item
$item = lock_free_queue_dequeue($lf_queue);
echo "Dequeued item: " . $item['data'] . "\n";

// Get statistics
$lf_stats = lock_free_queue_get_statistics($lf_queue);
echo "Lock-free queue statistics:\n";
echo "  Enqueues: " . $lf_stats['enqueues'] . "\n";
echo "  Dequeues: " . $lf_stats['dequeues'] . "\n";
echo "  Utilization: " . $lf_stats['utilization'] . "%\n";

echo "\n";

// ============================================================================
// HIERARCHICAL PRIME OPERATIONS
// ============================================================================
echo "--- HIERARCHICAL PRIME OPERATIONS ---\n";

// Create hierarchical prime generator
$prime_gen = hierarchical_prime_create();
echo "Created hierarchical prime generator\n";

// Generate next prime
$next_prime = hierarchical_prime_next($prime_gen);
echo "Next prime: $next_prime\n";

// Check if prime
$is_prime = hierarchical_prime_is_prime($prime_gen, 17);
echo "Is 17 prime: " . ($is_prime ? "yes" : "no") . "\n";

// Get statistics
$prime_stats = hierarchical_prime_get_stats($prime_gen);
echo "Prime generator statistics:\n";
echo "  Primes generated: " . $prime_stats['primes_generated'] . "\n";
echo "  Cache hits: " . $prime_stats['cache_hits'] . "\n";
echo "  Cache misses: " . $prime_stats['cache_misses'] . "\n";

echo "\n";

// ============================================================================
// PLATONIC MODEL OPERATIONS
// ============================================================================
echo "--- PLATONIC MODEL OPERATIONS ---\n";

// Create platonic model
$platonic = platonic_model_create(1, 8, 12, 6); // Cube
echo "Created platonic model (Cube): V=8, E=12, F=6\n";

// Validate Euler's formula
$euler_valid = platonic_model_validate_euler($platonic);
echo "Euler's formula valid: " . ($euler_valid ? "yes" : "no") . "\n";

// Validate geometry
$geom_valid = platonic_model_validate_geometry($platonic);
echo "Geometry valid: " . ($geom_valid ? "yes" : "no") . "\n";

// Scale dimensions
$scaled = platonic_scale_dimensions($platonic, 2.0);
echo "Scaled dimensions by 2.0: " . ($scaled ? "success" : "failed") . "\n";

echo "\n";

// ============================================================================
// ANCHOR TRACKING OPERATIONS
// ============================================================================
echo "--- ANCHOR TRACKING OPERATIONS ---\n";

// Create anchor tracking system
$anchor_system = anchor_tracking_system_create(10);
echo "Created anchor tracking system for 10 anchors\n";

// Add anchor
$anchor = anchor_tracking_create(13);
$anchor_position = array_fill(0, 13, 0.5);
add_anchor_to_system($anchor_system, $anchor, $anchor_position);
echo "Added anchor to system\n";

// Update anchors
update_all_anchors($anchor_system);
echo "Updated all anchors\n";

// Check convergence
$converged = check_global_convergence($anchor_system);
echo "System converged: " . ($converged ? "yes" : "no") . "\n";

// Get global statistics
$global_stats = compute_global_statistics($anchor_system);
echo "Global statistics:\n";
echo "  Mean error: " . $global_stats['mean_error'] . "\n";
echo "  Max error: " . $global_stats['max_error'] . "\n";

echo "\n";

echo "=== ALL ALGORITHM FUNCTION EXAMPLES COMPLETE ===\n";
?>