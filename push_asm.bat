git checkout %APPVEYOR_REPO_BRANCH% 
git add "asm/*"
git status
git diff-index --cached --quiet --exit-code HEAD || (git commit -m "[skip ci] Update ASM for %ASM_FOLDER%" && git push)
