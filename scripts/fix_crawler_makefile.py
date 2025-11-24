#!/usr/bin/env python3
"""Fix crawler library linking"""

with open('Makefile', 'r') as f:
    content = f.read()

# Fix the crawler library target
old = '$(CRAWLER_LIB): $(CRAWLER_OBJECTS)\n\t@echo "Creating crawler library: $@"\n\t$(CC) -shared -o $@ $^ -lcurl -lpthread -lm'
new = '$(CRAWLER_LIB): $(CRAWLER_OBJECTS) $(STATIC_LIB)\n\t@echo "Creating crawler library: $@"\n\t$(CC) -shared -o $@ $(CRAWLER_OBJECTS) -L. -lprimemath -lcurl -lpthread -lm'

content = content.replace(old, new)

with open('Makefile', 'w') as f:
    f.write(content)

print("✓ Makefile fixed")