# Building from source

[English](#english) · [简体中文](#简体中文)

## English

### Requirements

- Windows 10 or Windows 11.
- Visual Studio 2022 with the **Desktop development with C++** workload: v143 toolset, ATL, and a Windows SDK.
- NSIS 3.x, only to build the installer.
- WTL 10 is vendored in the repository (`src\third_party\wtl`).

### Layout

| Path | Contents |
| --- | --- |
| `src\ExplorerEnhancement` | Shell extension DLL (**New Folder +** and **Break Folders +**) |
| `src\ExplorerEnhancementManager` | Manager app; also handles `/install` and `/uninstall` |
| `src\Common` | Shared helper that loads UI strings in the user's language |
| `src\version.h` | Product version macros |
| `Installer.nsi` | NSIS installer script |
| `.github\workflows` | CI build and tag release workflows |

### Build the binaries

1. Open `src\ExplorerEnhancement.sln` in Visual Studio 2022.
2. Select **Release | x86** and build the solution (**Ctrl+Shift+B**), then **Release | x64** and build again.

Outputs are written to `src\build\`:

| Platform | Files |
| --- | --- |
| x86 | `ExplorerEnhancement_x86.dll`, `ExplorerEnhancementManager_x86.exe` |
| x64 | `ExplorerEnhancement_x64.dll`, `ExplorerEnhancementManager_x64.exe` |

### Build the installer

With both architectures built, run this from the repository root:

```bat
makensis /DVERSION=1.2.3.0 Installer.nsi
```

It writes `ExplorerExtensionSetup.exe` to the repository root. `VERSION` must be a four-part version and should match `APP_VER_STRING` in `src\version.h` (defaults to `1.0.0.0`).

### Test locally

1. Build the Release configuration for your architecture.
2. Register the DLL that sits next to the manager (normal prompt: per user; elevated prompt: all users):

```bat
src\build\ExplorerEnhancementManager_x64.exe /install
```

3. Restart Windows Explorer and test both commands. Close Explorer before replacing a DLL it has already loaded, otherwise the file is locked.
4. Unregister when you are done:

```bat
src\build\ExplorerEnhancementManager_x64.exe /uninstall
```

`regsvr32 src\build\ExplorerEnhancement_x64.dll` also works for a per-user registration; use `regsvr32 /i:machine` from an elevated prompt for all users. Registration failures are logged to `%LOCALAPPDATA%\ExplorerEnhancement\manager.log`.

### Version numbers

`src\version.h` is the single source of truth: `APP_VER_MAJOR`, `APP_VER_MINOR`, `APP_VER_PATCH`, `APP_VER_BUILD`, and the four-part `APP_VER_STRING` used in VERSIONINFO. For local builds, edit the header and pass the same version to `makensis /DVERSION`.

For releases, do not edit the header by hand: the release workflow overwrites it from the tag.

| Tag | Version |
| --- | --- |
| `v1.2` | `1.2.0.0` |
| `v1.2.3` | `1.2.3.0` |
| `v1.2.3-beta` | `1.2.3.0` (prerelease) |

### Release (maintainers)

- `.github/workflows/msbuild.yml` builds every push and pull request; it does not publish.
- `.github/workflows/create-release.yml` runs on a `v*` tag: it validates the tag, writes `src\version.h`, builds Release x86/x64, builds the installer and both zips, and publishes the GitHub release.
- Both workflows run on `windows-2022` (VS 2022 + ATL). Do not switch to `windows-latest` (VS 2026) without updating the toolset.
- The release job runs in the protected environment `release`, and the tag must be on `master`.
- Artifacts: `ExplorerExtensionSetup.exe`, `Extensions-x86-<version>.zip`, `Extensions-x64-<version>.zip`, `LICENSE.md`.

Publish:

```bat
git tag v1.2.3
git push origin v1.2.3
```

## 简体中文

### 环境要求

- Windows 10 或 Windows 11。
- Visual Studio 2022，并安装 **使用 C++ 的桌面开发** 工作负载：v143 工具集、ATL 和 Windows SDK。
- NSIS 3.x，仅在构建安装程序时需要。
- WTL 10 已随仓库提供（`src\third_party\wtl`）。

### 目录结构

| 路径 | 内容 |
| --- | --- |
| `src\ExplorerEnhancement` | Shell 扩展 DLL（**新建文件夹+** 和 **摊开文件夹+**） |
| `src\ExplorerEnhancementManager` | 管理器程序，同时负责 `/install` 和 `/uninstall` |
| `src\Common` | 按用户界面语言加载字符串的共用辅助头文件 |
| `src\version.h` | 产品版本宏 |
| `Installer.nsi` | NSIS 安装脚本 |
| `.github\workflows` | CI 构建与标签发布工作流 |

### 构建 DLL 和 EXE

1. 在 Visual Studio 2022 中打开 `src\ExplorerEnhancement.sln`。
2. 选择 **Release | x86** 并生成解决方案（**Ctrl+Shift+B**），再选择 **Release | x64** 生成一次。

输出写在 `src\build\`：

| 平台 | 文件 |
| --- | --- |
| x86 | `ExplorerEnhancement_x86.dll`、`ExplorerEnhancementManager_x86.exe` |
| x64 | `ExplorerEnhancement_x64.dll`、`ExplorerEnhancementManager_x64.exe` |

### 构建安装程序

两个架构都构建完成后，在仓库根目录运行：

```bat
makensis /DVERSION=1.2.3.0 Installer.nsi
```

会在仓库根目录生成 `ExplorerExtensionSetup.exe`。`VERSION` 必须是四段版本号，并与 `src\version.h` 中的 `APP_VER_STRING` 一致（默认 `1.0.0.0`）。

### 本地测试

1. 构建对应架构的 Release 版本。
2. 注册与管理器同目录的 DLL（普通命令提示符：按用户注册；管理员命令提示符：所有用户）：

```bat
src\build\ExplorerEnhancementManager_x64.exe /install
```

3. 重启 Windows 资源管理器并测试两个命令。替换已被资源管理器加载的 DLL 前，请先关闭资源管理器，否则文件会被锁定。
4. 用完注销：

```bat
src\build\ExplorerEnhancementManager_x64.exe /uninstall
```

`regsvr32 src\build\ExplorerEnhancement_x64.dll` 也可以按用户注册；要给所有用户注册，请以管理员身份运行 `regsvr32 /i:machine`。注册失败的信息会写入 `%LOCALAPPDATA%\ExplorerEnhancement\manager.log`。

### 版本号

`src\version.h` 是唯一来源：`APP_VER_MAJOR`、`APP_VER_MINOR`、`APP_VER_PATCH`、`APP_VER_BUILD`，以及用于 VERSIONINFO 的四段字符串 `APP_VER_STRING`。本地构建时修改该头文件，并把相同版本传给 `makensis /DVERSION`。

发布时不要手动修改头文件：发布工作流会用标签覆盖它。

| 标签 | 版本 |
| --- | --- |
| `v1.2` | `1.2.0.0` |
| `v1.2.3` | `1.2.3.0` |
| `v1.2.3-beta` | `1.2.3.0`（预发布） |

### 发布（维护者）

- `.github/workflows/msbuild.yml` 在每次推送和拉取请求时构建，不发布。
- `.github/workflows/create-release.yml` 在推送 `v*` 标签时运行：校验标签、写入 `src\version.h`、构建 Release x86/x64、构建安装程序和两个压缩包，并发布 GitHub Release。
- 两个工作流都运行在 `windows-2022`（VS 2022 + ATL）上。不要在不调整工具集的情况下改用 `windows-latest`（VS 2026）。
- 发布任务在受保护环境 `release` 中运行，且标签必须位于 `master`。
- 产物：`ExplorerExtensionSetup.exe`、`Extensions-x86-<version>.zip`、`Extensions-x64-<version>.zip`、`LICENSE.md`。

发布：

```bat
git tag v1.2.3
git push origin v1.2.3
```
