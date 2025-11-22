# Crystalline Repository - Naming Conventions

## 📋 ESTABLISHED FILE NAMING PATTERNS

### Core Library Files

#### Pattern: `prime_<category>.c` / `prime_<category>.h`

**Geometry Module:**
- `prime_coords.c` / `prime_coords.h` - Coordinate transformations
- `prime_hyperdim.c` / `prime_hyperdim.h` - Hyperdimensional operations
- `prime_lattice.c` / `prime_lattice.h` - Basic lattice operations
- `prime_lattice_core.c` / `prime_lattice_core.h` - Core lattice functions
- `prime_lattice_geometry.c` / `prime_lattice_geometry.h` - Geometric lattice operations
- `prime_matrix.c` / `prime_matrix.h` - Matrix operations
- `prime_rainbow.c` / `prime_rainbow.h` - Rainbow/color operations

**Transcendental Module:**
- `prime_basic.c` / `prime_basic.h` - Basic math operations
- `prime_bigint_transcendental.c` / `prime_bigint_transcendental.h` - BigInt transcendental functions
- `prime_float_math.c` / `prime_float_math.h` - Float math operations
- `prime_math.c` / `prime_math.h` - General math operations
- `prime_math_custom.c` / `prime_math_custom.h` - Custom math implementations

**Core Module:**
- `prime_lowlevel.c` / `prime_lowlevel.h` - Low-level operations
- `prime_types.h` - Type definitions (header only)

#### Pattern: `bigint_<category>.c` / `bigint_<category>.h`

- `bigint_core.c` / `bigint_core.h` - Core BigInt operations
- `bigint_conversions.c` - BigInt conversion utilities

#### Pattern: `bigfixed_<category>.c` / `bigfixed_<category>.h`

- `bigfixed_core.c` / `bigfixed_core.h` - Core BigFixed operations
- `bigfixed_constants.c` / `bigfixed_constants.h` - BigFixed constants

#### Pattern: `crystal_<category>.c` / `crystal_<category>.h`

- `crystal_abacus.c` / `crystal_abacus.h` - Prime generation abacus

### CLLM (Neural Network) Files

#### Pattern: `cllm_<function>.c` / `cllm_<function>.h`

**Core CLLM:**
- `cllm.h` - Main CLLM header
- `cllm_attention.c` - Attention mechanism
- `cllm_backward.c` - Backward pass
- `cllm_benchmark.c` - Benchmarking utilities
- `cllm_create.c` - Model creation
- `cllm_embedding.c` - Embedding layer
- `cllm_feedforward.c` - Feedforward layer
- `cllm_format.c` / `cllm_format.h` - Format utilities
- `cllm_forward_complete.c` - Complete forward pass (LEGITIMATE - technical term)
- `cllm_inference.c` / `cllm_inference.h` - Inference engine
- `cllm_init.c` - Initialization
- `cllm_lattice_embed.c` - Lattice embedding
- `cllm_lattice_init.c` - Lattice initialization
- `cllm_layernorm.c` - Layer normalization
- `cllm_loss.c` - Loss calculation
- `cllm_optimizer.c` - Optimizer
- `cllm_positional.c` - Positional encoding
- `cllm_symmetry.c` - Symmetry operations
- `cllm_tokenizer.c` / `cllm_tokenizer.h` - Tokenizer
- `cllm_training.c` / `cllm_training.h` - Training utilities
- `cllm_utils.c` / `cllm_utils.h` - Utility functions
- `cllm_validate.c` - Validation

### Application Files

#### Pattern: `<descriptive_name>.c` / `<descriptive_name>.h`

**Main Application:**
- `main.c` / `main.h` - Main entry point
- `ui.c` - User interface
- `calculator.c` - Calculator functionality
- `visualization.c` - Visualization
- `io.c` / `io.h` - Input/output

