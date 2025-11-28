/**
 * Email Extractor (EML)
 * 
 * Pure C implementation of email parsing.
 * TODO: Implement full email parsing
 */

#include "../docproc.h"
#include <stdio.h>
#include <string.h>

/**
 * Extract text from EML
 * TODO: Implement email header and body parsing
 */
DocProcStatus docproc_extract_eml(const char* filepath, char* text, size_t text_size) {
    // Placeholder: Will implement email parsing
    snprintf(text, text_size, "[EML extraction not yet implemented: %s]", filepath);
    return DOCPROC_ERROR_UNSUPPORTED_FORMAT;
}