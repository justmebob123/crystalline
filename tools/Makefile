# Tools Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I../include
LDFLAGS = -L.. -lprimemath -lm

TOOLS = web_scraper train_model

all: $(TOOLS)

web_scraper: web_scraper.c
	@echo "Building web_scraper..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ web_scraper built"

train_model: train_model.c
	@echo "Building train_model..."
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "✓ train_model built"

clean:
	@echo "Cleaning tools..."
	rm -f $(TOOLS)
	@echo "✓ Tools cleaned"

.PHONY: all clean