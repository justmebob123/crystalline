/**
 * EPUB Extractor
 * 
 * Pure C implementation using ZIP extraction and HTML parsing.
 * TODO: Implement full extraction logic
 */

#include "../docproc.h"
#include <stdio.h>
#include <string.h>

/**
 * Extract text from EPUB
 * TODO: Implement ZIP + HTML parsing
 */
DocProcStatus docproc_extract_epub(const char* filepath, char* text, size_t text_size) {
    // Placeholder: Will implement ZIP extraction + HTML chapter parsing
    snprintf(text, text_size, "[EPUB extraction not yet implemented: %s]", filepath);
    return DOCPROC_ERROR_UNSUPPORTED_FORMAT;
}