<#
.SYNOPSIS
    Adds or removes the MPC add-in's entry in the current user's VSTO inclusion list.

.DESCRIPTION
    The add-in's manifests are signed with a certificate Windows doesn't know, so on first start
    the VSTO runtime would ask whether to install (trust) it. An inclusion list entry - the
    deployment manifest's location plus the public key that signed it - records that trust
    decision in advance. The installer runs this script for "just me" installs; installs to
    Program Files are trusted without one.

    It uses the VSTO runtime's own API (Microsoft.VisualStudio.Tools.Office.Runtime.Security),
    as Microsoft documents in "How to: Add or Remove Inclusion List Entries".

.PARAMETER Action
    Add, Remove, or Show (print the public key read from the manifest).

.PARAMETER Manifest
    Full path of the installed MpcOutlookAddIn.vsto.
#>
param(
    [Parameter(Mandatory = $true)][ValidateSet('Add', 'Remove', 'Show')][string]$Action,
    [Parameter(Mandatory = $true)][string]$Manifest
)

$ErrorActionPreference = 'Stop'

$logDir = Join-Path ([Environment]::GetFolderPath('LocalApplicationData')) 'MpcOutlookAddIn'
$logFile = Join-Path $logDir 'install.log'

function Write-Log([string]$Message) {
    $line = '{0:yyyy-MM-dd HH:mm:ss} vsto-trust {1}: {2}' -f (Get-Date), $Action, $Message
    Write-Output $line
    try {
        if (-not (Test-Path $logDir)) { New-Item -ItemType Directory -Path $logDir -Force | Out-Null }
        Add-Content -Path $logFile -Value $line -Encoding UTF8
    } catch {
        # Logging is best effort.
    }
}

# The RSAKeyValue under the deployment manifest's StrongNameSignature, as
# <RSAKeyValue><Modulus>...</Modulus><Exponent>...</Exponent></RSAKeyValue> without namespaces.
function Get-ManifestPublicKey([string]$Path) {
    $xml = New-Object System.Xml.XmlDocument
    $xml.PreserveWhitespace = $false
    $xml.Load($Path)
    $node = $xml.SelectSingleNode(
        "//*[local-name()='Signature' and @Id='StrongNameSignature']/*[local-name()='KeyInfo']" +
        "/*[local-name()='KeyValue']/*[local-name()='RSAKeyValue']")
    if ($null -eq $node) { throw "No StrongNameSignature RSAKeyValue in $Path; is the manifest signed?" }

    $modulus = $node.SelectSingleNode("*[local-name()='Modulus']")
    $exponent = $node.SelectSingleNode("*[local-name()='Exponent']")
    if ($null -eq $modulus -or $null -eq $exponent) { throw "Incomplete RSAKeyValue in $Path." }

    '<RSAKeyValue><Modulus>{0}</Modulus><Exponent>{1}</Exponent></RSAKeyValue>' -f
        $modulus.InnerText.Trim(), $exponent.InnerText.Trim()
}

function Get-InclusionListType {
    # Installed into the GAC by the Visual Studio 2010 Tools for Office Runtime.
    $assembly = [Reflection.Assembly]::Load(
        'Microsoft.VisualStudio.Tools.Office.Runtime, Version=10.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a')
    $assembly.GetType('Microsoft.VisualStudio.Tools.Office.Runtime.Security.UserInclusionList', $true)
}

try {
    $Manifest = [IO.Path]::GetFullPath($Manifest)
    $location = New-Object System.Uri($Manifest)

    switch ($Action) {
        'Show' {
            Get-ManifestPublicKey $Manifest
            exit 0
        }
        'Add' {
            $publicKey = Get-ManifestPublicKey $Manifest
            $list = Get-InclusionListType
            $entryType = $list.Assembly.GetType('Microsoft.VisualStudio.Tools.Office.Runtime.Security.AddInSecurityEntry', $true)
            # Replace any older entry for this location: a rebuilt add-in may be signed with a new key.
            try { $list::Remove($location) } catch { }
            $list::Add($entryType::new($location, $publicKey))
            Write-Log "trusted $location"
        }
        'Remove' {
            $list = Get-InclusionListType
            $list::Remove($location)
            Write-Log "removed $location"
        }
    }
    exit 0
} catch {
    Write-Log ("failed for {0}: {1}" -f $Manifest, $_.Exception.Message)
    exit 1
}
