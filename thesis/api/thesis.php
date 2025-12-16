<?php
/**
 * Thesis REST API
 * Scans thesis directory structure and serves content
 */

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: GET, POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// Handle preflight requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    exit(0);
}

// Base directory (one level up from api/)
$baseDir = dirname(__DIR__);

/**
 * Get directory structure
 */
function getStructure($dir, $basePath = '') {
    $structure = [];
    
    if (!is_dir($dir)) {
        return $structure;
    }
    
    $items = scandir($dir);
    
    foreach ($items as $item) {
        if ($item === '.' || $item === '..' || $item === 'api' || $item === 'archive') {
            continue;
        }
        
        $fullPath = $dir . '/' . $item;
        $relativePath = $basePath ? $basePath . '/' . $item : $item;
        
        if (is_dir($fullPath)) {
            // Check if it's a part or chapter directory
            if (preg_match('/^part_\d+_/', $item) || preg_match('/^chapter_\d+_/', $item)) {
                $structure[] = [
                    'type' => 'directory',
                    'name' => $item,
                    'path' => $relativePath,
                    'displayName' => formatName($item),
                    'children' => getStructure($fullPath, $relativePath)
                ];
            }
        } elseif (pathinfo($item, PATHINFO_EXTENSION) === 'md') {
            $structure[] = [
                'type' => 'file',
                'name' => $item,
                'path' => $relativePath,
                'displayName' => formatName($item),
                'size' => filesize($fullPath)
            ];
        }
    }
    
    // Sort: directories first, then files
    usort($structure, function($a, $b) {
        if ($a['type'] === $b['type']) {
            return strcmp($a['name'], $b['name']);
        }
        return $a['type'] === 'directory' ? -1 : 1;
    });
    
    return $structure;
}

/**
 * Format directory/file names for display
 */
function formatName($name) {
    // Remove .md extension
    $name = preg_replace('/\.md$/', '', $name);
    
    // Replace underscores with spaces
    $name = str_replace('_', ' ', $name);
    
    // Capitalize words
    $name = ucwords($name);
    
    // Handle special cases
    $name = str_replace('Qa', 'Q&A', $name);
    $name = str_replace('Ntt', 'NTT', $name);
    $name = str_replace('Ai', 'AI', $name);
    $name = str_replace('88d', '88D', $name);
    
    return $name;
}

/**
 * Get file content
 */
function getContent($path) {
    global $baseDir;
    
    $fullPath = $baseDir . '/' . $path;
    
    // Security check: ensure path is within base directory
    $realPath = realpath($fullPath);
    $realBase = realpath($baseDir);
    
    if ($realPath === false || strpos($realPath, $realBase) !== 0) {
        return ['error' => 'Invalid path'];
    }
    
    if (!file_exists($fullPath)) {
        return ['error' => 'File not found'];
    }
    
    $content = file_get_contents($fullPath);
    $stats = [
        'size' => filesize($fullPath),
        'modified' => filemtime($fullPath),
        'lines' => substr_count($content, "\n") + 1
    ];
    
    return [
        'content' => $content,
        'stats' => $stats,
        'path' => $path
    ];
}

/**
 * Search content
 */
function searchContent($query) {
    global $baseDir;
    
    $results = [];
    $query = strtolower($query);
    
    $iterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($baseDir)
    );
    
    foreach ($iterator as $file) {
        if ($file->isFile() && $file->getExtension() === 'md') {
            $relativePath = str_replace($baseDir . '/', '', $file->getPathname());
            
            // Skip archive directory
            if (strpos($relativePath, 'archive/') === 0) {
                continue;
            }
            
            $content = file_get_contents($file->getPathname());
            $lowerContent = strtolower($content);
            
            if (strpos($lowerContent, $query) !== false) {
                // Find context around matches
                $lines = explode("\n", $content);
                $matches = [];
                
                foreach ($lines as $lineNum => $line) {
                    if (stripos($line, $query) !== false) {
                        $matches[] = [
                            'line' => $lineNum + 1,
                            'text' => trim($line),
                            'context' => array_slice($lines, max(0, $lineNum - 1), 3)
                        ];
                        
                        if (count($matches) >= 3) break; // Limit matches per file
                    }
                }
                
                if (!empty($matches)) {
                    $results[] = [
                        'file' => $relativePath,
                        'displayName' => formatName(basename($relativePath)),
                        'matches' => $matches
                    ];
                }
            }
        }
    }
    
    return $results;
}

/**
 * Get statistics
 */
function getStats() {
    global $baseDir;
    
    $stats = [
        'totalFiles' => 0,
        'totalLines' => 0,
        'totalSize' => 0,
        'chapters' => 0,
        'parts' => 0
    ];
    
    $iterator = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($baseDir)
    );
    
    foreach ($iterator as $file) {
        if ($file->isFile() && $file->getExtension() === 'md') {
            $relativePath = str_replace($baseDir . '/', '', $file->getPathname());
            
            // Skip archive
            if (strpos($relativePath, 'archive/') === 0) {
                continue;
            }
            
            $stats['totalFiles']++;
            $stats['totalSize'] += $file->getSize();
            
            $content = file_get_contents($file->getPathname());
            $stats['totalLines'] += substr_count($content, "\n") + 1;
            
            if (strpos($relativePath, 'chapter_') !== false) {
                $stats['chapters']++;
            }
            if (strpos($relativePath, 'part_') !== false && basename($relativePath) === 'README.md') {
                $stats['parts']++;
            }
        }
    }
    
    return $stats;
}

// Route handling
$action = $_GET['action'] ?? 'structure';

try {
    switch ($action) {
        case 'structure':
            $response = [
                'success' => true,
                'data' => getStructure($baseDir)
            ];
            break;
            
        case 'content':
            $path = $_GET['path'] ?? '';
            if (empty($path)) {
                throw new Exception('Path parameter required');
            }
            $response = [
                'success' => true,
                'data' => getContent($path)
            ];
            break;
            
        case 'search':
            $query = $_GET['query'] ?? '';
            if (empty($query)) {
                throw new Exception('Query parameter required');
            }
            $response = [
                'success' => true,
                'data' => searchContent($query)
            ];
            break;
            
        case 'stats':
            $response = [
                'success' => true,
                'data' => getStats()
            ];
            break;
            
        default:
            throw new Exception('Invalid action');
    }
} catch (Exception $e) {
    $response = [
        'success' => false,
        'error' => $e->getMessage()
    ];
}

echo json_encode($response, JSON_PRETTY_PRINT);