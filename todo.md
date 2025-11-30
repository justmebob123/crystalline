# TODO - Crystalline CLLM Project

## RULES (ALWAYS FOLLOW)

### RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
Before taking ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
The AUDIT.md contains:
- Current architectural violations
- Required fixes with priorities
- Implementation phases
- Testing requirements
- Success criteria

### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
The SECONDARY_OBJECTIVES.md contains:
- Detailed implementation tasks
- Code examples
- File-by-file changes
- Testing procedures
- Validation steps

### RULE 4: DO NOT CREATE NEW MD FILES
All documentation goes in existing files or this master plan only.

### RULE 5: ALWAYS COMMIT ALL CHANGES USING CORRECT AUTHENTICATION
```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
```

### RULE 6: THIS FILE IS READ-ONLY - DO NOT EDIT WITHOUT EXPLICIT APPROVAL
**CRITICAL**: MASTER_PLAN.md contains OBJECTIVES ONLY - NO status updates, NO ephemeral information.

---

## COMPREHENSIVE BIDIRECTIONAL ANALYSIS

### Analysis of User's Debug Log

#### What Worked:
1. **Training Tab Inputs (tab_id=6)** - ALL 5 inputs worked perfectly:
   - learning_rate: Click at (1175, 429) → Focused ✓
   - epochs: Click at (1171, 489) → Focused ✓
   - batch_size: Click at (1186, 550) → Focused ✓
   - thread_count: Click at (1189, 603) → Focused ✓
   - crawler_url: Click at (1148, 706) → Focused ✓

2. **Submenu Navigation** - Worked (tabs changed correctly)

3. **Sidebar Navigation** - Worked (main tabs changed)

#### What Did NOT Work:
1. **Research Tab (tab=7)** - NO inputs registered
   - Clicks at (1218, 165), (1131, 120), (1124, 212), (1271, 214), (1333, 206)
   - All skipped because training inputs have tab_id=6

2. **LLM Tab (tab=5)** - NO inputs registered
   - Clicks at (647, 845), (1239, 858), (1402, 172), (855, 300)
   - All skipped because training inputs have tab_id=6
   - Model creation failed: "ERROR: Failed to acquire model"

3. **Models Tab (tab=9)** - NO inputs registered
   - Click at (114, 191)
   - All skipped because training inputs have tab_id=6

#### Critical Discovery:
**ONLY the Training tab has inputs registered!**
- Training tab: 5 inputs registered ✓
- LLM tab: 0 inputs registered ✗
- Research tab: 0 inputs registered ✗
- Crawler tab: 0 inputs registered ✗
- Models tab: 0 inputs registered ✗

### Root Cause Analysis

#### Problem 1: Inputs Only Registered Once
Inputs are registered when a tab is FIRST drawn, but:
- Training tab was drawn first → inputs registered
- Other tabs never drawn → inputs never registered
- When you switch tabs, old inputs remain but with wrong tab_id

#### Problem 2: No Input Cleanup on Tab Switch
When switching tabs:
- Old inputs stay in input_manager
- New tab's inputs are never registered
- Result: Wrong tab_id, all inputs skipped

#### Problem 3: Lazy Registration Pattern
Each tab registers inputs on first draw:
```c
if (!inputs_registered) {
    register_inputs();
    inputs_registered = true;
}
```
But if tab is never drawn, inputs are never registered!

### Solution Required

#### Option A: Register All Inputs at Startup
- Register inputs for ALL tabs during initialization
- Each tab gets its inputs registered once
- Pros: Simple, all inputs always available
- Cons: Memory overhead, inputs for unused tabs

#### Option B: Register/Unregister on Tab Switch
- Register inputs when entering tab
- Unregister inputs when leaving tab
- Pros: Clean, only active tab has inputs
- Cons: Complex, need cleanup logic

#### Option C: Hybrid Approach (RECOMMENDED)
- Register inputs on first tab visit
- Keep inputs registered permanently
- Update visibility based on current tab
- Pros: Best of both worlds
- Cons: Slight memory overhead

---

## IMPLEMENTATION PLAN

### Phase 1: Ensure All Tabs Register Inputs
- [ ] Force draw of each tab during initialization
- [ ] OR: Call register functions explicitly at startup
- [ ] Verify all tabs have inputs registered

### Phase 2: Fix LLM Tab Input System
- [ ] Investigate why LLM tab has no inputs
- [ ] Check if LLM uses old text_input system
- [ ] Migrate LLM to InputManager if needed

### Phase 3: Fix Model Creation Error
- [ ] Debug "ERROR: Failed to acquire model"
- [ ] Check model_manager_create() function
- [ ] Verify model creation parameters

### Phase 4: Add Inputs to Missing Tabs
- [ ] Research tab needs inputs
- [ ] Crawler tab needs inputs registered
- [ ] URL Manager tab needs inputs

---

## STATUS: PERFORMING COMPREHENSIVE ANALYSIS