**Lattice Utilities:**
- `lattice_cache.c` / `lattice_cache.h` - Lattice caching
- `lattice_helpers.c` / `lattice_helpers.h` - Lattice helper functions
- `lattice_utils.c` / `lattice_utils.h` - Lattice utilities
- `natural_prime_lattice.c` - Natural prime lattice

**Clock/Visualization:**
- `clock_4d.c` - 4D clock visualization
- `clock_abacus.c` - Clock abacus
- `clock_crystalline.c` - Crystalline clock
- `clock_folding.c` - Clock folding
- `nested_clocks.c` - Nested clocks
- `ulam_clock_spiral.c` - Ulam spiral clock

**Other:**
- `text_input.c` / `text_input.h` - Text input handling
- `color_utils.c` - Color utilities
- `frequency_rings.c` - Frequency ring visualization
- `ring_evolution.c` - Ring evolution
- `spheres.c` - Sphere visualization
- `prime_input.c` - Prime input handling
- `analysis_manager.c` - Analysis management
- `cllm_integration.c` - CLLM integration

## ❌ PROHIBITED NAMING PATTERNS

### DO NOT ADD ARBITRARY DESCRIPTIVE WORDS

**Examples of WRONG naming:**
- ❌ `prime_lattice_complete.c` - "complete" is arbitrary
- ❌ `prime_lattice_full.c` - "full" is arbitrary
- ❌ `prime_lattice_extended.c` - "extended" is arbitrary
- ❌ `prime_lattice_enhanced.c` - "enhanced" is arbitrary
- ❌ `prime_lattice_improved.c` - "improved" is arbitrary
- ❌ `prime_lattice_advanced.c` - "advanced" is arbitrary
- ❌ `prime_lattice_optimized.c` - "optimized" is arbitrary

**Exception:** Technical terms that are part of the domain
- ✅ `cllm_forward_complete.c` - "complete forward pass" is a neural network term
- ✅ `prime_lattice_core.c` - "core" distinguishes from basic lattice
- ✅ `prime_lattice_geometry.c` - "geometry" is a specific category

## ✅ CORRECT NAMING APPROACH

### If you need to add functionality:

1. **Check if it belongs in an existing file**
   - Add functions to `prime_lattice_core.c` if they're core lattice operations
   - Add to `prime_lattice_geometry.c` if they're geometric operations

2. **If creating a new category, use a clear category name**
   - ✅ `prime_lattice_quantum.c` - if adding quantum-specific operations
   - ✅ `prime_lattice_fractal.c` - if adding fractal-specific operations
   - ✅ `prime_lattice_topology.c` - if adding topology-specific operations

3. **Never use vague descriptive words**
   - The code should be complete, full, extended, enhanced by default
   - Don't create files that imply other files are incomplete

## 📝 FUNCTION NAMING CONVENTIONS

### BigInt Operations
- `big_*()` - Core BigInt operations (add, sub, mul, div, etc.)
- `big_prime_*()` - Prime-specific BigInt wrappers

### Prime Math Operations
- `prime_*()` - Standard math on doubles (for compatibility/UI)

### CLLM Operations
- `cllm_*()` - CLLM model operations

### Lattice Operations
- `lattice_*()` - General lattice operations
- `L_lattice()` - Master lattice formula
- `theta_n()` - Angular position
- `r_n()` - Radial position
- `O_exponent()` - Recursive growth exponent
- `nu_lambda()` - Phonetic values
- `pythagorean_*()` - Pythagorean triple operations

## 🔍 VERIFICATION CHECKLIST

Before adding any new file, verify:

1. ✅ Does the name follow an established pattern?
2. ✅ Is the category name specific and meaningful?
3. ✅ Does it avoid arbitrary descriptive words?
4. ✅ Would it be clear to another developer what this file contains?
5. ✅ Does it fit within the module structure (core/transcendental/geometry/ai)?

## 📚 REFERENCE

See `SYSTEM_SCHEMA.md` for type naming conventions and data flow patterns.