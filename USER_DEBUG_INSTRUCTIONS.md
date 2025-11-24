# User Debug Instructions

## What We're Testing
We've added extensive debug output to figure out why the START CRAWLER button doesn't work on the first click.

## Steps to Test

1. **Rebuild the application**:
```bash
cd ~/code/math/crystalline
git pull origin main
make clean && make && make app
```

2. **Run the application**:
```bash
app/hyper_prime_spiral
```

3. **Go to Training tab**

4. **Paste the URL**:
   - Click in the "Crawler Start URL" input field
   - Press Ctrl+V to paste: `https://x.com/JustMeBob123`
   - You should see in terminal: `Pasted from clipboard: https://x.com/JustMeBob123`

5. **Click START CRAWLER button**:
   - Click it once
   - Watch the terminal output carefully

6. **If it doesn't work, click again**:
   - Click START CRAWLER a second time
   - Watch the terminal output

## What to Look For

The terminal should show debug messages like:
```
Pasted from clipboard: https://x.com/JustMeBob123
DEBUG: After paste, input->text = '...', length = ..., active = ...
DEBUG: handle_training_tab_click called at (X, Y)
DEBUG: START CRAWLER button clicked at (X, Y)
DEBUG: Button bounds: x=..., y=..., w=..., h=...
DEBUG: crawler_url_input.text = '...', length = ..., active = ...
DEBUG: Retrieved URL from input: '...'
```

## What to Report

**Please copy and paste the ENTIRE terminal output** from when you:
1. Paste the URL (Ctrl+V)
2. Click START CRAWLER (first time)
3. Click START CRAWLER (second time, if needed)

Include ALL the debug messages - they will tell us exactly what's happening.

## What We're Looking For

The debug output will tell us:
- ✅ Is the text actually stored after paste?
- ✅ Is the click reaching the button handler?
- ✅ Are the button bounds correct?
- ✅ Is the input field still active when clicking?
- ✅ What's in the text field when the button is clicked?

Once we see the debug output, we'll know exactly what's wrong and can fix it immediately.