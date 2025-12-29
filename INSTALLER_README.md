# Windows Explorer Extension Installer

## Overview
This installer script is designed to create a professional installation package for the Windows Explorer Extension project. It uses Inno Setup, a powerful and free installer creation tool for Windows.

## Features
- **Dual Architecture Support**: Automatically detects and installs the correct 32-bit or 64-bit version based on the user's system
- **Extension Registration**: Automatically registers and unregisters the Explorer extension DLL
- **Management Tool**: Installs the Explorer Enhancement Manager application
- **Multi-language Support**: English and Simplified Chinese
- **User-friendly Interface**: Standard Windows installer interface with customizable options
- **Shortcut Creation**: Creates start menu shortcuts and optional desktop shortcuts
- **Clean Uninstallation**: Properly removes all components and registry entries

## Prerequisites
1. **Inno Setup**: Download and install from [Inno Setup](https://jrsoftware.org/isdl.php)
2. **Build the Project**: First build the project in Visual Studio to generate the required DLL and EXE files

## Building the Installer

### Step 1: Build the Project
1. Open `src/ExplorerEnhancement.sln` in Visual Studio
2. Select **Release** configuration
3. Build both projects (ExplorerEnhancement and ExplorerEnhancementManager)
4. The build output will be in `src/build/` directory

### Step 2: Create the Installer Package
1. Open `Installer.iss` in Inno Setup Compiler
2. Click **Compile** (or press F9)
3. The installer package (`ExplorerExtensionSetup.exe`) will be created in the `Output` directory

## Installer Structure

### Files Installed
- `ExplorerEnhancement_x86.dll` (32-bit systems only)
- `ExplorerEnhancement_x64.dll` (64-bit systems only)
- `ExplorerEnhancementManager_x86.exe` (32-bit systems only)
- `ExplorerEnhancementManager_x64.exe` (64-bit systems only)

### Installation Process
1. User runs `ExplorerExtensionSetup.exe` (no administrator privileges required)
2. Installer detects system architecture
3. Installs the appropriate files to `%LOCALAPPDATA%\ExplorerEnhancement`
4. Registers the Explorer extension DLL using `DllRegisterServer` (per-user registration)
5. Creates start menu shortcuts
6. Optionally creates desktop shortcuts
7. Prompts user to restart Windows Explorer for changes to take effect

### Uninstallation Process
1. User runs the uninstaller from Start Menu or Control Panel (no administrator privileges required)
2. Uninstaller unregisters the Explorer extension DLL using `DllUnregisterServer` (per-user unregistration)
3. Removes all installed files
4. Removes shortcuts and registry entries
5. Prompts user to restart Windows Explorer

## Customization

### Changing Version Information
Edit the following lines in `Installer.iss`:
```ini
AppVersion=1.0
```

### Adding More Languages
Add additional language sections in the `[Languages]` section:
```ini
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
```

### Modifying Install Location
Change the `DefaultDirName` parameter in the `[Setup]` section:
```ini
DefaultDirName={pf}\YourCustomFolder
```

## Testing

### Testing Installation
1. Build the installer package
2. Run the installer on a test machine or virtual machine
3. Verify that the extension appears in Windows Explorer context menus
4. Verify that the Explorer Enhancement Manager runs correctly

### Testing Uninstallation
1. Uninstall the extension via Start Menu or Control Panel
2. Verify that all files are removed
3. Verify that the extension no longer appears in context menus

## Troubleshooting

### Installation Fails
- Ensure you're running the installer with administrator privileges
- Check that you've built the project in Release configuration
- Verify that the DLL and EXE files exist in `src/build/` directory

### Extension Doesn't Appear in Context Menu
- Restart Windows Explorer (or your computer)
- Verify that the DLL was registered correctly by checking the registry
- Check if there are any conflicts with other Explorer extensions

## License
This installer script is released under the same license as the Windows Explorer Extension project.

## Contributing
Feel free to modify and improve this installer script to better suit your needs.
