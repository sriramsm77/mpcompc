; Inno Setup script for the MPC Outlook add-in: produces one setup .exe that copies the add-in to
; the target machine and registers it with classic Outlook.
;
; Build it with build-installer.ps1, or directly:
;   ISCC.exe MpcOutlookAddIn.iss
;   ISCC.exe /DSourceDir=C:\path\to\bin\Release /DAppVersion=1.2.0 MpcOutlookAddIn.iss
;
; Install modes (the wizard asks; /CURRENTUSER or /ALLUSERS choose on the command line):
;   Just me (default, no admin rights): %LOCALAPPDATA%\Programs\MpcOutlookAddIn, registered under
;     HKCU. vsto-trust.ps1 adds the add-in to the VSTO inclusion list so Outlook doesn't ask
;     whether to trust it.
;   All users (admin): Program Files\MpcOutlookAddIn, registered under HKLM for both 32-bit and
;     64-bit Office. Office solutions installed to Program Files are trusted without a prompt.

#define AddInName      "MpcOutlookAddIn"
#define AppName        "MPC Outlook Add-in"
#define AppPublisher   "Sriram Murthy"
#define AppUrl         "https://github.com/sriramsm77/mpcompc"

#ifndef SourceDir
  #define SourceDir    "..\..\outlook-mpc-addin\src\MpcOutlookAddIn\bin\Release"
#endif

#if !FileExists(AddBackslash(SourceDir) + AddInName + ".vsto")
  #error The add-in is not built: MpcOutlookAddIn.vsto is missing from SourceDir. Build MpcOutlookAddIn in Release first, or run build-installer.ps1
#endif

#ifndef AppVersion
  #define AppVersion   GetVersionNumbersString(AddBackslash(SourceDir) + AddInName + ".dll")
#endif

; Outlook\Addins\<this> key, used for both HKCU and HKLM.
#define AddInKey       "Software\Microsoft\Office\Outlook\Addins\" + AddInName

