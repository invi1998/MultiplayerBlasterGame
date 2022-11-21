# UE4 C++基础 - 工程目录结构

## 简介

介绍UE4工程目录结构[[1\]](https://zhuanlan.zhihu.com/p/160917246#ref_1)以及常用的目录操作接口[[2\]](https://zhuanlan.zhihu.com/p/160917246#ref_2)。UE4将目录分为：

- **引擎目录**
- **项目目录**

其中一些子目录是两个都有的，称之为**通用目录**，有一些目录是引擎特有的。当我们从github

```text
https://github.com/EpicGames/UnrealEngine.git
```

上检出工程到本地时，起始目录叫做根目录。此外，开发过程中，为了方便起见，资源之间的引用是通过**沙盒路径**（虚拟路径）来进行标识的。

## 根目录

![img](https://pic4.zhimg.com/80/v2-966f91451f1a6f410fdc1aede753298b_720w.webp)

图 1-1 根目录

根目录说明如下：

- **Engine** 引擎目录，包含构成引擎的所有源代码、内容等。

- **Samples** 样例资源。

- **Templates** 样例工程模块，[创建新项目](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/zh-CN/Engine/Basics/Projects/Browser/index.html)时可用的项目模板集合。

- **Setup.bat** 只需运行一次。

- - 拉取引擎的三方依赖
  - 安装引擎运行环境 - 执行Engine\Extras\Redist\en-us\UE4PrereqSetup_x64.exe
  - 引擎注册 - 执行UnrealVersionSelector.exe，让uproject可以被windows识别。

- **GenerateProjectFiles.bat** 用于生成引擎vs工程文件。

Setup 和 GenerateProjectFiles还有对应的 .sh 文件，作用与.bat相同，用于在Mac平台下是生成从xcode工程文件。.command是Mac平台下的一种快捷方式，可以支持双击打开，而不在命令行模式执行。

## 通用目录

一些子目录是在引擎和游戏项目目录之间通用的：

- **Binaries -** 包含可执行文件或编译期间创建的其他文件。

- **Build** - 包含构建引擎或游戏所需的文件，包括创建特定于平台的构建版所需的文件。

- **Config** - 配置文件，用于设置用来控制引擎行为的值。项目Config文件中设置的值会覆盖 Engine\Config 目录中设置的值。

- **Content** - 保存引擎或游戏的内容，包括资源包和贴图。

- **DerivedDataCache** - 包含加载时针对引用内容生成的派生数据文件。引用内容没有相应的缓存文件会导致加载时间显著延长。

- **Intermediate** - 包含构建引擎或游戏时生成的临时文件。在游戏目录中，着色器存储在Intermediate目录中。

- **Saved** - 包含自动保存、配置（.ini）文件和日志文件。此外，Engine\Saved目录还包含崩溃日志、硬件信息和Swarm选项与数据。

- **Source** - 包含引擎或游戏的所有源文件，包括引擎源代码、工具和游戏类等。

- - **Engine** - Engine目录中的源文件组织结构如下：

  - - **Developer** - 编辑器和引擎共同使用的文件。
    - **Editor** - 仅供编辑器使用的文件。
    - **Programs** - 引擎或编辑器使用的外部工具。
    - **Runtime** - 仅供引擎使用的文件。

  - **Game** - 游戏项目源码，建议按模块的方式进行组织。

## 源码模块组织方式

游戏项目目录中的源文件按模块分组，一个模块一个目录。每个模块包含以下内容：

- **Classes** - 包含所有项目头（.h）文件。
- **Private** - 包含所有 `.cpp` 文件，包括游戏类实现文件和模块实现文件。
- **Public** - 包含模块标头文件。

## 引擎Source模块

![img](https://pic1.zhimg.com/80/v2-3e917bff26957763ac5fe5e5d6c97d18_720w.webp)

图 1-2 Engine\Source

引擎Source目录下分为以下目录：

- **Developer** - Editor&Programs会使用，但不能是Games
- **Editor** - 只被Editor使用
- **Programs** - 独立的Applitcations或者工具类
- **Runtime** - Editor,Games,Programs都会使用
- **ThirdParty** - 第三方库

## 模块依赖原则

- **Runtime** 模块不能依赖 **Editor**或者**Developer**中的模块
- **Plug-in** 模块不能依赖其他Plug-ins

![img](https://pic2.zhimg.com/80/v2-12ebbf153a8a5f0cb20702a0b1462c75_720w.webp)

不同类型应用程序的模块使用样例

## 基础模块（新手）

- **Core** - Fundamental core types & functions 基础数据类型和函数
- **CoreUObject** - UObject实现
- **Engine** - Game类以及引擎核心框架
- **OnlineSubsystem** - Online & social networking features
- **Slate** - Widget library & high-level UI features

## 其他几个有趣模块（进阶）

- **DesktopPlatform** – Useful APIs for Windows, Mac & Linux
- **DetailCustomizations** – Editor’s Details panel customizations
- **Launch** – Main loop classes & functions
- **Messaging** – Message passing sub-system
- **Sockets** – Network socket implementations
- **Settings** – Editor & Project Settings API
- **SlateCore** – Fundamental UI functionality
- **TargetPlatform** – Platform abstraction layer
- **UMG** – Unreal Motion Graphics implementation
- **UnrealEd** – Unreal Editor main frame & features
- **Analytics** – Collects usage statistics from Editor & games
- **AssetRegistry** – Database for assets in Unreal Editor
- **GameLiveStreaming** – Twitch Streaming
- **HeadMountedDisplay** – HMD Support API (Oculus, etc.)
- **JsonUtilities & XmlParser** – Handle Json & XML files
- **SourceControl** – API for custom source control providers

## 特定于引擎的目录

![img](https://pic2.zhimg.com/80/v2-f91d4f45c350cb05db5534784611465d_720w.webp)

图 1-3 Engine引擎目录

部分子目录特定于Engine目录。

- **Documentation** - 包含引擎文档，包括源文件和发布的文件。

- - **HTML** - 发布的HTML文档文件。
  - **Source** - 源markdown文档文件。

- **Extras** - 其他帮助和实用程序文件。

- **Plugins** - 包含引擎中使用的插件。

- **Programs** - 包含UE4根目录中存储的项目以及其他虚幻程序（如UnrealFrontend和UnrealHeaderTool）的配置文件和日志文件。

- **Shaders** - 保存引擎的着色器源文件（`.usf`）。

## 游戏项目目录

![img](https://pic1.zhimg.com/80/v2-b1bde76edc9dcd2e4b743f2fea4c9f90_720w.webp)

图 1-4 游戏项目目录

- **Binaries** - 包含可执行文件或编译期间创建的其他文件。
- **Config** - 游戏的默认项目设置。
- **Content** - 包含引擎或游戏的内容，包括资源包和贴图。
- **Plugins** - 包含项目中使用的插件。
- **Intermediate** - 包含UnrealBuildTool生成的文件，如Visual Studio项目文件。这些文件可以删除并重新构建。
- **Saved** - 包含引擎生成的文件，如配置文件和日志。这些文件可以删除并重新构建。
- **Source** - 包含游戏模块对象类文件。
- **EasyUE4.sln** 项目vs工程文件。
- **EasyUE4.uproject** 项目文件。
- **EasyUE4.vs.db** vs分析源码后的数据缓存，可以理解为ctags文件。

## Plugin插件目录

![img](https://pic4.zhimg.com/80/v2-0b129348ca6fdd94a747ff1f0314d5b7_720w.webp)

图 1-5 Plugin插件目录

- **Binaries** - 包含可执行文件或编译期间创建的其他文件。
- **Content** - 包含游戏内用到的插件资源目录。
- **Intermediate** - 包含UnrealBuildTool生成的文件，如Visual Studio项目文件。这些文件可以删除并重新构建。
- **Resources** - 插件额外资源目录，如插件图标。
- **ThirdParty** - 插件依赖的三方库目录。
- **ScriptPlugin.uplugin** 插件文件，包含这个文件的目录会被UE4识别为一个插件目录。

## 沙盒路径

当我们在编辑器状态下浏览资源时，显示的资源路径，通常是**沙盒路径**

![img](https://pic3.zhimg.com/80/v2-fc389106d1de92654feda11a53436fea_720w.webp)

图 1-6 沙盒路径

```text
/Game/ThirdPerson/Meshes/LeftArm_StaticMesh
/Engine/Maps/Entry
```

其中 **/Game** 是一个虚拟路径，实际表示的是项目的 **FPaths::ProjectContentDir() 。/Engine** 也是一个虚拟路径，实际路径是引擎的 **FPaths::EngineContentDir()**。更多虚拟路径可以查阅源码[[3\]](https://zhuanlan.zhihu.com/p/160917246#ref_3)

```text
K:\UnrealEngine\Engine\Source\Runtime\CoreUObject\Private\Misc\PackageName.cpp
```

类 FLongPackagePathsSingleton 的定义。

内容浏览器视图选项勾选显示引擎内容操作如下所示：

![img](https://pic3.zhimg.com/80/v2-c11c2b16bafe834fc834f1e06f9c5cd2_720w.webp)

图 1-7 内容浏览器显示引擎内容

![img](https://pic2.zhimg.com/80/v2-c75ba764f4ab86a0f1bac1b9add89e09_720w.webp)

图 1-8 内容浏览器切换至引擎目录

## 路径获取和操作API

```cpp
#include "BlankProgram.h"

#include "RequiredProgramMainCPPInclude.h"
#include "Misc/Paths.h"

DEFINE_LOG_CATEGORY_STATIC(LogBlankProgram, Log, All);
IMPLEMENT_APPLICATION(BlankProgram, "BlankProgram");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
GEngineLoop.PreInit(ArgC, ArgV);
UE_LOG(LogBlankProgram, Display, TEXT("Hello World"));

// 常用路径获取接口
UE_LOG(LogBlankProgram, Display, TEXT("EngineDir: %s"), *FPaths::EngineDir());
UE_LOG(LogBlankProgram, Display, TEXT("EngineSavedDir: %s"), *FPaths::EngineSavedDir());
UE_LOG(LogBlankProgram, Display, TEXT("EngineIntermediateDir: %s"), *FPaths::EngineIntermediateDir());
UE_LOG(LogBlankProgram, Display, TEXT("ProjectDir: %s"), *FPaths::ProjectDir());
UE_LOG(LogBlankProgram, Display, TEXT("ProjectContentDir: %s"), *FPaths::ProjectContentDir());
UE_LOG(LogBlankProgram, Display, TEXT("ProjectConfigDir: %s"), *FPaths::ProjectConfigDir());
UE_LOG(LogBlankProgram, Display, TEXT("ProjectSavedDir: %s"), *FPaths::ProjectSavedDir());
UE_LOG(LogBlankProgram, Display, TEXT("ProjectIntermediateDir: %s"), *FPaths::ProjectIntermediateDir());

FString TestFilename(TEXT("ParentDirectory/Directory/FileName.extion"));
FString Extension = FPaths::GetExtension(TestFilename);
FString BaseFilename = FPaths::GetBaseFilename(TestFilename);
FString CleanFilename = FPaths::GetCleanFilename(TestFilename);
FString Directory = FPaths::GetPath(TestFilename);
bool bFileExists = FPaths::FileExists(TestFilename);
bool bDirectoryExists = FPaths::DirectoryExists(Directory);

UE_LOG(LogBlankProgram, Display, TEXT("TestFilename: %s"), *TestFilename);
// 获取文件扩展名
UE_LOG(LogBlankProgram, Display, TEXT("Extension: %s"), *Extension);
// 获取文件名，不带扩展名
UE_LOG(LogBlankProgram, Display, TEXT("BaseFilename: %s"), *BaseFilename);
// 获取文件名，带扩展名
UE_LOG(LogBlankProgram, Display, TEXT("CleanFilename: %s"), *CleanFilename);
// 获取路径文件夹，去除CleanFilename后的路径
UE_LOG(LogBlankProgram, Display, TEXT("Directory: %s"), *Directory);
// 检测文件是否存在
UE_LOG(LogBlankProgram, Display, TEXT("FileExists: %s"), bFileExists ? TEXT("True") : TEXT("False"));
// 检测文件夹是否存在
UE_LOG(LogBlankProgram, Display, TEXT("DirectoryExists: %s"), bDirectoryExists ? TEXT("True") : TEXT("False"));

// 路径拼接
FString NewFilePath = FPaths::Combine(Directory, TEXT("NewFilePath.txt"));
// 简便写法
FString NewFilePathEasy = Directory / TEXT("NewFilePath.txt");
// 相对路径转换为绝对路径
FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::EngineDir());
UE_LOG(LogBlankProgram, Display, TEXT("NewFilePath: %s"), *NewFilePath);
UE_LOG(LogBlankProgram, Display, TEXT("NewFilePathEasy: %s"), *NewFilePathEasy);
UE_LOG(LogBlankProgram, Display, TEXT("FullPath: %s"), *FullPath);

return 0;
}
```

输出

```cpp
LogBlankProgram: Display: Hello World
LogBlankProgram: Display: EngineDir: ../../../Engine/
LogBlankProgram: Display: EngineSavedDir: ../../../Engine/Saved/
LogBlankProgram: Display: EngineIntermediateDir: ../../../Engine/Intermediate/
LogBlankProgram: Display: ProjectDir: ../../../Engine/Programs/BlankProgram/
LogBlankProgram: Display: ProjectContentDir: ../../../Engine/Programs/BlankProgram/Content/
LogBlankProgram: Display: ProjectConfigDir: ../../../Engine/Programs/BlankProgram/Config/
LogBlankProgram: Display: ProjectSavedDir: ../../../Engine/Programs/BlankProgram/Saved/
LogBlankProgram: Display: ProjectIntermediateDir: ../../../Engine/Programs/BlankProgram/Intermediate/
LogBlankProgram: Display: TestFilename: ParentDirectory/Directory/FileName.extion
LogBlankProgram: Display: Extension: extion
LogBlankProgram: Display: BaseFilename: FileName
LogBlankProgram: Display: CleanFilename: FileName.extion
LogBlankProgram: Display: Directory: ParentDirectory/Directory
LogBlankProgram: Display: FileExists: False
LogBlankProgram: Display: DirectoryExists: False
LogBlankProgram: Display: NewFilePath: ParentDirectory/Directory/NewFilePath.txt
LogBlankProgram: Display: NewFilePathEasy: ParentDirectory/Directory/NewFilePath.txt
LogBlankProgram: Display: FullPath: K:/UnrealEngine/Engine/
```



## 参考

1. [^](https://zhuanlan.zhihu.com/p/160917246#ref_1_0)目录结构 https://docs.unrealengine.com/zh-CN/Engine/Basics/DirectoryStructure/index.html
2. [^](https://zhuanlan.zhihu.com/p/160917246#ref_2_0)FPaths https://docs.unrealengine.com/en-US/API/Runtime/Core/Misc/FPaths/index.html
3. [^](https://zhuanlan.zhihu.com/p/160917246#ref_3_0)FPackageName https://docs.unrealengine.com/en-US/API/Runtime/CoreUObject/Misc/FPackageName/index.html