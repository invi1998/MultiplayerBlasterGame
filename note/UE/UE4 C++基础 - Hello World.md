# UE4 C++基础 - Hello World

## 入门

学习UE4最好的方式就是编写代码实际调试。方便快捷的调试可以加快我们学习的进度。尽管这个过程很简单，但是对于初学者来说，它仍然可能成为一大障碍，因为要达到这个目的，我们必须能够正确部署UE4 C++环境，且成功编译运行，然后输出结果。掌握了这些操作细节以后，其他事情就比较容易了。

## 环境搭建

为了方便起见，我们建议大家使用Windows开发环境，当然Mac和Ubuntu环境也是可行的。引擎下载和配置可以参考下面这篇文章。有疑问的同学也可以在评论区留言，我们会视情况解决你遇到的问题。尽管各自的引擎和VS版本不同，但是大体步骤是相同的。

1. 安装 [Visual Studio](https://link.zhihu.com/?target=https%3A//visualstudio.microsoft.com/zh-hans/)
2. 安装 [Git](https://link.zhihu.com/?target=https%3A//git-scm.com/)
3. 注册Github，并加入 EPIC 开发组
4. Git拉取源码
5. Setup.bat 执行环境初始化
6. 生成新的C++工程
7. Viusal Studio打开工程

[Louis丶：UE4引擎 源码的获取、安装，以及VS配置53 赞同 · 4 评论文章![img](https://pic1.zhimg.com/v2-9046d9a6924ac98495af9cf40829dab0_180x120.jpg)](https://zhuanlan.zhihu.com/p/59536013)

## UE4，Hello World！

执行完上述步骤，如果一切顺利的话，你应该可以通过VS成功启动编辑器，并看到如下界面。Good Luck！

![img](https://pic1.zhimg.com/80/v2-e68d0b7ea6da2ff70820b739a5edd974_720w.webp)

图 1 成功配置环境并运行编辑器的样例

不过我们的起点不是这里，因为我们还有很多基础知识没有掌握。需要重新关闭编辑器，回到我们的Hello World，操作如下所示：

![img](https://pic4.zhimg.com/80/v2-0e60aec23d41f7a0f91ae7f5cc3a017b_720w.webp)

图 2 设置BlackProgram为启动项目

右键 解决方案 -》 Programs -》 BlankProgram 设置为启动项目，打开Private\BlankProgram.cpp，在

```cpp
return 0;
```

添加断点，并点击运行。

![img](https://pic2.zhimg.com/80/v2-cb187813856227a454e935db48fea449_720w.webp)

图 3 编译并启动 BlankProgram

![img](https://pic4.zhimg.com/80/v2-f827038fab7ae7a0abb99d16f569f56b_720w.webp)

图 4 Hello World

到这里，我们就可以开始接下来的步骤了，开始学习之前，我们先了解一下这段代码。

```cpp
// 包含了 CoreMinimal.h, 是对引擎常用模块的头文件引用
#include "BlankProgram.h" 

// BlankProgram是UE4最简单的独立程序，包含了运行独立应用程序最少的头文件
#include "RequiredProgramMainCPPInclude.h" 

// 定义LOG策略，用于UE_LOG宏
DEFINE_LOG_CATEGORY_STATIC(LogBlankProgram, Log, All);
// 定义Application，包含运行独立程序的最基本代码，暂时忽略
IMPLEMENT_APPLICATION(BlankProgram, "BlankProgram");

// Main入口点函数
INT32_MAIN_INT32_ARGC_TCHAR_ARGV()
{
        // 初始化引擎
	GEngineLoop.PreInit(ArgC, ArgV);
        // 输出Hello world, UE_LOG稍后解释
	UE_LOG(LogBlankProgram, Display, TEXT("Hello World"));
        // 返回0，表示程序运行成功
	return 0;
}
```

## UE_LOG

UE_LOG 是我们常用的输出内容至控制台的方式，原型如下：

```cpp
#define UE_LOG(CategoryName, Verbosity, Format, ...)
```

它包含以下几个参数，

- CategoryName 用于区分不同的Logger

- Verbosity 表示logging等级

- - **Fatal** 致命错误，无论该CategoryName禁用无否,都会输出至控制台和日志
  - **Error** 错误，输出至控制台和日志
  - **Warning** 警告，输出至控制台和日志
  - **Display** 显示日志，输出至控制台和日志
  - **Log** 调试信息，只输出至日志
  - **Verbose** 较繁琐的信息，通常是为了输出某些详细日志
  - **VeryVerbose** 更繁琐的信息

更多详细解释可以查看源码

```cpp
UnrealEngine\Engine\Source\Runtime\Core\Public\Logging\LogVerbosity.h
```

**Format**则与 [printf](https://link.zhihu.com/?target=https%3A//www.cplusplus.com/reference/cstdio/printf/) 函数保持一致。