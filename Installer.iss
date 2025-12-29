; Windows Explorer Extension Installer
; Inno Setup Script

[Setup]
AppName=Windows Explorer Extension
AppVersion=1.0
AppPublisher=anpho
AppPublisherURL=https://github.com/anpho/WindowsExplorerExtension
AppSupportURL=https://github.com/anpho/WindowsExplorerExtension/issues
AppUpdatesURL=https://github.com/anpho/WindowsExplorerExtension
DefaultDirName={localappdata}\ExplorerEnhancement
DefaultGroupName=ExplorerEnhancement
OutputBaseFilename=ExplorerExtensionSetup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=lowest
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; 32-bit files
Source: "src\build\ExplorerEnhancement_x86.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin32
Source: "src\build\ExplorerEnhancementManager_x86.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin32

; 64-bit files
Source: "src\build\ExplorerEnhancement_x64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64
Source: "src\build\ExplorerEnhancementManager_x64.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: IsWin64

[Icons]
Name: "{group}\{cm:ProgramOnTheWeb,Windows Explorer Extension}"; Filename: "https://github.com/anpho/WindowsExplorerExtension"
Name: "{group}\{cm:UninstallProgram,Windows Explorer Extension}"; Filename: "{uninstallexe}"
Name: "{group}\Explorer Enhancement Manager"; Filename: "{app}\{code:GetManagerExeName}"
Name: "{commondesktop}\{cm:ProgramOnTheWeb,Windows Explorer Extension}"; Filename: "https://github.com/anpho/WindowsExplorerExtension"; Tasks: desktopicon

[Run]
Filename: "{app}\{code:GetDllName}"; Parameters: "/regserver"; StatusMsg: "Registering Windows Explorer Extension..."
Filename: "{app}\{code:GetManagerExeName}"; Description: "{cm:LaunchProgram,Explorer Enhancement Manager}"; Flags: nowait postinstall skipifsilent

[UninstallRun]
Filename: "{app}\{code:GetDllName}"; Parameters: "/unregserver"; StatusMsg: "Unregistering Windows Explorer Extension..."

[Code]
function GetDllName(Param: string): string;
begin
  if IsWin64 then
    Result := 'ExplorerEnhancement_x64.dll'
  else
    Result := 'ExplorerEnhancement_x86.dll';
end;

function GetManagerExeName(Param: string): string;
begin
  if IsWin64 then
    Result := 'ExplorerEnhancementManager_x64.exe'
  else
    Result := 'ExplorerEnhancementManager_x86.exe';
end;

function IsWin32: Boolean;
begin
  Result := not IsWin64;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    MsgBox('Windows Explorer Extension has been successfully uninstalled.'#13#10'Please restart Windows Explorer or your computer for the changes to take effect.', mbInformation, MB_OK);
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    MsgBox('Windows Explorer Extension has been successfully installed.'#13#10'Please restart Windows Explorer or your computer for the changes to take effect.', mbInformation, MB_OK);
  end;
end;