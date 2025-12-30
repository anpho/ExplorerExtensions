[![MSBuild](https://github.com/anpho/WindowsExplorerExtension/actions/workflows/msbuild.yml/badge.svg)](https://github.com/anpho/WindowsExplorerExtension/actions/workflows/msbuild.yml)


# WindowsExplorerExtension
Windows资源管理器扩展，已在Windows 10和Windows 11上测试。

### 新建文件夹扩展

灵感来自GNOME Nautilus的功能，当您选择多个文件/文件夹然后右键单击，创建一个新文件夹时，所有选定的文件/文件夹将被移动到一个新文件夹中，该文件夹的名称基于每个文件文件名的共同部分。

**使用方法**
> 选择一些文件/文件夹，右键单击，选择 ```新建文件夹+```。

### 打破文件夹扩展

厌倦了文件夹套文件夹？此功能将打破文件夹结构并将所有文件移动到外部。

**使用方法**
> 选择一些文件夹，右键单击，选择 ```摊开文件夹```。




## 安装方法

### 选项1：使用安装程序（推荐）
前往 ```Releases``` 并下载安装程序文件 `ExplorerExtensionSetup.exe`。
1. 运行安装程序（无需管理员权限）
2. 按照屏幕上的说明操作
3. 如果菜单项未显示，请重新启动Windows资源管理器或您的计算机。

### 选项2：手动安装
前往 ```Releases``` 并下载文件ExplorerExtension.dll，打开终端，然后使用：
```bash
# 对于32位系统
regsvr32 ExplorerEnhancement_x86.dll

# 对于64位系统  
regsvr32 ExplorerEnhancement_x64.dll
```

使用 `regsvr32 /u` 注销DLL。

提示：手动安装方法也不需要管理员权限。


### 联系方式

support@anpho.cn

[Twitter](https://x.com/anpho)

## 许可证

本项目采用专有软件许可证。个人非商业使用是免费的，但**任何形式的商业使用都需要付费许可证**。如需商业许可咨询，请联系项目所有者。

有关专有许可证的完整详细信息，请参阅[LICENSE](LICENSE)文件；有关商业许可条款，请参阅[LICENSE_COMMERCIAL](LICENSE_COMMERCIAL.md)文件。