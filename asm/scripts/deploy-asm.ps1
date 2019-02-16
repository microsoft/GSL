# Include the API access functions
. $PSScriptRoot\API-Functions.ps1

function collectAsm {
    # Create branch to merge asm into
    # $asmFinalBranch = asm/$env:APPVEYOR_REPO_COMMIT/final
    # git pull
    # git checkout $env:APPVEYOR_REPO_BRANCH
    # git checkout -b $asmFinalBranch

    #collect all branches to merge
    $asmBranches = @()
    (Get-AppVeyorBuild).build.jobs | Foreach-Object { 
        $branchName = "asm/$($env:APPVEYOR_REPO_COMMIT)/appveyor-$($_.jobId)"

        #Check that all branches exist
        git ls-remote --heads --exit-code https://github.com/dadonenf/GSL.git $branchName
        if(-not $?){
            throw "Missing branch for job $($_.jobId)"
        }

        git checkout $branchName 2>&1

        #Add branch to the branch list
        # $branchName = "origin/"+$branchName
        $asmBranches += $branchName
    }

    #TODO: collect asm from travis
    # $travisBuild = Get-TravisBuild
    # $travisJobs = @()
    # $travisBuild.jobs | Foreach-Object { $travisJobs += $_.id }

    #Merge all branches into master
    $branchString = $asmBranches -join ' '
    git checkout master 2>&1
    git pull
    git fetch --all
    git branch -a
    Write-Host "git merge --squash $($branchString)"
    git merge --squash $branchString
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