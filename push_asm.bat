git config --global credential.helper store
powershell -command 'Add-Content "$HOME\.git-credentials" "https://$($env:access_token):x-oauth-basic@github.com`n"' > nul 2>&1
git config --global user.email "dadonenf@microsoft.com"
git config --global user.name "Daniel Donenfeld"
git config --global core.autocrlf false 

REM Get branch to check asm into 
git checkout %APPVEYOR_REPO_BRANCH% 
git checkout -b asm/%APPVEYOR_REPO_COMMIT%/appveyor-%APPVEYOR_JOB_NUMBER%

REM Check asm into the branch
git add "asm/*"
git status
git diff-index --cached --quiet --exit-code HEAD || git commit -m "[skip ci] Update ASM for %ASM_FOLDER%"
git push -u origin HEAD