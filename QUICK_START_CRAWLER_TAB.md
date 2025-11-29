# Quick Start: Using the Crawler Tab

## Running the Application

1. **Navigate to the app directory:**
   ```bash
   cd app
   ```

2. **Run the application:**
   ```bash
   LD_LIBRARY_PATH=.. ./hyper_prime_spiral
   ```

3. **Click on the "Crawler" tab** at the top of the window (rightmost tab)

## Crawler Tab Features

### 1. Status Section (Top)
- **Status**: Shows "Running" (green) or "Stopped" (gray)
- **Pages**: Number of pages crawled
- **Queue**: Number of links in queue

### 2. Prime Configuration Panel
- **Enable/Disable Toggle**: Click to turn prime randomization on/off
- **Prime Inputs**: Enter prime numbers for:
  - **Frequency**: How often to crawl (default: 7)
  - **Selection**: Link selection pattern (default: 13)
  - **Delay Min**: Minimum delay in seconds (default: 3)
  - **Delay Max**: Maximum delay in seconds (default: 11)
- **Validation**: Green ✓ for valid primes, Red ✗ for invalid
- **Apply Button**: Appears when all inputs are valid primes

### 3. Link Queue Panel
- **Add URL**: Text field to enter URLs
- **Add Button**: Adds URL to the crawl queue
- **Clear Button**: Clears the input field
- **Confirmation**: "Link added!" message appears briefly

### 4. URL Patterns Panel
- **Checkboxes** showing enabled pattern types:
  - `[X] href` - Standard href attributes
  - `[X] onclick` - JavaScript onclick handlers
  - `[X] data-*` - Data attributes
  - `[X] meta` - Meta refresh redirects

### 5. Control Buttons (Bottom)
- **Start/Stop**: Toggle crawler execution
- **Save Config**: Save current configuration (placeholder)

## Testing the Crawler Tab

### Test Prime Validation
1. Click on the "Frequency" input field
2. Type a prime number (e.g., 7, 11, 13, 17, 19, 23)
3. You should see a green ✓ appear
4. Try a non-prime (e.g., 8, 10, 12)
5. You should see a red ✗ appear

### Test Link Management
1. Click in the "Add URL" field
2. Type a URL (e.g., https://example.com)
3. Click the "Add" button
4. You should see "Link added!" confirmation
5. The queue counter should increment

### Test Crawler Control
1. Click the "Start" button
2. The status should change to "Running" (green)
3. The button should change to "Stop"
4. Click "Stop" to stop the crawler

## Keyboard Input

When the "Add URL" field is active:
- **Type**: Enter URL characters
- **Backspace**: Delete characters
- **Enter**: (Currently not implemented, use Add button)

## Known Limitations

1. **Link List Display**: Currently shows placeholder text instead of actual queued links
2. **Pattern Checkboxes**: Visual only, not yet functional for toggling
3. **Start URL**: Hardcoded to "https://example.com" (will be configurable)
4. **Configuration Persistence**: Save/Load buttons are placeholders

## Troubleshooting

### Tab Not Visible
- Make sure you rebuilt the application after pulling the latest code
- The Crawler tab should be the rightmost tab

### Application Won't Start
```bash
# Check if libraries are built
cd ..
make

# Try running again
cd app
LD_LIBRARY_PATH=.. ./hyper_prime_spiral
```

### Prime Validation Not Working
- Make sure you're entering only numbers
- Prime numbers: 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47...
- Non-primes: 1, 4, 6, 8, 9, 10, 12, 14, 15, 16, 18, 20...

### Crawler Won't Start
- This is expected - the crawler integration is in place but may need additional configuration
- The UI is functional and ready for testing

## Next Steps

1. **Test all UI elements** to ensure they work correctly
2. **Report any bugs** or unexpected behavior
3. **Suggest improvements** for the interface

## Screenshots

(Screenshots would go here showing the Crawler tab interface)

## Additional Resources

- `CRAWLER_TAB_IMPLEMENTATION.md` - Detailed implementation documentation
- `DEPENDENCIES.md` - System requirements and installation
- `PROJECT_STATUS.md` - Overall project status

---

**Enjoy exploring the new Crawler tab!** 🚀