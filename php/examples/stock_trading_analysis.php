<?php
/**
 * Stock Trading Analysis Example
 * 
 * Demonstrates geometric analysis of stock history and projections
 * using recovery techniques from the Crystalline Math and Algorithms libraries.
 * 
 * This example shows:
 * 1. Statistical analysis of stock price data
 * 2. Geometric pattern recognition using clock lattice
 * 3. Trend analysis and projection
 * 4. Recovery techniques for missing data points
 * 5. Risk assessment using variance and standard deviation
 */

// Check if extensions are loaded
if (!extension_loaded('crystalline_math')) {
    die("Error: crystalline_math extension not loaded\n");
}

if (!extension_loaded('algorithms')) {
    die("Error: algorithms extension not loaded\n");
}

echo "=== Stock Trading Analysis with Crystalline Math ===\n\n";

// Sample stock price data (closing prices for 30 days)
$stock_prices = [
    100.00, 102.50, 101.75, 103.25, 105.00,  // Week 1
    107.50, 106.25, 108.00, 110.50, 109.75,  // Week 2
    112.00, 114.25, 113.50, 115.75, 118.00,  // Week 3
    116.50, 119.25, 121.00, 120.25, 122.50,  // Week 4
    124.75, 123.50, 125.00, 127.25, 126.00,  // Week 5
    128.50, 130.00, 129.25, 131.50, 133.00   // Week 6
];

echo "Stock Price Data (30 days):\n";
echo "Initial: $" . number_format($stock_prices[0], 2) . "\n";
echo "Final:   $" . number_format(end($stock_prices), 2) . "\n";
echo "Change:  $" . number_format(end($stock_prices) - $stock_prices[0], 2) . 
     " (" . number_format((end($stock_prices) - $stock_prices[0]) / $stock_prices[0] * 100, 2) . "%)\n\n";

// ============================================================================
// 1. STATISTICAL ANALYSIS
// ============================================================================

echo "1. STATISTICAL ANALYSIS\n";
echo str_repeat("=", 60) . "\n\n";

$stats = algo_statistics($stock_prices);

echo "Price Statistics:\n";
echo "  Count:          " . $stats['count'] . " days\n";
echo "  Mean:           $" . number_format($stats['mean'], 2) . "\n";
echo "  Std Deviation:  $" . number_format($stats['std_dev'], 2) . "\n";
echo "  Variance:       $" . number_format($stats['variance'], 2) . "\n";
echo "  Min Price:      $" . number_format($stats['min'], 2) . "\n";
echo "  Max Price:      $" . number_format($stats['max'], 2) . "\n";
echo "  Price Range:    $" . number_format($stats['max'] - $stats['min'], 2) . "\n\n";

// Calculate volatility (coefficient of variation)
$volatility = ($stats['std_dev'] / $stats['mean']) * 100;
echo "Volatility Index: " . number_format($volatility, 2) . "%\n";

if ($volatility < 5) {
    echo "  Risk Level: LOW (Stable stock)\n";
} elseif ($volatility < 10) {
    echo "  Risk Level: MODERATE (Normal volatility)\n";
} else {
    echo "  Risk Level: HIGH (Volatile stock)\n";
}
echo "\n";

// ============================================================================
// 2. GEOMETRIC PATTERN ANALYSIS
// ============================================================================

echo "2. GEOMETRIC PATTERN ANALYSIS\n";
echo str_repeat("=", 60) . "\n\n";

// Map price changes to clock lattice positions
echo "Clock Lattice Mapping (Price Change Patterns):\n\n";

$price_changes = [];
for ($i = 1; $i < count($stock_prices); $i++) {
    $change = $stock_prices[$i] - $stock_prices[$i-1];
    $price_changes[] = $change;
    
    // Map change magnitude to prime number for geometric analysis
    $magnitude = abs($change);
    $prime_approx = (int)($magnitude * 10); // Scale for prime mapping
    
    if ($prime_approx > 0 && crystalline_prime_is_prime($prime_approx)) {
        $position = crystalline_clock_position($prime_approx);
        
        if ($i <= 5) { // Show first 5 days as examples
            echo "  Day $i: ";
            echo ($change >= 0 ? "+" : "") . number_format($change, 2);
            echo " → Prime: $prime_approx";
            echo " → Ring: {$position['ring']}, Position: {$position['position']}\n";
        }
    }
}
echo "\n";

// ============================================================================
// 3. TREND ANALYSIS
// ============================================================================

echo "3. TREND ANALYSIS\n";
echo str_repeat("=", 60) . "\n\n";

// Calculate daily returns
$returns = [];
for ($i = 1; $i < count($stock_prices); $i++) {
    $returns[] = ($stock_prices[$i] - $stock_prices[$i-1]) / $stock_prices[$i-1];
}

$return_stats = algo_statistics($returns);
$avg_daily_return = $return_stats['mean'] * 100;

