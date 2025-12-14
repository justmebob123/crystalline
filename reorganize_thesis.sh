#!/bin/bash

# Script to systematically reorganize THESIS.md
# This will extract sections, reorganize them, and create a new comprehensive document

ORIGINAL="THESIS.md"
OUTPUT="THESIS_NEW.md"
TEMP_DIR="/tmp/thesis_sections"

# Create temp directory
mkdir -p $TEMP_DIR

echo "Extracting sections from $ORIGINAL..."

# Extract each major section by line numbers
# Section 1: Introduction (lines 42-69)
sed -n '42,69p' $ORIGINAL > $TEMP_DIR/01_introduction.txt

# Section 2: Babylonian Math (lines 70-478)
sed -n '70,478p' $ORIGINAL > $TEMP_DIR/02_babylonian.txt

# Section 3: Ancient Proverb (lines 479-1015)
sed -n '479,1015p' $ORIGINAL > $TEMP_DIR/03_proverb.txt

# Section 4: Clock Lattice (lines 1016-1085)
sed -n '1016,1085p' $ORIGINAL > $TEMP_DIR/04_clock_lattice.txt

# Section 5: Prime Generation (lines 1086-1594)
sed -n '1086,1594p' $ORIGINAL > $TEMP_DIR/05_prime_generation.txt

# Section 6: Clock Triangle (lines 1595-1668)
sed -n '1595,1668p' $ORIGINAL > $TEMP_DIR/06_clock_triangle.txt

# Section 7: Crystalline Abacus (lines 1669-1734)
sed -n '1669,1734p' $ORIGINAL > $TEMP_DIR/07_abacus.txt

# Section 8: Arithmetic Operations (lines 1735-2549)
sed -n '1735,2549p' $ORIGINAL > $TEMP_DIR/08_arithmetic.txt

# Section 9: Kissing Spheres (lines 2550-2605)
sed -n '2550,2605p' $ORIGINAL > $TEMP_DIR/09_kissing_spheres.txt

# Section 10: Platonic Solids (lines 2606-3238)
sed -n '2606,3238p' $ORIGINAL > $TEMP_DIR/10_platonic.txt

# Section 11: Framework Formula (lines 3239-3372)
sed -n '3239,3372p' $ORIGINAL > $TEMP_DIR/11_framework.txt

# Section 12: Plimpton 322 (lines 3373-3416)
sed -n '3373,3416p' $ORIGINAL > $TEMP_DIR/12_plimpton.txt

# Section 13: Cymatic (lines 3417-3473)
sed -n '3417,3473p' $ORIGINAL > $TEMP_DIR/13_cymatic.txt

# Section 14: Astronomical (lines 3474-3520)
sed -n '3474,3520p' $ORIGINAL > $TEMP_DIR/14_astronomical.txt

# Section 15: CLLM Architecture (lines 3521-3592)
sed -n '3521,3592p' $ORIGINAL > $TEMP_DIR/15_cllm_arch.txt

# Section 16: Implementation (lines 3593-3634)
sed -n '3593,3634p' $ORIGINAL > $TEMP_DIR/16_implementation.txt

# Section 17: Performance (lines 3635-3676)
sed -n '3635,3676p' $ORIGINAL > $TEMP_DIR/17_performance.txt

# Section 18: Conclusions (lines 3677-3740) - FIRST ONE
sed -n '3677,3740p' $ORIGINAL > $TEMP_DIR/18_conclusions_old.txt

# Section 19: References (lines 3741-3808)
sed -n '3741,3808p' $ORIGINAL > $TEMP_DIR/19_references.txt

# Appendices (lines 3809-4093) - THESE SHOULD BE AT END
sed -n '3809,3885p' $ORIGINAL > $TEMP_DIR/appendix_a.txt
sed -n '3886,3997p' $ORIGINAL > $TEMP_DIR/appendix_b.txt
sed -n '3998,4039p' $ORIGINAL > $TEMP_DIR/appendix_c.txt
sed -n '4040,4093p' $ORIGINAL > $TEMP_DIR/appendix_d.txt

# Section 20: Memory Hopping (lines 4094-4404)
sed -n '4094,4404p' $ORIGINAL > $TEMP_DIR/20_memory_hopping.txt

# Section 21: NTT Attention (lines 4405-4766)
sed -n '4405,4766p' $ORIGINAL > $TEMP_DIR/21_ntt_attention.txt

# Section 22: Formula Library (lines 4767-5222)
sed -n '4767,5222p' $ORIGINAL > $TEMP_DIR/22_formula_library.txt

# Section 23: 4-Layer Architecture (lines 5223-5665)
sed -n '5223,5665p' $ORIGINAL > $TEMP_DIR/23_architecture.txt

# Section 24: Testing (lines 5666-6142)
sed -n '5666,6142p' $ORIGINAL > $TEMP_DIR/24_testing.txt

# Section 25: Conclusions (lines 6143-6230) - SECOND ONE (DUPLICATE)
sed -n '6143,6230p' $ORIGINAL > $TEMP_DIR/25_conclusions_new.txt

# Section 26: Pi Phi (lines 6231-6711)
sed -n '6231,6711p' $ORIGINAL > $TEMP_DIR/26_pi_phi.txt

# Section 27: System Integration (lines 6712-7381)
sed -n '6712,7381p' $ORIGINAL > $TEMP_DIR/27_system_integration.txt

echo "Extraction complete. Files in $TEMP_DIR"
ls -lh $TEMP_DIR/

