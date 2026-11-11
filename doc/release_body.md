# anpho's Extensions

Two commands for the Windows Explorer right-click menu: **New Folder +** and **Break Folders +**. For Windows 10 and Windows 11. Installation is per user: no administrator rights, no Visual C++ Redistributable, no .NET.

## What's included

- `ExplorerExtensionSetup.exe` - installer (recommended)
- `Extensions-x86-<version>.zip` - 32-bit binaries
- `Extensions-x64-<version>.zip` - 64-bit binaries
- `LICENSE.md` - license terms

## Install

1. Download `ExplorerExtensionSetup.exe` and run it. No administrator rights are needed.
2. The installer copies the program to `%LOCALAPPDATA%\ExplorerEnhancement`, registers the extension, and opens the manager.
3. If the commands do not appear, restart Windows Explorer. On Windows 11, choose **Show more options** (or press **Shift+F10**) to find them.

### Manual installation

Download the zip for your Windows architecture, extract it, and run one of these commands from a normal (non-admin) command prompt in the extracted folder:

- 32-bit Windows: `regsvr32 ExplorerEnhancement_x86.dll`
- 64-bit Windows: `regsvr32 ExplorerEnhancement_x64.dll`

Restart Windows Explorer afterwards. Use `regsvr32 /u` to unregister. A manual installation creates no Start Menu shortcut and no uninstall entry.

## Notes

- **New Folder +**: select one or more files/folders, right-click, choose **New Folder +**. A new folder named after the common prefix of the selected names is created next to the first item, and the selected items are moved into it.
- **Break Folders +**: select one or more folders, right-click, choose **Break Folders +**. All files under each folder (at every nesting level) are moved to that folder's parent. Empty folders are left behind; delete them yourself.
- The moves are normal shell file operations, so you can undo them with **Ctrl+Z** in Explorer.
- On Windows 11 the commands are classic context-menu handlers, so they appear under **Show more options** (Shift+F10), not in the new Windows 11 top-level menu.
- The commands work on filesystem items only (local and UNC paths). Paths longer than MAX_PATH are not handled.
- The manager app in the Start Menu turns each command on or off. Restart Explorer after applying changes.

## License

Proprietary software. Personal non-commercial use is free. Commercial use requires a paid license: see `LICENSE.md` or contact support@anpho.cn.

## Contact

support@anpho.cn | https://x.com/anpho
