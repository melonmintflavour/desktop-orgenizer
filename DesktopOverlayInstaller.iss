; -- DesktopOverlayInstaller.iss --
; Inno Setup Script for Desktop Overlay Utility

#define MyAppName "Desktop Overlay"
#define MyAppVersion "0.1.0" ; Should be updated with actual version
#define MyAppPublisher "YourCompanyName" ; Replace
#define MyAppURL "https://yourcompany.com/desktopoverlay" ; Replace
#define MyAppExeName "DesktopOverlay.exe"
#define MyVCRedistName "vc_redist.x64.exe" ; Ensure this matches your redist file (e.g., for VS 2015-2022)

[Setup]
AppId={{YOUR_UNIQUE_GUID_HERE}} ; IMPORTANT: Generate a new GUID for your application
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName} ; Installs to Program Files (e.g., C:\Program Files\Desktop Overlay)
DefaultGroupName={#MyAppName} ; Start Menu folder name
; DisableProgramGroupPage=yes ; Uncomment if you don't want a "Select Start Menu Folder" page
OutputBaseFilename=DesktopOverlay_Setup_{#MyAppVersion}
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin ; Required for Program Files installation and VC Redist
WizardImageFile=compiler:WizModernImage-IS.bmp ; Optional: custom wizard image
WizardSmallImageFile=compiler:WizModernSmallImage-IS.bmp ; Optional: custom wizard small image
; SignTool=YourSignToolCommand $f ; Optional: For code signing the setup.exe (recommended for production)

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Application executable
Source: "release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
; Qt DLLs and other necessary files - windeployqt output should go into 'release' folder
Source: "release\*.dll"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createsubdirs
Source: "release\plugins\*"; DestDir: "{app}\plugins"; Flags: ignoreversion recursesubdirs createsubdirs
; Add other specific folders like imageformats, styles, translations if they are in your release dir
; e.g., Source: "release\imageformats\*"; DestDir: "{app}\imageformats"; Flags: ignoreversion recursesubdirs createsubdirs

; VC++ Redistributable - assumes it's in the same directory as the .iss script or a subfolder
Source: "{#MyVCRedistName}"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
; Install VC++ Redistributable if necessary.
; The VCRedistNeedsInstall function provides a basic check.
; For production, use a more robust check or a well-tested include script for VCRedist.
Filename: "{tmp}\{#MyVCRedistName}"; Parameters: "/install /passive /norestart"; \
    StatusMsg: "Installing Microsoft Visual C++ Redistributable..."; Check: VCRedistNeedsInstall; \
    BeforeInstall: SetupPrepareToInstall; AfterInstall: SetupPostInstall

; Launch application after setup
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; \
    Flags: nowait postinstall skipifsilent unchecked

[Code]
// Function to check if a specific version of VC++ Redistributable is installed.
// This is a SIMPLIFIED example. Real checks are more complex and version-specific.
// It's highly recommended to use a pre-made Inno Setup script include for VC Redist checks,
// such as those found on GitHub (e.g., search for "InnoSetup VCRedist").
// This example broadly checks for any common redistributable DLL.
function VCRedistNeedsInstall: Boolean;
var
  Version: String;
  InstallFlag: Cardinal;
begin
  Result := True; // Assume needs install by default

  // This is a very naive check. A proper check would look for specific registry keys
  // related to the product code of the VC++ Redistributable package.
  // For example, for VS 2015-2022 x64 Redist (14.3x):
  // HKLM\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64 -> Installed=1 (REG_DWORD)
  // Or by checking file versions of msvcp140.dll, vcruntime140.dll etc. in System32.

  // For this placeholder, let's just always return true to demonstrate the [Run] section.
  // In a real script, you'd implement the actual check here.
  // if IsWin64 then begin
  //   if RegKeyExists(HKLM, 'SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64') then
  //     if RegQueryDWordValue(HKLM, 'SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', InstallFlag) then
  //       if InstallFlag = 1 then Result := False;
  // end else begin
  //   // Similar check for 32-bit if needed
  // end;

  // Log(Format('VCRedistNeedsInstall check returned: %d', [Result]));
end;

// These are optional procedures that can be used for more complex setup logic.
procedure SetupPrepareToInstall(var NeedsRestart: Boolean);
begin
  Log('SetupPrepareToInstall: Preparing to install application.');
end;

procedure SetupPostInstall(var NeedsRestart: Boolean);
begin
  Log('SetupPostInstall: Application installation files copied.');
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  Log(Format('Current setup step: %d', [CurStep]));
  // if CurStep = ssInstall then begin
  // Perform checks or preparations right before installation starts
  // end;
end;

// It's good practice to initialize VCRedistInstalled in InitializeSetup if doing more complex checks.
function InitializeSetup(): Boolean;
begin
  Log('InitializeSetup: Setup program initialized.');
  Result := True;
end;

procedure InitializeWizard();
begin
  Log('InitializeWizard: Wizard initialized.');
end;

procedure DeinitializeSetup();
begin
  Log('DeinitializeSetup: Setup program deinitialized.');
end;

[UninstallDelete]
Type: files; Name: "{app}\*" ; Note: This removes all files in the app folder. Be careful if user data is stored there.
Type: dirifempty; Name: "{app}"
Type: filesandordirs; Name: "{group}"
Type: dirifempty; Name: "{group}"
; User data in AppData (QSettings INI, SQLite DB) is typically NOT removed by default.
; Add entries here if you want to offer an option to remove user data.
; Example:
; Type: files; Name: "{userappdata}\YourCompanyName\{#MyAppName}\*"
; Type: dirifempty; Name: "{userappdata}\YourCompanyName\{#MyAppName}"
; Type: dirifempty; Name: "{userappdata}\YourCompanyName"

```
