# TODO - Crystalline CLLM UI Redesign

## 🔒 RULES (PASTED FROM MASTER_PLAN.MD - RULE 0)

### ⭐ RULE 0: ALWAYS PASTE RULES TO TOP OF TODO.MD WITH EVERY RESPONSE ⭐
**HIGHEST PRIORITY RULE - MUST BE FOLLOWED WITH EVERY SINGLE RESPONSE**

At the beginning of EVERY response, you MUST:
1. Paste these rules to the top of todo.md
2. Read the MASTER_PLAN.md completely
3. Read the AUDIT.md for current architectural state
4. Read the SECONDARY_OBJECTIVES.md for detailed tasks

This creates a permanent loop ensuring rules are never forgotten.

### RULE 1: ALWAYS REREAD MASTER_PLAN.MD BEFORE ANY ACTION
**SECOND HIGHEST PRIORITY RULE**

Before taking ANY action, you MUST:
1. Read MASTER_PLAN.md completely
2. Understand the current objectives
3. Verify your action aligns with the master plan
4. Check for any blocking priorities

This ensures all work follows the architectural design.

### RULE 2: REFERENCE AUDIT.MD FOR ARCHITECTURAL STATE
**CRITICAL REFERENCE DOCUMENT**

The AUDIT.md contains:
- Current architectural violations
- Required fixes with priorities
- Implementation phases
- Testing requirements
- Success criteria

Always consult AUDIT.md before starting work to understand:
- What is broken
- What needs fixing
- What is blocking other work
- What the correct architecture should be

### RULE 3: REFERENCE SECONDARY_OBJECTIVES.MD FOR DETAILED TASKS
**DETAILED IMPLEMENTATION GUIDE**

The SECONDARY_OBJECTIVES.md contains:
- Detailed implementation tasks
- Code examples
- File-by-file changes
- Testing procedures
- Validation steps

Use this for step-by-step implementation guidance.

### RULE 4: DO NOT CREATE NEW MD FILES
All documentation goes in existing files or this master plan only.

### RULE 5: ALWAYS COMMIT ALL CHANGES USING CORRECT AUTHENTICATION
```bash
git add .
git commit -m "descriptive message"
git push https://x-access-token:$GITHUB_TOKEN@github.com/justmebob123/crystalline.git main
```

### RULE 7: FIX ALL BUILD WARNINGS BEFORE PROCEEDING
All code must compile with zero warnings before moving to the next objective.

---

## 🎯 CURRENT OBJECTIVE: Complete UI Redesign

### Critical Issues Identified:
1. **Models Tab**: Black screen - completely broken
2. **Training Tab**: Controls bunched up and overlapping
3. **All Tabs**: Text and UI elements overlapping
4. **Layout System**: Not working properly - needs complete rethink

### Phase 1: Analyze Current UI State [IN PROGRESS]

#### 1.1: Examine Models Tab (Black Screen Issue)
- [ ] Check tab_models.c rendering function
- [ ] Verify draw_models_tab() is being called
- [ ] Check for SDL rendering errors
- [ ] Verify button initialization
- [ ] Check for coordinate/bounds issues
- [ ] Identify why screen is black

#### 1.2: Examine Training Tab (Overlapping Controls)
- [ ] Analyze tab_training.c layout
- [ ] Document all UI element positions
- [ ] Identify overlapping elements
- [ ] Check hardcoded positions vs dynamic layout
- [ ] Verify proper spacing and margins

#### 1.3: Examine All Tabs for Text Overlap
- [ ] Check text rendering in each tab
- [ ] Verify font sizes and line heights
- [ ] Check text bounds calculations
- [ ] Identify clipping issues

### Phase 2: Design New Layout System

#### 2.1: Layout Principles
- [ ] Define consistent spacing (padding, margins)
- [ ] Define grid system for alignment
- [ ] Define responsive behavior
- [ ] Define z-ordering for overlays

#### 2.2: Component Sizing
- [ ] Define button sizes (small, medium, large)
- [ ] Define input field sizes
- [ ] Define label sizes
- [ ] Define panel sizes

#### 2.3: Tab-Specific Layouts
- [ ] Design Models Tab layout
- [ ] Design Training Tab layout
- [ ] Design LLM Tab layout
- [ ] Design Research Tab layout
- [ ] Design Benchmark Tab layout

### Phase 3: Implement New Layout System

#### 3.1: Fix Models Tab
- [ ] Implement proper rendering
- [ ] Fix black screen issue
- [ ] Position all elements correctly
- [ ] Test visibility and interaction

#### 3.2: Fix Training Tab
- [ ] Reorganize controls
- [ ] Fix overlapping elements
- [ ] Implement proper spacing
- [ ] Test all controls

#### 3.3: Fix All Tabs
- [ ] Apply consistent layout
- [ ] Fix text rendering
- [ ] Fix overlapping issues
- [ ] Test each tab thoroughly

### Phase 4: Testing and Validation
- [ ] Test Models Tab functionality
- [ ] Test Training Tab functionality
- [ ] Test all tabs for visual issues
- [ ] Test window resizing
- [ ] Test all interactions

---

## 📋 EXECUTION LOG

### Current Session:
- [x] Pasted rules to top of todo.md (RULE 0)
- [ ] Analyzing Models Tab black screen issue
- [ ] Analyzing Training Tab overlapping controls
- [ ] Designing new layout system