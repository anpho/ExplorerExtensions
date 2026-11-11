# 构建指南

本文档介绍如何从源代码构建 ExplorerExtensions，以及如何生成安装程序。

[English](how-to-build.md)

## 前提条件

- Windows 10 或 Windows 11。
- Visual Studio 2022，并安装 **使用 C++ 的桌面开发** 工作负载。该工作负载提供 v143 工具集、ATL 以及 Windows 10/11 SDK。
- NSIS 3.x（仅在构建安装程序时需要）。
- Git（仅在克隆仓库时需要）。

WTL 10 已随仓库提供（`src\third_party\wtl`）。

## 仓库结构

- `src\ExplorerEnhancement` - Shell 扩展 DLL 项目（**新建文件夹+** 和 **摊开文件夹+**）。
- `src\ExplorerEnhancementManager` - 管理器程序，同时处理 `/install` 和 `/uninstall`。
- `src\Common` - 两个项目共用的一个小头文件（按用户界面语言加载字符串）。
- `src\third_party\wtl` - 随仓库提供的 WTL 10 头文件，使用 Microsoft Public License，见 `src\third_party\wtl\MS-PL.txt`。
- `src\version.h` - 产品版本的唯一来源（`APP_VER_*` 宏和 `APP_VER_STRING`）。
- `Installer.nsi` - NSIS 安装脚本。
- `.github\workflows` - CI 构建和标签发布工作流。

## 构建程序

1. 在 Visual Studio 2022 中打开 `src\ExplorerEnhancement.sln`。
2. 选择 **Release** 配置和 **x86** 平台，然后生成解决方案（**生成 > 生成解决方案**，`Ctrl+Shift+B`）。
3. 选择 **x64** 平台，再生成一次。

两个项目都支持 x86 和 x64。Release 输出位于 `src\build\`：

| 平台 | 文件 |
| --- | --- |
| x86 | `ExplorerEnhancement_x86.dll`、`ExplorerEnhancementManager_x86.exe` |
| x64 | `ExplorerEnhancement_x64.dll`、`ExplorerEnhancementManager_x64.exe` |

## 构建安装程序

1. 安装 NSIS 3.x，并确保命令提示符中可以找到 `makensis`。
2. 先构建 Release x86 和 x64 程序（见上文）。安装程序会同时包含两种架构，并安装与 Windows 匹配的一种。
3. 在仓库根目录运行：

```bat
makensis /DVERSION=1.2.3.0 Installer.nsi
```

安装程序会生成在仓库根目录的 `ExplorerExtensionSetup.exe`。

- `/DVERSION` 必须是四段版本号，例如 `1.2.3.0`。默认值为 `1.0.0.0`。
- 请传入与 `src\version.h` 中 `APP_VER_STRING` 相同的版本号（见下文）。

## 在本地测试

1. 为当前 Windows 架构构建 Release 配置。
2. 用管理器注册扩展。它会注册位于自身旁边的 DLL。普通权限注册给当前用户；如果要注册给所有用户（HKLM），请从管理员命令提示符运行：

```bat
src\build\ExplorerEnhancementManager_x64.exe /install
```

在 32 位 Windows 上使用 `ExplorerEnhancementManager_x86.exe`。成功时退出码为 `0`，失败时为 `1`。

3. 重启 Windows 资源管理器。如果旧版 DLL 已被资源管理器加载，请先关闭/重启资源管理器再替换或重新编译，否则文件会被占用。
4. 在文件和文件夹上测试两个命令。
5. 测试完成后注销，然后再次重启 Windows 资源管理器：

```bat
src\build\ExplorerEnhancementManager_x64.exe /uninstall
```

也可以用普通（非管理员）用户通过 `regsvr32` 注册 DLL：`regsvr32 src\build\ExplorerEnhancement_x64.dll`。如果要注册给所有用户，请从管理员命令提示符使用 `regsvr32 /i:machine`。

注册失败时，管理器会把失败的注册表键和错误码写到 `%LOCALAPPDATA%\ExplorerEnhancement\manager.log`。

## 版本号

`src\version.h` 是产品版本的唯一来源：

- `APP_VER_MAJOR`、`APP_VER_MINOR`、`APP_VER_PATCH`、`APP_VER_BUILD` - 数字部分。
- `APP_VER_STRING` - 用于 VERSIONINFO 资源的四段版本字符串，例如 `1.0.0.0`。

两个资源文件都会包含此头文件。安装程序自身的版本来自 `/DVERSION`。

在本地构建时需要升级版本，请修改 `src\version.h`（所有宏），并把相同的版本号传给 `makensis /DVERSION`。

发布时不要手动修改该头文件。发布工作流会根据 git 标签覆盖 `src\version.h`：

| 标签 | 版本 | 发布类型 |
| --- | --- | --- |
| `v1.2` | `1.2.0.0` | 正式版 |
| `v1.2.3` | `1.2.3.0` | 正式版 |
| `v1.2.3-beta` | `1.2.3.0` | 预发布，去掉后缀 |

## 发布（维护者）

发布版本由 GitHub Actions 构建并签名。

- `.github/workflows/msbuild.yml` 会在每次推送和拉取请求时构建，不签名，也不发布任何内容。
- `.github/workflows/create-release.yml` 在推送标签时运行。它会校验标签并推导出四段版本号，覆盖 `src\version.h`，构建 Release x86 和 x64，签名程序，构建安装程序和两个压缩包，并发布带产物的 GitHub Release。
- 两个工作流都固定在 `windows-2022` 运行镜像（Visual Studio 2022，v143 工具集带 ATL）。`windows-latest` 目前指向 Visual Studio 2026 镜像，其中的 v143 只是兼容工具集、不带 ATL 头文件，构建会报找不到 `atlbase.h`。如果要迁移到 VS 2026，请同时修改运行镜像和项目工具集。

签名要求（仓库密钥）：

- `PFX_CERTIFICATE` - Base64 编码的 PFX 代码签名证书。
- `PFX_PASSWORD` - PFX 密码。

发布要求：

- 发布任务在名为 `release` 的受保护环境中运行。如果需要在发布前人工审批，请为该环境配置必需审核人。
- 只有标签位于 `master` 上时才会发布。在其他分支上创建的标签会被拒绝。

发布产物：

- `ExplorerExtensionSetup.exe`
- `Extensions-x86-<version>.zip`
- `Extensions-x64-<version>.zip`
- `LICENSE.md`

发布版本时，在 `master` 上创建标签并推送：

```bat
git tag v1.2.3
git push origin v1.2.3
```
