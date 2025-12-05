/**
 * @file cllm_cymatic_frequencies.h
 * @brief Cymatic frequency integration for crystalline CLLM
 * 
 * Implements cymatic frequency corrections ω for the angular position formula.
 * These frequencies represent natural resonances found in nature and ancient
 * musical tuning systems.
 * 
 * Mathematical Foundation:
 * ω_correction(ω) = sin(2π·ω/432) for frequency-based angular corrections
 * 
 * Key Frequencies:
 * - 432 Hz: "Verdi's A" - natural tuning frequency
 * - 528 Hz: "Love frequency" - DNA repair frequency
 * - 7.83 Hz: Schumann resonance - Earth's electromagnetic frequency
 * - 40 Hz: Gamma brainwave frequency - consciousness binding
 */

#ifndef CLLM_CYMATIC_FREQUENCIES_H
#define CLLM_CYMATIC_FREQUENCIES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Cymatic frequency constants (in Hz)
 */
#define FREQ_432_HZ         432.0    /**< Verdi's A - natural tuning */
#define FREQ_528_HZ         528.0    /**< Love frequency - DNA repair */
#define FREQ_SCHUMANN       7.83     /**< Earth's electromagnetic resonance */
#define FREQ_GAMMA          40.0     /**< Gamma brainwave - consciousness */
#define FREQ_ALPHA          10.0     /**< Alpha brainwave - relaxation */
#define FREQ_THETA          6.0      /**< Theta brainwave - meditation */
#define FREQ_DELTA          2.0      /**< Delta brainwave - deep sleep */
#define FREQ_BETA           20.0     /**< Beta brainwave - active thinking */

/**
 * @brief Solfeggio frequencies (ancient musical scale)
 */
#define FREQ_SOLFEGGIO_UT   396.0    /**< Liberating guilt and fear */
#define FREQ_SOLFEGGIO_RE   417.0    /**< Undoing situations and facilitating change */
#define FREQ_SOLFEGGIO_MI   528.0    /**< Transformation and miracles (DNA repair) */
#define FREQ_SOLFEGGIO_FA   639.0    /**< Connecting relationships */
#define FREQ_SOLFEGGIO_SOL  741.0    /**< Awakening intuition */
#define FREQ_SOLFEGGIO_LA   852.0    /**< Returning to spiritual order */

/**
 * @brief Reference frequency for normalization
 */
#define FREQ_REFERENCE      FREQ_432_HZ

/**
 * @brief Cymatic frequency type enumeration
 */
typedef enum {
    CYMATIC_FREQ_432_HZ = 0,
    CYMATIC_FREQ_528_HZ,
    CYMATIC_FREQ_SCHUMANN,
    CYMATIC_FREQ_GAMMA,
    CYMATIC_FREQ_ALPHA,
    CYMATIC_FREQ_THETA,
    CYMATIC_FREQ_DELTA,
    CYMATIC_FREQ_BETA,
    CYMATIC_FREQ_CUSTOM
} CymaticFrequencyType;

/**
 * @brief Cymatic frequency configuration
 */
typedef struct {
    CymaticFrequencyType type;    /**< Frequency type */
    double frequency;             /**< Frequency value in Hz */
    double amplitude;             /**< Amplitude factor (0.0 to 1.0) */
    double phase;                 /**< Phase offset in radians */
} CymaticFrequency;

/**
 * @brief Calculate cymatic correction for angular position
 * 
 * Computes the angular correction based on cymatic frequency resonance.
 * Formula: ω_correction = amplitude * sin(2π·ω/432 + phase)
 * 
 * @param omega Frequency in Hz
 * @param amplitude Amplitude factor (default 1.0)
 * @param phase Phase offset in radians (default 0.0)
 * @return Angular correction value (typically small, < 0.1)
 */
double cymatic_correction(double omega, double amplitude, double phase);

/**
 * @brief Calculate cymatic correction from frequency configuration
 * 
 * @param config Cymatic frequency configuration
 * @return Angular correction value
 */
double cymatic_correction_from_config(const CymaticFrequency *config);

/**
 * @brief Get predefined cymatic frequency configuration
 * 
 * @param type Frequency type
 * @return Frequency configuration
 */
CymaticFrequency cymatic_get_frequency_config(CymaticFrequencyType type);

/**
 * @brief Calculate resonance factor between two frequencies
 * 
 * Measures how well two frequencies resonate with each other.
 * Returns value in range [0, 1] where 1 is perfect resonance.
 * 
 * @param freq1 First frequency in Hz
 * @param freq2 Second frequency in Hz
 * @return Resonance factor (0.0 to 1.0)
 */
double cymatic_resonance_factor(double freq1, double freq2);

/**
 * @brief Calculate harmonic series for a base frequency
 * 
 * Generates the harmonic series (overtones) for a given base frequency.
 * 
 * @param base_freq Base frequency in Hz
 * @param harmonic_number Harmonic number (1 = fundamental, 2 = first overtone, etc.)
 * @return Harmonic frequency
 */
double cymatic_harmonic(double base_freq, uint32_t harmonic_number);

/**
 * @brief Check if frequency is in natural harmonic series
 * 
 * Determines if a frequency is a natural harmonic of 432 Hz.
 * 
 * @param freq Frequency to check in Hz
 * @param tolerance Tolerance for matching (default 0.1 Hz)
 * @return true if frequency is a natural harmonic, false otherwise
 */
bool cymatic_is_natural_harmonic(double freq, double tolerance);

/**
 * @brief Calculate combined correction from multiple frequencies
 * 
 * Combines multiple cymatic frequencies into a single correction value.
 * Uses weighted sum based on amplitudes.
 * 
 * @param frequencies Array of frequency configurations
 * @param count Number of frequencies
 * @return Combined angular correction
 */
double cymatic_combined_correction(const CymaticFrequency *frequencies, size_t count);

/**
 * @brief Get frequency name as string
 * 
 * @param type Frequency type
 * @return Frequency name (static string)
 */
const char* cymatic_frequency_name(CymaticFrequencyType type);

/**
 * @brief Calculate Schumann resonance harmonics
 * 
 * The Schumann resonance has multiple harmonics at approximately:
 * 7.83, 14.3, 20.8, 27.3, 33.8 Hz
 * 
 * @param harmonic_number Harmonic number (1-5)
 * @return Schumann harmonic frequency
 */
double cymatic_schumann_harmonic(uint32_t harmonic_number);

#ifdef __cplusplus
}
#endif

#endif /* CLLM_CYMATIC_FREQUENCIES_H */