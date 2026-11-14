<#
.SYNOPSIS
    Adds a mail account to the lab's green.mpc or blue.mpc mail server, from your own computer.

.DESCRIPTION
    Finds the mail server's EC2 instance by its Name tag, and runs add-mail-user.sh on it through
    AWS Systems Manager Run Command (the instances already have the SSM role), so no SSH is needed.
    It then prints the Outlook settings for the new account.

    Needs the AWS CLI, signed in to the lab's account, in the lab's region.

    The password is passed to the server as part of the Run Command, so it also appears in that
    command's history in the Systems Manager console. That's fine for a test lab; don't reuse a
    real password here.

.PARAMETER Domain
    green.mpc (mail.green.mpc, alice's server) or blue.mpc (mail.blue.mpc, bob's server).

.PARAMETER UserName
    The new account's name: lowercase letters, digits, _ and -. The address is UserName@Domain.

.PARAMETER Password
    The account's password (8+ characters). Leave it out to generate one.

.PARAMETER ResetPassword
    The account exists already: give it a new password instead.

.PARAMETER Region
    The lab's AWS region (the "region" in terraform.tfvars). Defaults to the AWS CLI's region.

.PARAMETER NamePrefix
    The lab's name_prefix in terraform.tfvars (default mpc-lab).

.PARAMETER AwsProfile
    An AWS CLI profile to use, if not the default.

.EXAMPLE
    .\New-LabMailUser.ps1 -Domain blue.mpc -UserName carol -Region ap-southeast-2

.EXAMPLE
    .\New-LabMailUser.ps1 -Domain green.mpc -UserName dave -Password 'Lab-Pass-2026' -Region ap-southeast-2

.EXAMPLE
    .\New-LabMailUser.ps1 -Domain blue.mpc -UserName carol -ResetPassword -Region ap-southeast-2
#>
param(
    [Parameter(Mandatory = $true)][ValidateSet('green.mpc', 'blue.mpc')][string]$Domain,
    [Parameter(Mandatory = $true)][ValidatePattern('(?-i)^[a-z_][a-z0-9_-]{0,31}$')][string]$UserName,
    [string]$Password,
    [switch]$ResetPassword,
    [string]$Region,
    [string]$NamePrefix = 'mpc-lab',
    [string]$AwsProfile
)

$ErrorActionPreference = 'Stop'

# The lab host (lab/aws-opentofu/main.tf) that is each domain's mail server.
$mailHosts = @{ 'green.mpc' = 'mta-sender'; 'blue.mpc' = 'mx-recipient' }
$serverScript = Join-Path $PSScriptRoot 'add-mail-user.sh'

if (-not (Get-Command aws -ErrorAction SilentlyContinue)) {
    throw 'The AWS CLI was not found. Install it (winget install Amazon.AWSCLI) and sign in, then try again.'
}
if (-not (Test-Path $serverScript)) { throw "add-mail-user.sh must be next to this script ($serverScript)." }

$common = @()
if ($Region) { $common += @('--region', $Region) }
if ($AwsProfile) { $common += @('--profile', $AwsProfile) }

function Invoke-Aws {
    $output = & aws @args @common
    if ($LASTEXITCODE -ne 0) { throw "aws $($args[0]) $($args[1]) failed (exit code $LASTEXITCODE)." }
    $output
}

# Same rule as add-mail-user.sh.
if ($Password) {
    if ($Password.Length -lt 8) { throw 'The password must be at least 8 characters.' }
    if ($Password -match "[`r`n]") { throw 'The password must not contain a line break.' }
} else {
    $chars = [char[]]'ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789'
    $bytes = New-Object byte[] 16
    $rng = [Security.Cryptography.RandomNumberGenerator]::Create()
    $rng.GetBytes($bytes)
    $rng.Dispose()
    $Password = -join ($bytes | ForEach-Object { $chars[$_ % $chars.Length] })
}

# --- Find the mail server -------------------------------------------------------------------
$tag = "$NamePrefix-$($mailHosts[$Domain])"
$found = Invoke-Aws ec2 describe-instances `
    --filters "Name=tag:Name,Values=$tag" 'Name=instance-state-name,Values=running' `
    --query 'Reservations[].Instances[].[InstanceId,PublicIpAddress]' --output text
$found = @($found | Where-Object { $_ -and $_.Trim() })
if ($found.Count -eq 0) {
    throw "No running instance named $tag. Is the lab up (tofu apply), and is the region right? Pass -Region, e.g. -Region ap-southeast-2."
}
if ($found.Count -gt 1) { throw "More than one running instance is named $tag." }
$instanceId, $publicIp = ($found[0] -split '\s+')
Write-Host "Mail server for ${Domain}: $tag ($instanceId, $publicIp)"

# --- Run add-mail-user.sh on it --------------------------------------------------------------
function ConvertTo-ShellWord([string]$Value) { "'" + ($Value -replace "'", "'\''") + "'" }

$mode = if ($ResetPassword) { '--reset' } else { '' }
$commands = @("set -- $(ConvertTo-ShellWord $UserName) $(ConvertTo-ShellWord $Password) $(ConvertTo-ShellWord $mode)")
$commands += Get-Content -Path $serverScript

# Through a file: the commands contain quotes that don't survive Windows command-line parsing.
$paramsFile = Join-Path ([IO.Path]::GetTempPath()) ("mpc-lab-user-" + [Guid]::NewGuid().ToString('N') + '.json')
try {
    $json = @{ commands = $commands } | ConvertTo-Json -Depth 3
    [IO.File]::WriteAllText($paramsFile, $json, (New-Object Text.UTF8Encoding($false)))
    $commandId = Invoke-Aws ssm send-command --instance-ids $instanceId `
        --document-name AWS-RunShellScript --comment "Add lab mail user $UserName@$Domain" `
        --parameters "file://$paramsFile" --query 'Command.CommandId' --output text
} finally {
    Remove-Item -Path $paramsFile -Force -ErrorAction SilentlyContinue
}
$commandId = "$commandId".Trim()
Write-Host "Running add-mail-user.sh through Systems Manager (command $commandId)..."

$deadline = (Get-Date).AddMinutes(3)
do {
    Start-Sleep -Seconds 2
    # Right after send-command the invocation may not exist yet; that's an error we wait out.
    $raw = & aws ssm get-command-invocation --command-id $commandId --instance-id $instanceId --output json @common 2>$null
    $result = if ($LASTEXITCODE -eq 0) { ($raw -join "`n") | ConvertFrom-Json } else { $null }
    $status = if ($result) { $result.Status } else { 'Pending' }
} while ($status -in @('Pending', 'InProgress', 'Delayed') -and (Get-Date) -lt $deadline)

if ($result -and $result.StandardOutputContent) { Write-Host $result.StandardOutputContent.TrimEnd() }
if ($result -and $result.StandardErrorContent) { Write-Warning $result.StandardErrorContent.TrimEnd() }
if ($status -ne 'Success') {
    throw "Adding the user did not succeed (status $status). If the SSM agent is not online yet, wait a minute after tofu apply and try again."
}

# --- Outlook settings --------------------------------------------------------------------------
Write-Host ''
Write-Host 'Add it to Outlook as an IMAP account with manual setup:'
Write-Host "  Email address   : $UserName@$Domain"
Write-Host "  User name       : $UserName"
Write-Host "  Password        : $Password"
Write-Host "  Incoming (IMAP) : $publicIp, port 993, SSL/TLS"
Write-Host "  Outgoing (SMTP) : $publicIp, port 587, STARTTLS, same user name and password"