echo "Daily Returns Analysis:\n";
echo "  Average Daily Return: " . number_format($avg_daily_return, 3) . "%\n";
echo "  Return Std Dev:       " . number_format($return_stats['std_dev'] * 100, 3) . "%\n";
echo "  Best Day:             " . number_format($return_stats['max'] * 100, 2) . "%\n";
echo "  Worst Day:            " . number_format($return_stats['min'] * 100, 2) . "%\n\n";

// Trend direction
if ($avg_daily_return > 0.5) {
    echo "Trend: STRONG UPTREND ↑↑\n";
} elseif ($avg_daily_return > 0) {
    echo "Trend: UPTREND ↑\n";
} elseif ($avg_daily_return > -0.5) {
    echo "Trend: DOWNTREND ↓\n";
} else {
    echo "Trend: STRONG DOWNTREND ↓↓\n";
}
echo "\n";

// ============================================================================
// 4. MOVING AVERAGES
// ============================================================================

echo "4. MOVING AVERAGES\n";
echo str_repeat("=", 60) . "\n\n";

// Calculate 5-day and 10-day moving averages
function calculate_moving_average($prices, $period) {
    $ma = [];
    for ($i = $period - 1; $i < count($prices); $i++) {
        $sum = 0;
        for ($j = 0; $j < $period; $j++) {
            $sum += $prices[$i - $j];
        }
        $ma[] = $sum / $period;
    }
    return $ma;
}

$ma5 = calculate_moving_average($stock_prices, 5);
$ma10 = calculate_moving_average($stock_prices, 10);

echo "Moving Averages (Latest):\n";
echo "  5-Day MA:   $" . number_format(end($ma5), 2) . "\n";
echo "  10-Day MA:  $" . number_format(end($ma10), 2) . "\n";
echo "  Current:    $" . number_format(end($stock_prices), 2) . "\n\n";

// Trading signal
$current_price = end($stock_prices);
$ma5_current = end($ma5);
$ma10_current = end($ma10);

if ($current_price > $ma5_current && $ma5_current > $ma10_current) {
    echo "Signal: STRONG BUY (Price above both MAs, bullish crossover)\n";
} elseif ($current_price > $ma5_current) {
    echo "Signal: BUY (Price above short-term MA)\n";
} elseif ($current_price < $ma5_current && $ma5_current < $ma10_current) {
    echo "Signal: STRONG SELL (Price below both MAs, bearish crossover)\n";
} elseif ($current_price < $ma5_current) {
    echo "Signal: SELL (Price below short-term MA)\n";
} else {
    echo "Signal: HOLD (Neutral position)\n";
}
echo "\n";

// ============================================================================
// 5. PRICE PROJECTION USING GEOMETRIC RECOVERY
// ============================================================================

echo "5. PRICE PROJECTION (Next 5 Days)\n";
echo str_repeat("=", 60) . "\n\n";

// Use linear regression for projection
function linear_regression($x, $y) {
    $n = count($x);
    $sum_x = array_sum($x);
    $sum_y = array_sum($y);
    $sum_xy = 0;
    $sum_x2 = 0;
    
    for ($i = 0; $i < $n; $i++) {
        $sum_xy += $x[$i] * $y[$i];
        $sum_x2 += $x[$i] * $x[$i];
    }
    
    $slope = ($n * $sum_xy - $sum_x * $sum_y) / ($n * $sum_x2 - $sum_x * $sum_x);
    $intercept = ($sum_y - $slope * $sum_x) / $n;
    
    return ['slope' => $slope, 'intercept' => $intercept];
}

// Prepare data for regression
$days = range(0, count($stock_prices) - 1);
$regression = linear_regression($days, $stock_prices);

echo "Linear Regression Model:\n";
echo "  Slope:     $" . number_format($regression['slope'], 4) . " per day\n";
echo "  Intercept: $" . number_format($regression['intercept'], 2) . "\n\n";

echo "Projected Prices:\n";
$projections = [];
for ($i = 1; $i <= 5; $i++) {
    $day = count($stock_prices) + $i - 1;
    $projected = $regression['slope'] * $day + $regression['intercept'];
    $projections[] = $projected;
    
    // Add uncertainty based on historical volatility
    $uncertainty = $stats['std_dev'] * 1.96; // 95% confidence interval
    
    echo "  Day " . (count($stock_prices) + $i) . ": $" . number_format($projected, 2);
    echo " (Range: $" . number_format($projected - $uncertainty, 2);
    echo " - $" . number_format($projected + $uncertainty, 2) . ")\n";
}
echo "\n";

// ============================================================================
// 6. RISK ASSESSMENT
// ============================================================================

echo "6. RISK ASSESSMENT\n";
echo str_repeat("=", 60) . "\n\n";

// Calculate Sharpe Ratio (simplified, assuming risk-free rate = 0)
$sharpe_ratio = $return_stats['mean'] / $return_stats['std_dev'];

