function Get-AppVeyorBuild {
    param()

    if (-not ($env:APPVEYOR_API_TOKEN)) {
        throw "missing api token for AppVeyor."
    }

    Invoke-RestMethod -Uri "https://ci.appveyor.com/api/projects/$env:APPVEYOR_ACCOUNT_NAME/$env:APPVEYOR_PROJECT_SLUG" -Method GET -Headers @{
        "Authorization" = "Bearer $env:APPVEYOR_API_TOKEN"
        "Content-type"  = "application/json"
    }
}

function Get-TravisBuild {
    param(
        [int]$limit    = 10,
        [int]$maxLimit = 100
    )
    if (-not ($env:TRAVIS_API_TOKEN)) {
        throw "missing api token for Travis-CI."
    }
    
    $builds = Invoke-RestMethod -Uri "https://api.travis-ci.org/builds?limit=$($limit)" -Method Get -Headers @{
        "Authorization" = "token $env:TRAVIS_API_TOKEN"
        "Travis-API-Version" = "3" 
    } 
    $currentBuild = $builds.builds | Where-Object {$_.commit.sha -eq $env:APPVEYOR_REPO_COMMIT}
    if (!$currentBuild) {
        if($limit+10 -le $maxLimit) {
            return Get-TravisBuild -limit $limit+10 -maxLimit $maxLimit
        } else {
            throw "Could not get information about Travis build with sha $REPO_COMMIT"
        }
    } 
    return $currentBuild
}

function appveyorFinished {
    param()
    $buildData = Get-AppVeyorBuild
    $lastJob = ($buildData.build.jobs | Select-Object -Last 1).jobId

    if ($lastJob -ne $env:APPVEYOR_JOB_ID) {
        return $false
    }

    Write-Host "[Appveyor] At Last Job: "
    Write-Host $buildData
    Write-Host $buildData.build
    Write-Host $buildData.build.jobs

    [datetime]$stop = ([datetime]::Now).AddMinutes($env:TIMEOUT_MINS)

    do {
        Write-Host "[Appveyor] Checking Build Jobs"

        $allSuccess = $true
        (Get-AppVeyorBuild).build.jobs | Where-Object {$_.jobId -ne $env:APPVEYOR_JOB_ID} | Foreach-Object `
            { 
                $job = $_
                Write-Host "[Appveyor] Checking Build Job $($job.jobId)"
                Write-Host $job
                Write-Host " "

                switch ($job.status) {
                    "failed" { throw "AppVeyor's Job ($($job.jobId)) failed." }
                    "success" { continue }
                    Default { Write-Host "Job status: $($job.status)"; Write-Host $job ;$allSuccess = $false }
                }
            }
        if ($allSuccess) { return $true }
        Start-sleep 5
    } while (([datetime]::Now) -lt $stop)

    throw "Test jobs were not finished in $env:TIMEOUT_MINS minutes"
}

function travisFinished {
    param()

    [datetime]$stop = ([datetime]::Now).AddMinutes($env:TIMEOUT_MINS)

    do {
        $builds = Get-TravisBuild
        $currentBuild = $builds.builds | Where-Object {$_.commit.sha -eq $env:APPVEYOR_REPO_COMMIT}
        if (!$currentBuild) {
            throw "Could not get information about Travis build with sha $REPO_COMMIT"
        }    
        switch -regex ($currentBuild.state) {
            "^passed$" {
                return $true
            }
            "^(errored|failed|canceled)" {
                throw "Travis Job ($($builds.builds.id)) failed"
            }
        }

        Start-sleep 5
    } while (([datetime]::Now) -lt $stop)

    throw "Travis build did not finished in $env:TIMEOUT_MINS minutes"
}

function collectAsm {
    # Create branch to merge asm into
    # $asmFinalBranch = asm/$env:APPVEYOR_REPO_COMMIT/final
    # git pull
    # git checkout $env:APPVEYOR_REPO_BRANCH
    # git checkout -b $asmFinalBranch

    #collect all branches to merge
    $asmBranches = @()
    (Get-AppVeyorBuild).build.jobs | Foreach-Object { $asmBranches += "origin/asm/$($env:APPVEYOR_REPO_COMMIT)/appveyor-$($_.jobId)"}
    #TODO: collect asm from travis
    # $travisBuild = Get-TravisBuild
    # $travisJobs = @()
    # $travisBuild.jobs | Foreach-Object { $travisJobs += $_.id }

    #Merge all branches into master
    $branchString = $asmBranches -join ' '
    git checkout master
    git pull
    git merge --squash $branchString
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