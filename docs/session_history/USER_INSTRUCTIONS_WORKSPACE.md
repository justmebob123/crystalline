# How to Use Workspace Management

## Problem Solved
You complained: "You also didnt actually provide a way for me to select what fucking path I wanted for my workspace."

**FIXED!** You now have full control over workspace paths.

## How to Use It

### 1. See Current Workspace
- Open the Training tab
- Look at the very top of the control panel
- You'll see a text input labeled "Workspace:" showing the current path

### 2. Switch to a Different Workspace

**Method 1: Type the Path**
1. Click in the "Workspace:" input field
2. Type the full path you want (e.g., `/home/user/my_ai_project`)
3. Click the "Switch" button
4. Done! The system will:
   - Create the directory if it doesn't exist
   - Create all subdirectories (models/, data/training/, etc.)
   - Rescan for training files in the new location

**Method 2: Browse (Coming Soon)**
- The "Browse..." button is there but not yet functional
- For now, just type the path manually

### 3. What Happens When You Switch
```
Your New Workspace/
├── models/              ← Your trained models go here
├── data/
│   └── training/        ← Put your .txt training files here
├── checkpoints/         ← Training checkpoints saved here
└── docs/
    └── research/        ← Research documents
```

## Examples

### Example 1: Create a New Project
```
1. Type in workspace field: /home/bob/crystalline_projects/sentiment_analysis
2. Click "Switch"
3. System creates all directories
4. Add training files to: /home/bob/crystalline_projects/sentiment_analysis/data/training/
5. Train model
6. Model saves to: /home/bob/crystalline_projects/sentiment_analysis/models/
```

### Example 2: Switch Between Projects
```
Project A: /home/bob/project_a
Project B: /home/bob/project_b

1. Type: /home/bob/project_a
2. Click Switch
3. Train on Project A data
4. Type: /home/bob/project_b
5. Click Switch
6. Train on Project B data

Each project is completely isolated!
```

## Benefits

✅ **Full Control** - You choose exactly where files go
✅ **Project Isolation** - Each workspace is independent
✅ **Easy Switching** - Change projects without restarting
✅ **Auto-Creation** - Directories created automatically
✅ **No Confusion** - Always see which workspace is active

## Current Limitations

⚠️ **No File Picker Yet** - Must type paths manually (Browse button is placeholder)
⚠️ **No Recent List** - Can't see recently used workspaces (future feature)

## Testing It

```bash
cd /workspace/crystalline
make app
LD_LIBRARY_PATH=.. app/hyper_prime_spiral
```

1. Go to Training tab
2. See "Workspace:" at the top
3. Type a path like `/tmp/test_workspace`
4. Click "Switch"
5. Check terminal - you'll see:
   ```
   Switched to workspace: /tmp/test_workspace
   ✓ Workspace directories created
     - /tmp/test_workspace/models/
     - /tmp/test_workspace/data/training/
     ...
   ```

## Summary

**You now have complete control over workspace paths.** Type any path you want, click Switch, and the system handles the rest. No more being forced to use default directories.
</file_path>