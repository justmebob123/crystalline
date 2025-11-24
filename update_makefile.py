#!/usr/bin/env python3
"""Update Makefile to build libcrawler.so and new CLI tool"""

with open('Makefile', 'r') as f:
    content = f.read()

# Find the crawler section
old_section = """# Crawler Tool
# ============================================================================

CRAWLER_SOURCES = src/crawler/crawler_core.c src/crawler/preprocessor.c \\
                  src/crawler/tokenizer.c src/crawler/continuous_training.c
CRAWLER_OBJECTS = $(CRAWLER_SOURCES:.c=.o)

crawler: $(STATIC_LIB) $(CRAWLER_OBJECTS)
\t@echo "Building crawler tool..."
\t@mkdir -p tools
\t$(CC) $(CFLAGS) -o tools/cllm_crawler tools/cllm_crawler.c $(CRAWLER_OBJECTS) \\
\t\t-L. -lprimemath -lcurl -lpthread -lm
\t@echo "✓ Crawler built: tools/cllm_crawler"
"""

new_section = """# ============================================================================
# Crawler Library (libcrawler.so)
# ============================================================================

CRAWLER_SOURCES = src/crawler/crawler_core.c src/crawler/preprocessor.c \\
                  src/crawler/tokenizer.c src/crawler/continuous_training.c \\
                  src/crawler/crawler_api.c
CRAWLER_OBJECTS = $(CRAWLER_SOURCES:.c=.o)
CRAWLER_LIB = libcrawler.so

$(CRAWLER_LIB): $(CRAWLER_OBJECTS)
\t@echo "Creating crawler library: $@"
\t$(CC) -shared -o $@ $^ -lcurl -lpthread -lm
\t@echo "✓ Crawler library created"

# ============================================================================
# Crawler CLI Tool (uses libcrawler.so)
# ============================================================================

crawler: $(CRAWLER_LIB)
\t@echo "Building crawler CLI tool..."
\t@mkdir -p tools
\t$(CC) $(CFLAGS) -o tools/cllm_crawler tools/cllm_crawler_new.c \\
\t\t-L. -lcrawler -lpthread -Wl,-rpath,'$$ORIGIN/..'
\t@echo "✓ Crawler CLI built: tools/cllm_crawler"
"""

content = content.replace(old_section, new_section)

with open('Makefile', 'w') as f:
    f.write(content)

print("✓ Makefile updated successfully")