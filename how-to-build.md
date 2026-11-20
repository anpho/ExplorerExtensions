# How to Build

This document describes how to build ExplorerExtensions from source and how to produce the installer.

[简体中文](how-to-build-cn.md)

## Prerequisites

- Windows 10 or Windows 11.
- Visual Studio 2022 with the **Desktop development with C++** workload. This provides the v143 toolset, ATL, and a Windows 10/11 SDK.
- NSIS 3.x (only needed to build the installer).
- Git (only needed to clone the repository).

WTL 10 is vendored in the repository (`src\third_party\wtl`).

## Repository layout

- `src\ExplorerEnhancement` - the shell extension DLL project (**New Folder +** and **Break Folders +**).
- `src\ExplorerEnhancementManager` - the manager app; it also handles `/install` and `/uninstall`.
- `src\Common` - small header shared by both projects (loads UI strings in the user's language).
- `src\third_party\wtl` - WTL 10 headers, vendored. Licensed under the Microsoft Public License; see `src\third_party\wtl\MS-PL.txt`.
- `src\version.h` - the single source of the product version (`APP_VER_*` macros and `APP_VER_STRING`).
- `Installer.nsi` - the NSIS installer script.
- `.github\workflows` - the CI build and tag release workflows.

## Build the binaries

1. Open `src\ExplorerEnhancement.sln` in Visual Studio 2022.
2. Select the **Release** configuration and the **x86** platform, then build the solution (**Build > Build Solution**, `Ctrl+Shift+B`).
3. Select the **x64** platform and build again.

Both projects build for x86 and x64. Release outputs are written to `src\build\`:

| Platform | Files |
| --- | --- |
| x86 | `ExplorerEnhancement_x86.dll`, `ExplorerEnhancementManager_x86.exe` |
| x64 | `ExplorerEnhancement_x64.dll`, `ExplorerEnhancementManager_x64.exe` |

## Build the installer

1. Install NSIS 3.x and make sure `makensis` is available in your command prompt.
2. Build the Release x86 and x64 binaries first (see above). The installer carries both and installs the one that matches Windows.
3. From the repository root, run:

```bat
makensis /DVERSION=1.2.3.0 Installer.nsi
```

The installer is written to `ExplorerExtensionSetup.exe` in the repository root.

- `/DVERSION` must be a four-part version such as `1.2.3.0`. The default is `1.0.0.0`.
- Pass the same version as `APP_VER_STRING` in `src\version.h` (see below).

## Test a local build

1. Build the Release configuration for your Windows architecture.
2. Register the extension with the manager. It registers the DLL that sits next to it. Run it normally for a per-user registration, or from an elevated prompt to register for all users (`HKLM`):

```bat
src\build\ExplorerEnhancementManager_x64.exe /install
```

Use `ExplorerEnhancementManager_x86.exe` on 32-bit Windows. The exit code is `0` on success and `1` on failure.

3. Restart Windows Explorer. If a previous version of the DLL is already loaded by Explorer, close/restart Explorer before replacing or rebuilding it, otherwise the file is locked.
4. Test both commands on files and folders.
5. Unregister when you are done, then restart Windows Explorer again:

```bat
src\build\ExplorerEnhancementManager_x64.exe /uninstall
```

You can also register the DLL with `regsvr32` as a normal (non-admin) user: `regsvr32 src\build\ExplorerEnhancement_x64.dll`. For an all-users registration use `regsvr32 /i:machine` from an elevated prompt.

If registration fails, the manager writes the failing key and error code to `%LOCALAPPDATA%\ExplorerEnhancement\manager.log`.

## Version numbers

`src\version.h` is the single source of truth for the product version:

- `APP_VER_MAJOR`, `APP_VER_MINOR`, `APP_VER_PATCH`, `APP_VER_BUILD` - the numeric parts.
- `APP_VER_STRING` - the four-part version string used in the VERSIONINFO resources, for example `1.0.0.0`.

Both resource files include this header. The installer takes its own version from `/DVERSION`.

To bump the version for a local build, edit `src\version.h` (all macros) and pass the same version to `makensis /DVERSION`.

For a release, do not edit the header by hand. The release workflow overwrites `src\version.h` from the git tag:

| Tag | Version | Release type |
| --- | --- | --- |
| `v1.2` | `1.2.0.0` | normal |
| `v1.2.3` | `1.2.3.0` | normal |
| `v1.2.3-beta` | `1.2.3.0` | prerelease, suffix dropped |

## Release (maintainers)

Releases are built by GitHub Actions.

- `.github/workflows/msbuild.yml` builds every push and pull request. It does not publish anything.
- `.github/workflows/create-release.yml` runs when a tag is pushed. It validates the tag and derives the four-part version, overwrites `src\version.h`, builds Release x86 and x64, builds the installer and the two zips, and publishes the GitHub release with the artifacts.
- Both workflows run on the `windows-2022` runner image (Visual Studio 2022, v143 toolset with ATL). `windows-latest` currently points to the Visual Studio 2026 image, where the v143 toolset is installed for compatibility without ATL headers, so the build fails with `atlbase.h` not found. If you migrate to VS 2026, change the runner image and the project toolset together.

Publishing requirements:

- The release job runs in a protected environment named `release`. Add required reviewers to that environment if you want a manual approval before publishing.
- The release is published only when the tag is on `master`. A tag created on another branch is rejected.

Release artifacts:

- `ExplorerExtensionSetup.exe`
- `Extensions-x86-<version>.zip`
- `Extensions-x64-<version>.zip`
- `LICENSE.md`

To publish a release, create a tag from `master` and push it:

```bat
git tag v1.2.3
git push origin v1.2.3
```
