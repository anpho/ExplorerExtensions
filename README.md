[![MSBuild](https://github.com/anpho/ExplorerExtensions/actions/workflows/msbuild.yml/badge.svg)](https://github.com/anpho/ExplorerExtensions/actions/workflows/msbuild.yml)
[![Create Release](https://github.com/anpho/ExplorerExtensions/actions/workflows/create-release.yml/badge.svg)](https://github.com/anpho/ExplorerExtensions/actions/workflows/create-release.yml)

# ExplorerExtensions

English | [简体中文](README_CN.md)

anpho's Extensions adds two commands to the Windows Explorer right-click menu: **New Folder +** and **Break Folders +**. It is a native C++ shell extension for Windows 10 and Windows 11. Installation is per user. It needs no administrator rights, no Visual C++ Redistributable, no .NET, and no MFC.

## Features

### New Folder +

Select one or more files or folders, right-click, and choose **New Folder +**.

- A new folder is created next to the first selected item.
- The folder is named after the longest common prefix of the selected names. File extensions are stripped first. If there is no usable common prefix, the name of the first selected item is used.
- All selected items are moved into the new folder.
- The move is a shell file operation, so it can be undone with Explorer's Undo (**Ctrl+Z**).

The command is hidden when exactly one folder is selected. Files are not affected by this rule.

### Break Folders +

Select one or more folders, right-click, and choose **Break Folders +**.

- For each selected folder, every file below it (at all nesting levels) is moved into that folder's parent folder.
- The empty folder tree is left in place. Delete it yourself when you no longer need it.
- The moves are shell file operations, so they can be undone with Explorer's Undo (**Ctrl+Z**).
- If a move fails, an error dialog is shown instead of failing silently.

### How to use

1. Select the items in Explorer.
2. Right-click and choose **New Folder +** or **Break Folders +**.
3. On Windows 11, click **Show more options** (or press **Shift+F10**) first.

## Requirements

- Windows 10 or Windows 11, 32-bit or 64-bit.
- No administrator rights. Everything is installed and registered per user.
- No Visual C++ Redistributable: the binaries use the static C++ runtime.
- No .NET and no MFC.

## Installation

### Installer (recommended)

1. Download `ExplorerExtensionSetup.exe` from [Releases](https://github.com/anpho/ExplorerExtensions/releases).
2. Run it. No administrator rights are needed; the extension is installed and registered for the current user.
3. To install for all users, run the installer as administrator. The extension is then registered in `HKLM` and its settings are shared by every user.
4. The installer copies the files to `%LOCALAPPDATA%\ExplorerEnhancement`, registers the extension, adds a Start Menu shortcut for the manager, and writes an uninstall entry.
5. If the commands do not appear, restart Windows Explorer.

### Manual installation

Download `Extensions-x86-<version>.zip` (32-bit Windows) or `Extensions-x64-<version>.zip` (64-bit Windows) from [Releases](https://github.com/anpho/ExplorerExtensions/releases), then extract it.

Open a normal (non-admin) command prompt in the extracted folder and run the command for your architecture.

For 32-bit Windows:

```bat
regsvr32 ExplorerEnhancement_x86.dll
```

For 64-bit Windows:

```bat
regsvr32 ExplorerEnhancement_x64.dll
```

Then restart Windows Explorer.

Notes:

- Registration is per user, so run `regsvr32` as the same user who will use the menu.
- Use `regsvr32 /u ExplorerEnhancement_<arch>.dll` to unregister.
- For all users, run the manager with `/install` from an elevated prompt (or `regsvr32 /i:machine`); unregister with `/uninstall` from an elevated prompt.
- A manual installation creates no Start Menu shortcut and no uninstall entry.

## Enable or disable the commands

The installer adds a Start Menu shortcut for the manager. You can also start `ExplorerEnhancementManager_<arch>.exe` from the install folder. The manager has two checkboxes, **Break Folders +** and **New Folder +**, plus **Apply**, **About** and **Exit**.

- Opening the manager when the extension is not registered registers it automatically: for the current user normally, for all users when the manager is run as administrator.
- On Windows versions that block per-user COM registration (`HKCU\Software\Classes\CLSID` is hardened, error 5), the manager asks for UAC and installs for all users instead. Declining UAC leaves the extension unregistered.
- If the extension is registered for all users (installed as administrator), the manager offers to restart itself with administrator rights so the settings can be changed. If you decline, the window is read-only and **Apply** is disabled.
- After a UAC-assisted installation the manager restarts itself elevated automatically, so you get a usable window instead of a greyed-out one.
- **Shift+click Exit** (or Shift+close the window) unregisters the extension after a confirmation. Removing an all-users registration requires running the manager as administrator.
- A disabled command is recorded per user or per machine in `HKCU`/`HKLM` `\Software\anpho\ExplorerEnhancement` (the DLL checks it each time the menu opens).
- If the change does not show up in an already-open Explorer window, restart Windows Explorer.

The manager also has a command-line mode:

- `ExplorerEnhancementManager_<arch>.exe /install` registers the DLL that sits next to the exe.
- `ExplorerEnhancementManager_<arch>.exe /uninstall` unregisters it.

Both run without a window and return exit code `0` on success and `1` on failure. The installer uses them.

## Uninstall

- Open **Settings > Apps**, find **anpho's Extensions**, and click **Uninstall**.
- Or run `%LOCALAPPDATA%\ExplorerEnhancement\uninstall.exe`.
- The uninstaller runs the manager with `/uninstall`, then removes the files, the Start Menu shortcuts, the uninstall entry, and the per-user registration and settings.
- Silent uninstall: `uninstall.exe /S`.
- Restart Windows Explorer afterwards.

## Notes and limitations

- **Windows 11 menu:** the commands are classic context-menu handlers. They appear under **Show more options** (Shift+F10), not in the new Windows 11 top-level menu.
- **Filesystem items only:** the commands work on local paths and UNC paths. Items from virtual folders, archives, or cloud storage that are not real filesystem paths are not supported.
- **Empty folders remain:** **Break Folders +** moves files only. The empty folder tree stays in place until you delete it.
- **Long paths:** paths longer than `MAX_PATH` (260 characters) are not handled.
- **Single folder selection:** **New Folder +** is hidden when exactly one folder is selected.

## Build

See [how-to-build.md](how-to-build.md) for the build prerequisites, the local build and installer steps, versioning, and the release process. GitHub Actions builds tagged releases.

## License

This project is proprietary software. Personal non-commercial use is free. Any commercial use requires a paid license; contact support@anpho.cn to obtain one.

See [LICENSE.md](LICENSE.md) for the full terms.

## Contact

- Email: support@anpho.cn
- X (Twitter): [@anpho](https://x.com/anpho)
