[![MSBuild](https://github.com/anpho/WindowsExplorerExtension/actions/workflows/msbuild.yml/badge.svg)](https://github.com/anpho/WindowsExplorerExtension/actions/workflows/msbuild.yml)


# WindowsExplorerExtension
Extensions for Windows Explorer, tested on Windows 10 & Windows 11. 

### New Folder Extension

A GNOME Nautilus inspired feature that when you select several files/folders then right-click, create a new folder, all the selected files/folders will be moved to a new folder whose name is based on the common part of each file's filename.

**How to Use**
> Select some files/folders, right-click, choose ```New Folder +```.

### Break Folders Extension

Tired of folders in folders? This will break folder structure and move all files outside.

**How to Use**
> Select some folder(s), right-click, choose ```Break Folders +```.



## How to Install

### Option 1: Using Installer (Recommended)
Go to ```Releases``` and download the installer file `ExplorerExtensionSetup.exe`.
1. Run the installer (no administrator privileges required)
2. Follow the on-screen instructions
3. Restart Windows Explorer or your computer in case the menu item doesn't show up.

### Option 2: Manual Installation
Go to ```Releases``` and download the file ExplorerExtension.dll, open a terminal, then use:
```bash
# For 32-bit systems
regsvr32 ExplorerEnhancement_x86.dll

# For 64-bit systems  
regsvr32 ExplorerEnhancement_x64.dll
```

Use `regsvr32 /u` to unregister the DLL.

Note: The manual installation method doesn't require administrator privileges either.


### Contact

[Twitter](https://x.com/anpho)