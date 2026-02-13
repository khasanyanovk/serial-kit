; SerialKit Installer Script for Inno Setup
; https://jrsoftware.org/isinfo.php
;
; To build the installer:
; 1. Install Inno Setup 6 from https://jrsoftware.org/isdl.php
; 2. Open this file in Inno Setup Compiler
; 3. Build -> Compile

#define MyAppName "SerialKit"
#define MyAppVersion "0.1.0"
#define MyAppPublisher "Kirill Khasanyanov"
#define MyAppURL "https://github.com/khasanyanovk/serial-kit"
#define MyAppExeName "serialkit-compiler.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
AppId={{8F5A7C9D-3E2B-4A1C-8F6D-9B3E5A2C7D4F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=..\LICENSE
InfoBeforeFile=..\README.md
OutputDir=.\output
OutputBaseFilename=SerialKit-{#MyAppVersion}-Setup
; SetupIconFile=serialkit.ico  ; Uncomment and add .ico file for custom icon
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ChangesEnvironment=yes
; UninstallDisplayIcon={app}\bin\{#MyAppExeName}  ; Uncomment with SetupIconFile
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "addtopath"; Description: "Add SerialKit to system PATH (allows running 'serialkit-compiler' from any directory)"; GroupDescription: "Environment:"; Flags: checkedonce

#define MainExeSource FindSource( \
    "..\build\bin\serialkit-compiler.exe", \
    "..\build\bin\Release\serialkit-compiler.exe" \
)

[Files]
Source: "{#MainExeSource}"; DestDir: "{app}\bin"; Flags: ignoreversion

; Documentation
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\CONTRIBUTING.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\docs\*"; DestDir: "{app}\docs"; Flags: ignoreversion recursesubdirs createallsubdirs

; Examples
Source: "..\examples\*"; DestDir: "{app}\examples"; Flags: ignoreversion recursesubdirs createallsubdirs

; [Icons]
; Name: "{group}\SerialKit Compiler"; Filename: "{app}\bin\{#MyAppExeName}"
; Name: "{group}\Examples Folder"; Filename: "{app}\examples"
; Name: "{group}\Documentation Folder"; Filename: "{app}\docs"
; Name: "{group}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"

[Code]
const
    EnvironmentKey = 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment';

procedure EnvAddPath(Path: string);
var
    Paths: string;
begin
    { Retrieve current path (use empty string if entry not exists) }
    if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths) then
        Paths := '';

    { Skip if string already found in path }
    if Pos(';' + Uppercase(Path) + ';', ';' + Uppercase(Paths) + ';') > 0 then begin
        Log(Format('The [%s] already exists in PATH', [Path]));
        exit;
    end;

    { Append path to the end of the path variable }
    if Paths <> '' then begin
        { Add separator if needed }
        if Paths[Length(Paths)] <> ';' then
            Paths := Paths + ';';
    end;
    Paths := Paths + Path;

    { Overwrite (or create if missing) path environment variable }
    if RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths) then begin
        Log(Format('Successfully added [%s] to PATH', [Path]));
    end else begin
        Log(Format('Error while adding the [%s] to PATH', [Path]));
    end;
end;

procedure EnvRemovePath(Path: string);
var
    Paths: string;
    P: Integer;
begin
    { Skip if registry entry not exists }
    if not RegQueryStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths) then
        exit;

    { Skip if string not found in path }
    P := Pos(';' + Uppercase(Path) + ';', ';' + Uppercase(Paths) + ';');
    if P = 0 then begin
        { Try without leading semicolon (at the start of PATH) }
        P := Pos(Uppercase(Path) + ';', Uppercase(Paths));
        if P <> 1 then begin
            Log(Format('The [%s] not found in PATH', [Path]));
            exit;
        end;
    end else begin
        P := P - 1;
    end;

    { Update path variable }
    Delete(Paths, P, Length(Path) + 1);
    
    { Clean up double semicolons }
    StringChangeEx(Paths, ';;', ';', True);
    
    { Remove trailing semicolon }
    if (Length(Paths) > 0) and (Paths[Length(Paths)] = ';') then
        Delete(Paths, Length(Paths), 1);

    { Overwrite path environment variable }
    if RegWriteStringValue(HKEY_LOCAL_MACHINE, EnvironmentKey, 'Path', Paths) then begin
        Log(Format('Successfully removed [%s] from PATH', [Path]));
    end else begin
        Log(Format('Error while removing the [%s] from PATH', [Path]));
    end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
    if CurStep = ssPostInstall then begin
        if IsTaskSelected('addtopath') then begin
            Log('Adding to PATH...');
            EnvAddPath(ExpandConstant('{app}\bin'));
        end;
    end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
    if CurUninstallStep = usPostUninstall then begin
        Log('Removing from PATH...');
        EnvRemovePath(ExpandConstant('{app}\bin'));
    end;
end;

[Run]
Filename: "{app}\examples"; Description: "Open examples folder"; Flags: postinstall shellexec skipifsilent nowait unchecked
Filename: "{app}"; Description: "Open installation folder"; Flags: postinstall shellexec skipifsilent nowait unchecked
