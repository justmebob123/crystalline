<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Crystalline Math - O(1) Prime Generation</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }
        
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            overflow: hidden;
        }
        
        header {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            padding: 40px;
            text-align: center;
        }
        
        header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
        }
        
        header p {
            font-size: 1.2em;
            opacity: 0.9;
        }
        
        .content {
            padding: 40px;
        }
        
        .calculator {
            background: #f8f9fa;
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 40px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        
        .calculator h2 {
            color: #667eea;
            margin-bottom: 20px;
            font-size: 1.8em;
        }
        
        .calc-row {
            display: flex;
            gap: 20px;
            margin-bottom: 20px;
            flex-wrap: wrap;
        }
        
        .calc-group {
            flex: 1;
            min-width: 200px;
        }
        
        .calc-group label {
            display: block;
            margin-bottom: 8px;
            font-weight: 600;
            color: #333;
        }
        
        .calc-group input {
            width: 100%;
            padding: 12px;
            border: 2px solid #ddd;
            border-radius: 8px;
            font-size: 16px;
            transition: border-color 0.3s;
        }
        
        .calc-group input:focus {
            outline: none;
            border-color: #667eea;
        }
        
        .calc-buttons {
            display: flex;
            gap: 15px;
            flex-wrap: wrap;
        }
        
        button {
            padding: 12px 30px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 12px rgba(102, 126, 234, 0.4);
        }
        
        button:active {
            transform: translateY(0);
        }
        
        .result {
            margin-top: 20px;
            padding: 20px;
            background: white;
            border-radius: 8px;
            border-left: 4px solid #667eea;
            display: none;
        }
        
        .result.show {
            display: block;
            animation: slideIn 0.3s ease-out;
        }
        
        @keyframes slideIn {
            from {
                opacity: 0;
                transform: translateY(-10px);
            }
            to {
                opacity: 1;
                transform: translateY(0);
            }
        }
        
        .result h3 {
            color: #667eea;
            margin-bottom: 10px;
        }
        
        .result-value {
            font-size: 1.5em;
            font-weight: bold;
            color: #333;
            margin: 10px 0;
        }
        
        .examples {
            margin-top: 40px;
        }
        
        .example-section {
            background: #f8f9fa;
            border-radius: 15px;
            padding: 30px;
            margin-bottom: 30px;
        }
        
        .example-section h2 {
            color: #667eea;
            margin-bottom: 20px;
            font-size: 1.8em;
        }
        
        .example-section h3 {
            color: #764ba2;
            margin: 20px 0 10px 0;
            font-size: 1.3em;
        }
        
        .output {
            background: white;
            padding: 20px;
            border-radius: 8px;
            border-left: 4px solid #667eea;
            font-family: 'Courier New', monospace;
            line-height: 1.8;
            margin-top: 15px;
        }
        
        .output-line {
            margin: 5px 0;
        }
        
        .prime {
            color: #28a745;
            font-weight: bold;
        }
        
        .composite {
            color: #dc3545;
        }
        
        .info-box {
            background: #e3f2fd;
            border-left: 4px solid #2196f3;
            padding: 20px;
            border-radius: 8px;
            margin: 20px 0;
        }
        
        .info-box h4 {
            color: #1976d2;
            margin-bottom: 10px;
        }
        
        footer {
            background: #333;
            color: white;
            text-align: center;
            padding: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>🔢 Crystalline Math</h1>
            <p>O(1) Deterministic Prime Generation</p>
            <p style="font-size: 0.9em; margin-top: 10px;">Extension Version: <?php echo crystalline_version(); ?></p>
        </header>
        
        <div class="content">
            <!-- Interactive Calculator -->
            <div class="calculator">
                <h2>🧮 Interactive Prime Calculator</h2>
                
                <div class="calc-row">
                    <div class="calc-group">
                        <label for="checkNumber">Check if Prime:</label>
                        <input type="number" id="checkNumber" placeholder="Enter a number" value="157">
                    </div>
                    <div class="calc-group">
                        <label for="nthPrime">Get Nth Prime:</label>
                        <input type="number" id="nthPrime" placeholder="Enter N" value="10" min="1">
                    </div>
                </div>
                
                <div class="calc-row">
                    <div class="calc-group">
                        <label for="position">O(1) Position:</label>
                        <input type="number" id="position" placeholder="Position (0-11)" value="3" min="0" max="11">
                    </div>
                    <div class="calc-group">
                        <label for="magnitude">O(1) Magnitude:</label>
                        <input type="number" id="magnitude" placeholder="Magnitude" value="0" min="0">
                    </div>
                </div>
                
                <div class="calc-buttons">
                    <button onclick="checkPrime()">Check Prime</button>
                    <button onclick="getNthPrime()">Get Nth Prime</button>
                    <button onclick="generateO1()">Generate O(1)</button>
                </div>
                
                <div id="calcResult" class="result"></div>
            </div>
            
            <div class="info-box">
                <h4>ℹ️ About O(1) Prime Generation</h4>
                <p>This extension uses a revolutionary deterministic formula to generate primes in constant time O(1), without trial division or probabilistic tests. The clock lattice structure maps primes to positions on a Babylonian clock (12, 60, 60, 100).</p>
            </div>
            
            <!-- Examples -->
            <div class="examples">
                <div class="example-section">
                    <h2>📊 Live Examples</h2>
                    
                    <h3>1. O(1) Prime Generation</h3>
                    <div class="output">
                        <?php
                        $examples = [
                            [3, 0], [3, 1], [3, 2],
                            [6, 0], [6, 1],
                            [9, 0], [9, 1]
                        ];
                        foreach ($examples as list($pos, $mag)) {
                            $prime = crystalline_prime_generate_o1($pos, $mag);
                            echo "<div class='output-line'>";
                            echo "Position $pos, Magnitude $mag: ";
                            if ($prime > 0) {
                                echo "<span class='prime'>$prime (PRIME)</span>";
                            } else {
                                echo "<span class='composite'>composite</span>";
                            }
                            echo "</div>";
                        }
                        ?>
                    </div>
                    
                    <h3>2. Primality Testing</h3>
                    <div class="output">
                        <?php
                        $test_numbers = [2, 3, 4, 5, 17, 100, 157, 997, 1009];
                        foreach ($test_numbers as $n) {
                            $is_prime = crystalline_prime_is_prime($n);
                            echo "<div class='output-line'>";
                            echo "$n is ";
                            if ($is_prime) {
                                echo "<span class='prime'>PRIME</span>";
                            } else {
                                echo "<span class='composite'>composite</span>";
                            }
                            echo "</div>";
                        }
                        ?>
                    </div>
                    
                    <h3>3. First 20 Primes</h3>
                    <div class="output">
                        <?php
                        echo "<div class='output-line'>";
                        for ($i = 1; $i <= 20; $i++) {
                            $prime = crystalline_prime_nth($i);
                            echo "<span class='prime'>$prime</span>";
                            if ($i < 20) echo ", ";
                        }
                        echo "</div>";
                        ?>
                    </div>
                    
                    <h3>4. Clock Positions (Magnitude 0)</h3>
                    <div class="output">
                        <?php
                        for ($pos = 0; $pos < 12; $pos++) {
                            $prime = crystalline_prime_generate_o1($pos, 0);
                            echo "<div class='output-line'>";
                            echo "Position $pos: ";
                            if ($prime > 0) {
                                echo "<span class='prime'>$prime</span>";
                            } else {
                                echo "<span class='composite'>composite</span>";
                            }
                            echo "</div>";
                        }
                        ?>
                    </div>
                    
                    <h3>5. Performance Benchmark</h3>
                    <div class="output">
                        <?php
                        $start = microtime(true);
                        $count = 10000;
                        for ($i = 0; $i < $count; $i++) {
                            crystalline_prime_is_prime(rand(1, 100000));
                        }
                        $elapsed = microtime(true) - $start;
                        $per_second = $count / $elapsed;
                        ?>
                        <div class='output-line'>
                            Checked <?php echo number_format($count); ?> numbers in <?php echo number_format($elapsed, 4); ?> seconds
                        </div>
                        <div class='output-line'>
                            <span class='prime'>Rate: <?php echo number_format($per_second, 0); ?> checks/second</span>
                        </div>
                    </div>
                </div>
            </div>
        </div>
        
        <footer>
            <p>&copy; 2024 Crystalline Math | NinjaTech AI</p>
            <p style="font-size: 0.9em; margin-top: 5px;">Powered by O(1) Deterministic Prime Generation</p>
        </footer>
    </div>
    
    <script>
        function checkPrime() {
            const number = document.getElementById('checkNumber').value;
            if (!number) {
                alert('Please enter a number');
                return;
            }
            
            fetch('?action=check&number=' + number)
                .then(r => r.json())
                .then(data => {
                    const result = document.getElementById('calcResult');
                    result.innerHTML = `
                        <h3>Primality Test Result</h3>
                        <div class="result-value">
                            ${number} is <span class="${data.isPrime ? 'prime' : 'composite'}">${data.isPrime ? 'PRIME' : 'COMPOSITE'}</span>
                        </div>
                    `;
                    result.classList.add('show');
                });
        }
        
        function getNthPrime() {
            const n = document.getElementById('nthPrime').value;
            if (!n || n < 1) {
                alert('Please enter a positive number');
                return;
            }
            
            fetch('?action=nth&n=' + n)
                .then(r => r.json())
                .then(data => {
                    const result = document.getElementById('calcResult');
                    result.innerHTML = `
                        <h3>Nth Prime Result</h3>
                        <div class="result-value">
                            Prime #${n} = <span class="prime">${data.prime}</span>
                        </div>
                    `;
                    result.classList.add('show');
                });
        }
        
        function generateO1() {
            const position = document.getElementById('position').value;
            const magnitude = document.getElementById('magnitude').value;
            
            if (position === '' || magnitude === '') {
                alert('Please enter both position and magnitude');
                return;
            }
            
            fetch(`?action=o1&position=${position}&magnitude=${magnitude}`)
                .then(r => r.json())
                .then(data => {
                    const result = document.getElementById('calcResult');
                    result.innerHTML = `
                        <h3>O(1) Generation Result</h3>
                        <div class="result-value">
                            Position ${position}, Magnitude ${magnitude} = 
                            ${data.prime > 0 ? 
                                `<span class="prime">${data.prime} (PRIME)</span>` : 
                                `<span class="composite">composite</span>`
                            }
                        </div>
                        <p style="margin-top: 10px; color: #666;">
                            Generated in O(1) constant time using deterministic formula
                        </p>
                    `;
                    result.classList.add('show');
                });
        }
    </script>
</body>
</html>

<?php
// API endpoints for AJAX requests
if (isset($_GET['action'])) {
    header('Content-Type: application/json');
    
    switch ($_GET['action']) {
        case 'check':
            $number = intval($_GET['number']);
            $isPrime = crystalline_prime_is_prime($number);
            echo json_encode(['isPrime' => $isPrime]);
            exit;
            
        case 'nth':
            $n = intval($_GET['n']);
            $prime = crystalline_prime_nth($n);
            echo json_encode(['prime' => $prime]);
            exit;
            
        case 'o1':
            $position = intval($_GET['position']);
            $magnitude = intval($_GET['magnitude']);
            $prime = crystalline_prime_generate_o1($position, $magnitude);
            echo json_encode(['prime' => $prime]);
            exit;
    }
}
?>