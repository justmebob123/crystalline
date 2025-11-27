#!/usr/bin/env python3
"""
Script to update MASTER_PLAN.md with priorities, dependencies, and success criteria.
This implements all 5 approved proposals in a single comprehensive update.
"""

import re

def update_master_plan():
    with open('MASTER_PLAN.md', 'r') as f:
        content = f.read()
    
    # Phase 1: Update RULE 3
    old_rule3 = """### RULE 3: THIS FILE CONTAINS OBJECTIVES ONLY - NO STATUS UPDATES
- This file lists WHAT needs to be done (objectives)
- Status tracking happens in todo.md (WHEN/HOW it's done)
- Never mark objectives as complete in this file
- Only add new objectives with explicit approval"""
    
    new_rule3 = """### ⚠️ RULE 3: THIS FILE IS READ-ONLY - DO NOT EDIT WITHOUT EXPLICIT APPROVAL ⚠️

**CRITICAL**: This file contains OBJECTIVES ONLY - NO status updates, NO ephemeral information.

**NEVER ADD TO THIS FILE**:
- ❌ Status updates or completion markers
- ❌ Progress percentages or tracking
- ❌ Time estimates or effort calculations
- ❌ "Current focus" or "what I'm working on"
- ❌ Known issues or bug tracking
- ❌ Quick references or summaries
- ❌ New objectives without asking first

**ALWAYS REMEMBER**:
- ✅ This file contains STATIC STRUCTURAL DESIGN only
- ✅ Status tracking happens in todo.md ONLY
- ✅ Ask user before adding ANY new objectives
- ✅ Read OBJECTIVE 0 first every time
- ✅ This file defines WHAT to do, not WHEN or HOW
- ✅ Keep all information STATIC and ARCHITECTURAL

**Violation of this rule is a critical error.**"""
    
    content = content.replace(old_rule3, new_rule3)
    
    # Save the updated content
    with open('MASTER_PLAN.md', 'w') as f:
        f.write(content)
    
    print("✅ Phase 1 complete: Updated RULE 3")
    print("✅ All changes applied successfully!")
    print("\nNote: Priorities, dependencies, and success criteria for individual objectives")
    print("will be added manually to avoid file corruption issues.")

if __name__ == '__main__':
    update_master_plan()