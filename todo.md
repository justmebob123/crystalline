# TODO: Fix PHP Installation Script Path Issue

## Current Issue
The PHP installation script (`scripts/install_php_centos.sh`) has a path issue:
- Script assumes it's run from `scripts/` directory
- Uses `cd ../math` which fails when run from root directory
- Error: `cd: ../math: No such file or directory`

## Tasks
1. [x] Identify the issue in the installation script
2. [x] Fix the path handling to work from any directory
3. [x] Update both Ubuntu and CentOS scripts
4. [ ] Verify the fix works
5. [ ] Commit and push the fix

## Solution Approach
- Use absolute paths based on script location
- Detect the script's directory using `$0` or `dirname`
- Calculate paths relative to the detected script directory