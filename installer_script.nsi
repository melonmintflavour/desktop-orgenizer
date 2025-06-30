; Installer script for DesktopOrgUtility
; Requires NSIS (Nullsoft Scriptable Install System) - https://nsis.sourceforge.io/

;--------------------------------
; Basic Installer Attributes
;--------------------------------

!define APP_NAME "DesktopOrgUtility"
!define APP_VERSION "1.0"
!define COMPANY_NAME "YourCompanyName" ; TODO: Replace with actual company name or remove
!define EXE_NAME "DesktopOrgUtility.exe"
!define MUI_ICON "path\\to\\your\\app_icon.ico" ; TODO: Replace with actual icon path
!define MUI_UNICON "path\\to\\your\\uninst_icon.ico" ; TODO: Replace with actual icon path (can be same as app_icon)
!define INSTALLER_OUTPUT_NAME "${APP_NAME}_${APP_VERSION}_Setup.exe"

Name "${APP_NAME} ${APP_VERSION}"
OutFile "${INSTALLER_OUTPUT_NAME}"
InstallDir "$PROGRAMFILES\${APP_NAME}"
InstallDirRegKey HKLM "Software\${COMPANY_NAME}\${APP_NAME}" "Install_Dir"
RequestExecutionLevel admin ; Request admin privileges for installation

;--------------------------------
; Modern UI 2 Setup
;--------------------------------
!include "MUI2.nsh"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page (optional)
; !insertmacro MUI_PAGE_LICENSE "path\\to\\your\\license.txt" ; TODO: Add license if applicable
; Components page (if you have optional components, not used in this basic script)
; !insertmacro MUI_PAGE_COMPONENTS
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page (installation progress)
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\${EXE_NAME}" ; Option to run the application after install
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Language Setup (English is default)
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Globals
;--------------------------------
Var VCRedistNeedsInstall ; Variable to track if VC Redist needs to be installed

;--------------------------------
; Version Information (for Add/Remove Programs)
;--------------------------------
VIProductVersion "${APP_VERSION}.0.0"
VIAddVersionKey "ProductName" "${APP_NAME}"
VIAddVersionKey "CompanyName" "${COMPANY_NAME}"
VIAddVersionKey "LegalCopyright" "© ${COMPANY_NAME}" ; TODO: Update copyright
VIAddVersionKey "FileDescription" "${APP_NAME} Application"
VIAddVersionKey "FileVersion" "${APP_VERSION}"

;--------------------------------
; Helper function to check for Visual C++ Redistributable
; This is a simplified check. A more robust check would verify the specific version.
; For VS 2015/2017/2019/2022, the Universal CRT is usually the main dependency.
; Key for VC++ 2015-2022 Redistributable (x86 or x64 based on your app target)
; Check for x64 version:
; HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\14.0\VC\Runtimes\x64 -> "Installed"=1
; Or for x86:
; HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86 -> "Installed"=1
; More reliable: check for specific files like msvcp140.dll, vcruntime140.dll in System32/SysWOW64
; Or use the registry keys for the redistributable package itself.
; Example for VC++ 2015-2022 x64: DisplayName "Microsoft Visual C++ 2015-2022 Redistributable (x64) - <version>"
;--------------------------------
Function .onInit
    SetShellVarContext all ; Ensure registry checks are done for all users if possible

    StrCpy $VCRedistNeedsInstall "true" ; Assume it needs to be installed

    ; --- Simplified Check for VC++ Redistributable (Example for 2015-2022 x64) ---
    ; A more robust check would iterate through Uninstall keys or check specific file versions.
    ; This example checks for a common DLL. This is NOT foolproof.
    IfFileExists "$WINDIR\System32\msvcp140.dll" PathGood NoPath
    PathGood:
        StrCpy $VCRedistNeedsInstall "false"
    NoPath:

    ; If you bundle the redistributable, you might prompt here or set a flag.
    If $VCRedistNeedsInstall == "true"
        MessageBox MB_OK|MB_ICONINFORMATION "This application requires the Microsoft Visual C++ Redistributable. The installer will attempt to guide you if it's missing, or you may need to install it manually."
    EndIf
FunctionEnd


;--------------------------------
; Installation Section
;--------------------------------
Section "Install ${APP_NAME}" SEC_APP
    SetOutPath $INSTDIR

    ; TODO: Ensure your compiled DesktopOrgUtility.exe is in a 'dist' folder relative to this script,
    ; or update the path.
    File "build\\Debug\\${EXE_NAME}" ; Main application executable
    ; File "path\\to\\other\\files\\*" ; Add other necessary files (DLLs, assets, etc.)
    ; File /r "path\\to\\assets_folder" ; Recursively add a folder

    ; Create Uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Create Start Menu Shortcuts
    CreateDirectory "$SMPROGRAMS\${APP_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk" "$INSTDIR\${EXE_NAME}"
    CreateShortCut "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk" "$INSTDIR\Uninstall.exe"

    ; Add to Add/Remove Programs
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" "$INSTDIR\${EXE_NAME},0"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" "${APP_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" "${COMPANY_NAME}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1

    WriteRegStr HKLM "Software\${COMPANY_NAME}\${APP_NAME}" "Install_Dir" "$INSTDIR"

    ; Dependency Installation (Visual C++ Redistributable)
    ; This part is tricky. Ideally, you bundle the redistributable or use a bootstrapper.
    ; If $VCRedistNeedsInstall == "true"
    ;    MessageBox MB_YESNO|MB_ICONQUESTION "The Visual C++ Redistributable is required. Would you like to download and install it now? (Opens a web browser)" IDYES DownloadRedist IDNO SkipRedist
    ;    DownloadRedist:
    ;        ExecShell "open" "https://aka.ms/vs/17/release/vc_redist.x64.exe" ; Or x86, direct link to VS 2022 x64
    ;        ; Installer should pause or user should be instructed to run it and then continue.
    ;        MessageBox MB_OK|MB_ICONINFORMATION "Please download and run the Visual C++ Redistributable installer. After it completes, you may need to re-run this application's installer if issues persist."
    ;    SkipRedist:
    ; EndIf
SectionEnd

;--------------------------------
; Uninstallation Section
;--------------------------------
Section "Uninstall" SEC_UNINSTALL
    ; Remove files
    Delete "$INSTDIR\${EXE_NAME}"
    Delete "$INSTDIR\Uninstall.exe"
    ; Delete other files...
    ; RMDir /r "$INSTDIR\assets_folder" ; If you have folders

    ; Remove shortcuts
    Delete "$SMPROGRAMS\${APP_NAME}\${APP_NAME}.lnk"
    Delete "$SMPROGRAMS\${APP_NAME}\Uninstall ${APP_NAME}.lnk"
    RMDir "$SMPROGRAMS\${APP_NAME}"

    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
    DeleteRegKey HKLM "Software\${COMPANY_NAME}\${APP_NAME}"

    SetOutPath $TEMP ; Avoid issues with locked directory
    RMDir "$INSTDIR" ; Remove main installation directory (only if empty)
SectionEnd
