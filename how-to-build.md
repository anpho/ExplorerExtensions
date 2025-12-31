# How to Build

This document describes how to build the Windows Explorer Extension project.

## Prerequisites

- [Git](https://git-scm.com/downloads) installed
- [Visual Studio 2019 or later](https://visualstudio.microsoft.com/downloads/) with the following workloads:
  - Desktop development with C++
  - Universal Windows Platform development (optional, for UWP support)

## Step 1: Clone the Repository

Open a terminal or command prompt and run the following command to clone the repository:

```bash
git clone https://github.com/anpho/WindowsExplorerExtension.git
cd WindowsExplorerExtension
```

## Step 2: Open the Solution in Visual Studio

Navigate to the cloned repository folder and open the solution file:

```
src/ExplorerEnhancement.sln
```

You can do this in two ways:

1. **Using File Explorer**: 
   - Navigate to the `src` folder
   - Double-click on `ExplorerEnhancement.sln` to open it in Visual Studio

2. **Using Visual Studio**: 
   - Launch Visual Studio
   - Click on "File" > "Open" > "Project/Solution"
   - Navigate to the `src` folder and select `ExplorerEnhancement.sln`

## Step 3: Build the Project

Once the solution is open in Visual Studio:

1. **Select the Build Configuration**: 
   - Choose either "Debug" or "Release" from the configuration dropdown menu
   - Select the platform ("x86" or "x64")

2. **Build the Solution**: 
   - Click on "Build" > "Build Solution" from the menu
   - Or use the keyboard shortcut `Ctrl+Shift+B`

## Step 4: Build the Installer (Optional)

If you want to build the installer package:

1. Make sure you have [NSIS (Nullsoft Scriptable Install System)](https://nsis.sourceforge.io/Download) installed
2. Open the `Installer.nsi` file in NSIS
3. Click on "Compile NSIS Script" to build the installer

The compiled installer will be named `ExplorerExtensionSetup.exe`.

## Troubleshooting

- If you encounter missing dependencies, make sure all required Visual Studio workloads are installed
- If you have issues with the build, try cleaning the solution first ("Build" > "Clean Solution") and then rebuilding
- Ensure you're using a compatible version of Visual Studio (2019 or later)