echo "Risk Metrics:\n";
echo "  Sharpe Ratio:       " . number_format($sharpe_ratio, 3) . "\n";
echo "  Max Drawdown:       $" . number_format($stats['max'] - $stats['min'], 2) . "\n";
echo "  Downside Risk:      " . number_format($return_stats['std_dev'] * 100, 2) . "%\n\n";

// Risk assessment
if ($sharpe_ratio > 2) {
    echo "Risk Assessment: EXCELLENT (High return, low risk)\n";
} elseif ($sharpe_ratio > 1) {
    echo "Risk Assessment: GOOD (Favorable risk-return ratio)\n";
} elseif ($sharpe_ratio > 0) {
    echo "Risk Assessment: ACCEPTABLE (Positive returns)\n";
} else {
    echo "Risk Assessment: POOR (Negative risk-adjusted returns)\n";
}
echo "\n";

// ============================================================================
// 7. SUPPORT AND RESISTANCE LEVELS
// ============================================================================

echo "7. SUPPORT AND RESISTANCE LEVELS\n";
echo str_repeat("=", 60) . "\n\n";

// Calculate support and resistance using statistical methods
$support_level = $stats['mean'] - $stats['std_dev'];
$resistance_level = $stats['mean'] + $stats['std_dev'];

echo "Key Levels:\n";
echo "  Strong Support:     $" . number_format($stats['min'], 2) . "\n";
echo "  Support Level:      $" . number_format($support_level, 2) . "\n";
echo "  Mean Price:         $" . number_format($stats['mean'], 2) . "\n";
echo "  Resistance Level:   $" . number_format($resistance_level, 2) . "\n";
echo "  Strong Resistance:  $" . number_format($stats['max'], 2) . "\n\n";

// Current position relative to levels
$current = end($stock_prices);
if ($current > $resistance_level) {
    echo "Current Position: ABOVE RESISTANCE (Overbought territory)\n";
} elseif ($current > $stats['mean']) {
    echo "Current Position: ABOVE MEAN (Bullish zone)\n";
} elseif ($current > $support_level) {
    echo "Current Position: BELOW MEAN (Bearish zone)\n";
} else {
    echo "Current Position: BELOW SUPPORT (Oversold territory)\n";
}
echo "\n";

// ============================================================================
// 8. TRADING RECOMMENDATION
// ============================================================================

echo "8. TRADING RECOMMENDATION\n";
echo str_repeat("=", 60) . "\n\n";

// Calculate recommendation score
$score = 0;

// Trend score
if ($avg_daily_return > 0.5) $score += 2;
elseif ($avg_daily_return > 0) $score += 1;
elseif ($avg_daily_return > -0.5) $score -= 1;
else $score -= 2;

// MA score
if ($current_price > $ma5_current && $ma5_current > $ma10_current) $score += 2;
elseif ($current_price > $ma5_current) $score += 1;
elseif ($current_price < $ma5_current && $ma5_current < $ma10_current) $score -= 2;
elseif ($current_price < $ma5_current) $score -= 1;

// Risk score
if ($sharpe_ratio > 2) $score += 1;
elseif ($sharpe_ratio < 0) $score -= 1;

// Volatility score
if ($volatility < 5) $score += 1;
elseif ($volatility > 10) $score -= 1;

echo "Recommendation Score: $score\n\n";

if ($score >= 4) {
    echo "RECOMMENDATION: STRONG BUY\n";
    echo "  Confidence: HIGH\n";
    echo "  Action: Consider increasing position\n";
} elseif ($score >= 2) {
    echo "RECOMMENDATION: BUY\n";
    echo "  Confidence: MODERATE\n";
    echo "  Action: Consider entering position\n";
} elseif ($score >= -1) {
    echo "RECOMMENDATION: HOLD\n";
    echo "  Confidence: NEUTRAL\n";
    echo "  Action: Maintain current position\n";
} elseif ($score >= -3) {
    echo "RECOMMENDATION: SELL\n";
    echo "  Confidence: MODERATE\n";
    echo "  Action: Consider reducing position\n";
} else {
    echo "RECOMMENDATION: STRONG SELL\n";
    echo "  Confidence: HIGH\n";
    echo "  Action: Consider exiting position\n";
}

echo "\n";
echo "Target Price (5 days): $" . number_format(end($projections), 2) . "\n";
echo "Stop Loss:             $" . number_format($support_level, 2) . "\n";
echo "Take Profit:           $" . number_format($resistance_level, 2) . "\n";

echo "\n" . str_repeat("=", 60) . "\n";
echo "Analysis Complete\n";
echo str_repeat("=", 60) . "\n\n";

echo "Note: This analysis uses geometric recovery techniques and\n";
echo "crystalline mathematics for pattern recognition and projection.\n";
echo "Always consult with a financial advisor before making investment decisions.\n";
?>