# Include the API access functions
. $PSScriptRoot\API-Functions.ps1

function collectAsm {
    # Create branch to merge asm into
    git fetch --all 2>&1 

    $asmFinalBranch = "asm/$($env:APPVEYOR_REPO_COMMIT)/final"
    git checkout -b $asmFinalBranch $env:APPVEYOR_REPO_BRANCH

    #merge all branches into final branch
    (Get-AppVeyorBuild).build.jobs | Foreach-Object { 
        $branchName = "asm/$($env:APPVEYOR_REPO_COMMIT)/appveyor-$($_.jobId)"

        #Check that all branches exist
        git ls-remote --heads --exit-code https://github.com/dadonenf/GSL.git $branchName
        if(-not $?){
            throw "Missing branch for job $($_.jobId)"
        }

        git merge $branchName 2>&1
    }
    
    #Merge all branches into master
    git checkout master 2>&1
    cmd.exe /c "git merge --squash $($asmFinalBranch)"
    if(-not $?){
        throw "Failed merge"
    }
    git diff-index --cached --quiet --exit-code HEAD
    if(-not $?) {
        git commit -m "[skip ci] Update ASM for $($env:APPVEYOR_REPO_COMMIT)"
        git push
    }
}

Write-Host "Starting asm deployment"

# Wait for all jobs
if(appveyorFinished) {

    Write-Host "Appveyor Finished, collecting ASM"

    # Collect ASM (currently from Appveyor only)
    collectAsm
}