[Setup]
; AppId identifies the product for upgrades and uninstall; keep it unchanged between versions.
AppId={{6F1D2B7A-3C4E-4B8F-9A51-0D7E2C9B4F10}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppUrl}
AppSupportURL={#AppUrl}
VersionInfoVersion={#AppVersion}
VersionInfoDescription={#AppName} Setup

DefaultDirName={code:InstallRoot}\{#AddInName}
DisableDirPage=auto
DisableProgramGroupPage=yes
DisableReadyPage=no

PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog commandline
UsedUserAreasWarning=no
ArchitecturesAllowed=x86compatible x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=6.1sp1

; Outlook keeps the add-in's DLL open; offer to close it when files need replacing.
CloseApplications=yes
RestartApplications=no

OutputDir=Output
OutputBaseFilename={#AddInName}-Setup-{#AppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
UninstallDisplayName={#AppName}
UninstallDisplayIcon={sys}\shell32.dll,43
SetupLogging=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Messages]
FinishedLabelNoIcons=Setup has installed [name] on your computer.%n%nIf Outlook is open, close it and start it again to load the add-in. It adds an MPC group to the Home tab and a Send via MPC button to new messages.

[Files]
; The deployment and application manifests are signed and list the DLL's hash: they must ship
; exactly as built, alongside the DLL.
Source: "{#SourceDir}\{#AddInName}.dll";          DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\{#AddInName}.dll.manifest"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\{#AddInName}.vsto";         DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\{#AddInName}.dll.config";   DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#SourceDir}\Microsoft.Office.Tools.Common.v4.0.Utilities.dll";  DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\Microsoft.Office.Tools.Outlook.v4.0.Utilities.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vsto-trust.ps1"; DestDir: "{app}"; Flags: ignoreversion

[Registry]
; Outlook loads add-ins listed under Software\Microsoft\Office\Outlook\Addins. HKA is HKCU for a
; "just me" install and HKLM for "all users". HKLM is split into 32-bit and 64-bit views, and
; Office reads the view that matches its own bitness, so register in both; HKCU has one view, so
; there the two entries are the same key.
Root: HKA32; Subkey: "{#AddInKey}"; Flags: uninsdeletekey
Root: HKA32; Subkey: "{#AddInKey}"; ValueType: string; ValueName: "FriendlyName"; ValueData: "{#AppName}"
Root: HKA32; Subkey: "{#AddInKey}"; ValueType: string; ValueName: "Description"; ValueData: "Splits outgoing mail across MPC relays (X-MPC-* headers) and rejoins received chunks."
Root: HKA32; Subkey: "{#AddInKey}"; ValueType: dword;  ValueName: "LoadBehavior"; ValueData: "3"
Root: HKA32; Subkey: "{#AddInKey}"; ValueType: string; ValueName: "Manifest"; ValueData: "{code:ManifestValue}"

Root: HKA64; Subkey: "{#AddInKey}"; Flags: uninsdeletekey; Check: IsWin64
Root: HKA64; Subkey: "{#AddInKey}"; ValueType: string; ValueName: "FriendlyName"; ValueData: "{#AppName}"; Check: IsWin64
Root: HKA64; Subkey: "{#AddInKey}"; ValueType: string; ValueName: "Description"; ValueData: "Splits outgoing mail across MPC relays (X-MPC-* headers) and rejoins received chunks."; Check: IsWin64
Root: HKA64; Subkey: "{#AddInKey}"; ValueType: dword;  ValueName: "LoadBehavior"; ValueData: "3"; Check: IsWin64
Root: HKA64; Subkey: "{#AddInKey}"; ValueType: string; ValueName: "Manifest"; ValueData: "{code:ManifestValue}"; Check: IsWin64

; Stop Outlook 2013 (15.0) and later (16.0) from disabling the add-in because it started slowly
; (Outlook's add-in resiliency). This list is per user.
Root: HKCU; Subkey: "Software\Microsoft\Office\15.0\Outlook\Resiliency\DoNotDisableAddinList"; ValueType: dword; ValueName: "{#AddInName}"; ValueData: "1"; Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\Microsoft\Office\16.0\Outlook\Resiliency\DoNotDisableAddinList"; ValueType: dword; ValueName: "{#AddInName}"; ValueData: "1"; Flags: uninsdeletevalue

[Code]
const
  NetFx48Release = 528040;
  VstoRuntimeUrl = 'https://www.microsoft.com/download/details.aspx?id=105522';

{ Program Files for all users; %LOCALAPPDATA%\Programs (Windows' per-user Program Files) for
  "just me". }
function InstallRoot(Param: String): String;
begin
  if IsAdminInstallMode then
    Result := ExpandConstant('{autopf}')
  else
    Result := ExpandConstant('{localappdata}\Programs');
end;

{ "file:///C:/Users/me/AppData/Local/Programs/MpcOutlookAddIn/MpcOutlookAddIn.vsto|vstolocal":
  the form Visual Studio writes. |vstolocal makes the VSTO runtime load the add-in from this
  folder instead of copying it into the ClickOnce cache. }
function ManifestValue(Param: String): String;
var
  Path: String;
begin
  Path := ExpandConstant('{app}\{#AddInName}.vsto');
  StringChangeEx(Path, '\', '/', True);
  Result := 'file:///' + Path + '|vstolocal';
end;

function HasNetFx48: Boolean;
var
  Release: Cardinal;
begin
  Result := RegQueryDWordValue(HKLM, 'SOFTWARE\Microsoft\NET Framework Setup\NDP\v4\Full', 'Release', Release)
            and (Release >= NetFx48Release);
end;

{ The VSTO 2010 runtime: the redistributable writes v4R; the copy that ships with Office writes v4. }
function RuntimeVersionAt(RootKey: Integer; SubKey: String): String;
begin
  if not RegQueryStringValue(RootKey, SubKey, 'Version', Result) then Result := '';
end;

function HasVstoRuntime: Boolean;
begin
  Result := (RuntimeVersionAt(HKLM32, 'SOFTWARE\Microsoft\VSTO Runtime Setup\v4R') <> '') or
            (RuntimeVersionAt(HKLM32, 'SOFTWARE\Microsoft\VSTO Runtime Setup\v4') <> '');
  if not Result and IsWin64 then
    Result := (RuntimeVersionAt(HKLM64, 'SOFTWARE\Microsoft\VSTO Runtime Setup\v4R') <> '') or
              (RuntimeVersionAt(HKLM64, 'SOFTWARE\Microsoft\VSTO Runtime Setup\v4') <> '');
end;

function InitializeSetup: Boolean;
var
  ErrorCode: Integer;
begin
  Result := True;

  if not HasNetFx48 then
  begin
    SuppressibleMsgBox('The MPC Outlook add-in needs the .NET Framework 4.8 or later, which is not installed.' + #13#10#13#10 +
                       'Install it from Windows Update or https://dotnet.microsoft.com/download/dotnet-framework/net48, then run this setup again.',
                       mbCriticalError, MB_OK, IDOK);
    Result := False;
    Exit;
  end;

  if not HasVstoRuntime then
  begin
    Log('VSTO runtime not found in the registry.');
    case SuppressibleMsgBox('The Visual Studio 2010 Tools for Office Runtime was not found. Outlook needs it to load the add-in.' + #13#10#13#10 +
                            'Click Yes to open its download page (install it, then run this setup again), No to install the add-in anyway, or Cancel to quit.',
                            mbConfirmation, MB_YESNOCANCEL, IDNO) of
      IDYES:
        begin
          ShellExecAsOriginalUser('open', VstoRuntimeUrl, '', '', SW_SHOWNORMAL, ewNoWait, ErrorCode);
          Result := False;
        end;
      IDCANCEL:
        Result := False;
    end;
  end;
end;

function PowerShellPath: String;
begin
  Result := ExpandConstant('{sys}\WindowsPowerShell\v1.0\powershell.exe');
end;

{ Adds or removes this user's VSTO inclusion list entry. Failure is not fatal: Outlook then asks
  once whether to trust the add-in. }
procedure RunTrustScript(Action: String);
var
  Params: String;
  ResultCode: Integer;
begin
  Params := '-NoProfile -NonInteractive -ExecutionPolicy Bypass -File "' + ExpandConstant('{app}\vsto-trust.ps1') + '"' +
            ' -Action ' + Action + ' -Manifest "' + ExpandConstant('{app}\{#AddInName}.vsto') + '"';
  if not FileExists(PowerShellPath) then
    Log('Windows PowerShell not found; skipping the VSTO inclusion list (' + Action + ').')
  else if Exec(PowerShellPath, Params, '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
    Log('vsto-trust.ps1 ' + Action + ' exited with ' + IntToStr(ResultCode) + '.')
  else
    Log('Could not run vsto-trust.ps1: ' + SysErrorMessage(ResultCode));
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  { Program Files installs are trusted already. }
  if (CurStep = ssPostInstall) and not IsAdminInstallMode then
    RunTrustScript('Add');
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  { Before [Files] are removed, while vsto-trust.ps1 and the manifest are still there. }
  if (CurUninstallStep = usUninstall) and not IsAdminInstallMode then
    RunTrustScript('Remove');
end;
