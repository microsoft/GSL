# Include the API access functions
. $PSScriptRoot\API-Functions.ps1

function Run-Command-With-Retry {
    param(
        [string]$cmd,
        [string]$cleanup_cmd = "",
        [int]$maxTries = 3
    )

    while ($maxTries -gt 0) {
        cmd.exe /c $cmd
        if($?){
            return
        } else {
            cmd.exe /c $cleanup_cmd
            $maxTries = $maxTries - 1
        }
    }

    throw "Command $($command) failed with exit code $($LastExitCode)"
}

function collectAsm {
    cmd.exe /c "git checkout $($env:APPVEYOR_REPO_BRANCH) 2>&1"
    cmd.exe /c "git fetch --all 2>&1" 

    # Create branch to merge asm into
    $asmFinalBranch = "asm/$($env:APPVEYOR_REPO_COMMIT)/final"
    cmd.exe /c "git checkout -b $($asmFinalBranch) $($env:APPVEYOR_REPO_COMMIT) 2>&1"

    #merge all branches into final branch
    (Get-AppVeyorBuild).build.jobs | Foreach-Object { 
        $branchName = "asm/$($env:APPVEYOR_REPO_COMMIT)/appveyor-$($_.jobId)"

        #Check that all branches exist
        cmd.exe /c "git ls-remote --heads --exit-code https://github.com/dadonenf/GSL.git $($branchName)"
        if(-not $?){
            throw "Missing branch for job $($_.jobId)"
        }

        #Only merge in asm if there is any change between the current branch and the repo branch
        cmd.exe /c "git diff-tree --quiet origin/$($branchName)..$($env:APPVEYOR_REPO_COMMIT)"
        if(-not $?){
            # Use cherry-pick as the asm branches only have a single commit
            cmd.exe /c "git cherry-pick origin/$($branchName) 2>&1"
            if(-not $?){
                throw "Failed merge of $($branchName)"
            }
        }
    }

    #Merge all branches into $($env:APPVEYOR_REPO_BRANCH)
    cmd.exe /c "git checkout $($env:APPVEYOR_REPO_BRANCH) 2>&1"
    cmd.exe /c "git merge --squash $($asmFinalBranch) 2>&1"
    if(-not $?){
        throw "Failed merge"
    }
    cmd.exe /c "git diff-index --cached --quiet --exit-code HEAD"
    if(-not $?) {
        git commit -m "[skip ci] Update ASM for $($env:APPVEYOR_REPO_COMMIT)"
        Run-Command-With-Retry -cmd "git push 2>&1" -cleanup_cmd "git pull --rebase 2>&1"
    }
}

Write-Host "Starting asm deployment"

# Wait for all jobs
if(appveyorFinished) {

    Write-Host "Appveyor Finished, collecting ASM"

    # Collect ASM (currently from Appveyor only)
    collectAsm
}