# 构建指南

本文档介绍如何构建Windows资源管理器扩展项目。

## 前提条件

- 已安装 [Git](https://git-scm.com/downloads)
- 已安装 [Visual Studio 2019 或更高版本](https://visualstudio.microsoft.com/downloads/)，并包含以下工作负载：
  - 使用 C++ 的桌面开发
  - 通用 Windows 平台开发（可选，用于 UWP 支持）

## 步骤 1: 克隆仓库

打开终端或命令提示符，运行以下命令克隆仓库：

```bash
git clone https://github.com/anpho/WindowsExplorerExtension.git
cd WindowsExplorerExtension
```

## 步骤 2: 在 Visual Studio 中打开解决方案

导航到克隆的仓库文件夹并打开解决方案文件：

```
src/ExplorerEnhancement.sln
```

您可以通过两种方式执行此操作：

1. **使用文件资源管理器**：
   - 导航到 `src` 文件夹
   - 双击 `ExplorerEnhancement.sln` 在 Visual Studio 中打开

2. **使用 Visual Studio**：
   - 启动 Visual Studio
   - 点击 "文件" > "打开" > "项目/解决方案"
   - 导航到 `src` 文件夹并选择 `ExplorerEnhancement.sln`

## 步骤 3: 构建项目

解决方案在 Visual Studio 中打开后：

1. **选择构建配置**：
   - 从配置下拉菜单中选择 "Debug" 或 "Release"
   - 选择平台（"x86" 或 "x64"）

2. **构建解决方案**：
   - 从菜单中点击 "构建" > "构建解决方案"
   - 或使用键盘快捷键 `Ctrl+Shift+B`

## 步骤 4: 构建安装程序（可选）

如果您想构建安装程序包：

1. 确保已安装 [NSIS (Nullsoft 脚本安装系统)](https://nsis.sourceforge.io/Download)
2. 在 NSIS 中打开 `Installer.nsi` 文件
3. 点击 "编译 NSIS 脚本" 构建安装程序

编译后的安装程序将命名为 `ExplorerExtensionSetup.exe`。

## 故障排除

- 如果遇到缺少依赖项的问题，请确保已安装所有必需的 Visual Studio 工作负载
- 如果构建时出现问题，请先尝试清理解决方案（"构建" > "清理解决方案"），然后再重新构建
- 确保使用兼容版本的 Visual Studio（2019 或更高版本）