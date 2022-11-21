# UE4 C++基础 - 字符串和本地化

## 概览

UE4内建字符串有3种类型：

- **FString**[[1\]](https://zhuanlan.zhihu.com/p/163587790#ref_1)
- **FName**[[2\]](https://zhuanlan.zhihu.com/p/163587790#ref_2)
- **FText**[[3\]](https://zhuanlan.zhihu.com/p/163587790#ref_3)

最常见的是 **FString** 类型，**FName** 和 **FText** 属于不可变字符串，一经创建其中的内容就无法改变。**FString** 属于**可变**字符串，可以对其常规的字符串操作，如分片，大小写转换等操作，编程时使用较多的也是它。三者之间可以进行相互转换。

**FName** 不区分大小写，主要用于标识资源路径，它的优点就是查找和访问速度快，在游戏开发过程中，如果可以确定字符串是固定不变的数据，如资源路径，或者数据表格原始数据，且无需考虑文本国际化的时候，建议使用FName进行创建。

**FText** 主要用于文本本地化[[4\]](https://zhuanlan.zhihu.com/p/163587790#ref_4)。如今游戏出海已经成为一种趋势，当你的游戏需要支持不止一种语言时，就需要考虑文本本地化，建议最初时就作出这样设定。文本本地化是一项比较复杂的内容，会单独抽出章节讲解，现在需要需要记住的是，当字符串需要显示给玩家时，使用FText比较合适。

## 1. FString[[1\]](https://zhuanlan.zhihu.com/p/163587790#ref_1)

## 创建

> ***注意\*** ***大多数输出函数接收的是 TCHAR\* 类型的指针,因此需要使用 \****

```cpp
#include "GenericPlatform/GenericPlatformMisc.h"

FString TestFString = FString(TEXT("This is my test FString" LINE_TERMINATOR));
FGenericPlatformMisc::LocalPrint(*TestFString); // 输出: // This is my test FString.
```

## 比较

```cpp
FString StringA = FString(TEXT("This is my test FString.\n"));
FString StringB = FString(TEXT("This is my test FString.\n"));
// 直接使用 == 运算符
if (StringA == StringB) { }
// 等价于
if (StringA.Equals(StringB, ESearchCase::CaseSensitive)) { }
// 忽略大小写
if (StringA.Equals(StringB, ESearchCase::IgnoreCase)) { }
// 检测字符串是否以特定字符串开头
if (TestString.StartsWith(TEXT("This"))) { }
// 检测字符串是否以特定字符串结尾
if (TestString.EndsWith(TEXT("FString"))) { }
// 判断字符串是否为空
if (TestString.IsEmpty()) { }
```

## 查找

```cpp
// 查找某个字串是否存在有两种方式
// 1.  FString::Contains(),默认位忽略大小写，从字符串首查找至尾部
FString TestString = FString(TEXT("This is my test FString.\n"));
if (TestString.Contains(TEXT("This"), ESearchCase::IgnoreCase, ESearchDir::FromStart)) { }
// 等价于 2. FString.Find()
if (TestString.Find(TEXT("This"), ESearchCase::IgnoreCase, ESearchDir::FromStart, INDEX_NONE) != INDEX_NONE) { }
```

## 拼接

```cpp
// 直接使用 + 可以拼接2个字符串
FString A = FString(TEXT("A"));
FString B = FString(TEXT("B" LINE_TERMINATOR));
FString Plus = A + B;
FGenericPlatformMisc::LocalPrint(*Plus);

// 如果想在字符串尾部添加字符串，则使用 += 
A += FString(TEXT("C" LINE_TERMINATOR));
FGenericPlatformMisc::LocalPrint(*A);

// 更常见的是使用 FString::Printf() 函数
FString FormatString = FString::Printf(TEXT("FormatString:\n%s%s%s"), *A, *B, *Plus);
FGenericPlatformMisc::LocalPrint(*FormatString);

// 输出
// AB
// AC
// FormatString:
// AC
// B
// AB
```

## 其他常用方法

```cpp
// 逆序
FString MyString = FString(TEXT("123456"));
// 逆序并返回字符串副本
FString ReverseString = MyString.Reverse(); // 654321
// 直接逆序原始字符串
MyString.ReverseString();

// 字符串替换
FString SrcString = FString(TEXT("Apple Orange"));
// 返回替换后的字符串副本
FString ReplaceString = SrcString.Replace(TEXT("Apple"), TEXT("Coffee")); // Coffe Orange
// 直接替换原始字符串
SrcString.ReplaceInline(TEXT("Orange"), TEXT("Banana")); // Apple Banana

// 去除无用字符串
FString QuoteAndSpaceString = FString(TEXT("	\"StringWithEmpty\"  "));
// 返回剔除行首行末空白字符的字符串副本
FString WithoutSpace = QuoteAndSpaceString.TrimStartAndEnd(); // "StringWithEmpty"
// 直接剔除原始字符串的行首行末的空白字符
QuoteAndSpaceString.TrimStartAndEndInline(); // "StringWithEmpty"
// 返回剔除行首行末引号的字符串副本
FString WithoutQuotesString = QuoteAndSpaceString.TrimQuotes(); //StringWithEmpty
// 清空字符串
QuoteAndSpaceString.Empty();
```

## 字符串分片

将字符串按下标拆解为我们需要的字符串称之为***分片(Silce)，\***常用分片函数包含如下几个***：\***

- **FString::Mid**[[5\]](https://zhuanlan.zhihu.com/p/163587790#ref_5)
- **FString::Left**[[6\]](https://zhuanlan.zhihu.com/p/163587790#ref_6)
- **FString::LeftChop**[[7\]](https://zhuanlan.zhihu.com/p/163587790#ref_7)
- **FString::Right**[[8\]](https://zhuanlan.zhihu.com/p/163587790#ref_8)
- **FString::RightChop**[[9\]](https://zhuanlan.zhihu.com/p/163587790#ref_9)

5个函数都接受一个 **Count** 参数，表明要操作的字符串长度，这个长度如果超过字符串本身长度，就会被默认处理为字符串本身长度。其中Left,LeftChop和Right,RightChop可以看做Mid的一种简便写法，具体样例如下：

```cpp
// 字符串分片
FString Str = FString(TEXT("ABCDEFGHIJ"));
// 字符串长度
int32 StringLength = Str.Len();
// 通过下标获取字符
TCHAR Char = Str[0]; // A
// 从左往右截取指定长度的字符串
FString LeftStr = Str.Left(4); // ABCD
// 去除字符串末尾指定的长度字符串
FString LeftChopStr = Str.LeftChop(4); // ABCDEF
// 从右往左截取指定长度的字符串
FString RightStr = Str.Right(4); // GHIJ
// 去除字符串开头指定长度的字符串
FString RightChopStr = Str.RightChop(4); // EFGHIJ
// 从字符串指定位置获取指定长度的字符串
FString MidStr = Str.Mid(2,6);// CDEFGH
```

LeftChop 和 RightChop 这2个函数名歧义较大，硬记即可。如下图所示

![img](https://pic4.zhimg.com/80/v2-48f177aa23a9db44bd73fc364f41d157_720w.webp)

图 1-1 字符串切片

此外还有 ParseIntoArray[[10\]](https://zhuanlan.zhihu.com/p/163587790#ref_10) 函数用于将字符串按分隔符进行拆分

```cpp
// 按指定分割符号进行拆分
FString SplitStr = FString(TEXT("ABC,DEF,GHI,"));
// 保存拆分后的字符串数组
TArray<FString> SplitedStr;
// 最后一个参数 InCullEmtpy 表示是否剔除空字符串
SplitStr.ParseIntoArray(SplitedStr, TEXT(","), false); // ABC DEF GHI Empty
SplitStr.ParseIntoArray(SplitedStr, TEXT(","), true); // ABC DEF GHI
```

## 2. FName[[2\]](https://zhuanlan.zhihu.com/p/163587790#ref_2)

通常用于标识资源路径或者其他几乎不变的字符串，如资源文件类型，或者平台标识等。FName主要是为了性能上的优势。运行时，为了性能最优，应该尽可能的使用FName，只在必要的时候才将FName转换为其他字符串类型。引擎的UObject的就是使用的FName来储存对象名称。

```cpp
// K:\UnrealEngine\Engine\Source\Runtime\CoreUObject\Public\UObject\UObjectBase.h
/** Name of this object */
FName NamePrivate;
```

> 注意：
> FName是不区分大小写的，FString转换至FName时会丢失原始字符串的大小写信息

```cpp
// 创建FName
FName TestFName = FName(TEXT("ThisIsMyTestFName"));
FName TestFNameFromFString = FName(*FString(TEXT("FString")));
// 获取字符串长度
int32 FNameLength = TestFName.GetStringLength(); // 17
// 转换为 FString
FString FStringFromFName = TestFName.ToString();
// 测试字符串是否相等,注意比较是忽略大小写的
if (TestFName == FName(TEXT("thisismytestfname"))) { }
if (TestFName.IsEqual(FName(TEXT("THISISMYTESTFNAME")))) { }
```

## 3. FText[[3\]](https://zhuanlan.zhihu.com/p/163587790#ref_3)

主要用于支持**文本本地化**，本地化[[11\]](https://zhuanlan.zhihu.com/p/163587790#ref_11)是一个较复杂流程，我们这里只在源码层面[[12\]](https://zhuanlan.zhihu.com/p/163587790#ref_12)介绍如何编写可以本地化的代码，其他部分可以参考UE4官方文档。要让我们的C++源码支持本地化，在使用字符串时，必须对所有需要翻译的源码字符串用如下2个宏中的一个进行包裹：

- **NSLOCTEXT**
- **LOCTEXT**

NSLOCTEXT包含3个参数:

- **namespace 命名空间**
- **key 上下文**
- **source string 源文**

一个工程中可以存在多个命名空间，用于区分翻译的不同用途。例如我们可以简单的将要翻译的源码区分为引擎和项目2个命名空间。上下文用于相同的源文在不同的语义下需要翻译成不同的场景。源文则是我们要翻译的原始文本。样例如下：

```cpp
FText constFTextHelloWorld = NSLOCTEXT("MyOtherNamespace","HelloWorld","Hello World!");
```

**LOCTEXT** 可以看做 **NSLOCTEXT** 的一种简便写法，使用 **LOCTEXT** 必须在源文件头定义 **LOCTEXT_NAMESPACE** 宏：

```cpp
// 定义 LOCTEXT 命令空间
// 这个宏只在该源文件中生效，且必须在源文件结尾处取消定义
#define LOCTEXT_NAMESPACE "MyNamespace"
// Create text literals
FText constFTextGoodbyeWorld= LOCTEXT("GoodbyeWorld","Goodbye World!");
// Undefine the namespace before the end of the file
#undef LOCTEXT_NAMESPACE // 注意：必须取消宏定义
```

更多样例可以查看引擎单元测试源码

```cpp
UnrealEngine\Engine\Source\Runtime\Core\Private\Tests\Internationalization\TextTest.cpp
```

## 4. 字符串类型的转换

三种字符串类型之间可以根据需要进行转换，值得注意的是FText并不能直接转换为FName,需要转换为FString之后再转换为FText。FString转换至FName时会丢失原始字符串的大小写信息。FText转换为FString会丢失本地化信息。

```cpp
// 字符串之间的转换
FString SrcStr = FString(TEXT("SrcStr"));
FText SrcText = FText::FromString(*FString(TEXT("SrcText")));
FName SrcName = FName(TEXT("SrcName"));
// FString -> FName
FName NameStr = FName(*SrcStr);
// FString -> FText
FText TextFromString = FText::FromString(SrcStr);
// FName -> FText
FText TextFromName = FText::FromName(NameStr);
// FName -> FString
FString StringFromName = SrcName.ToString();
// FText -> FString
FString StringFromText = SrcText.ToString();
```

三者之间的转换方式如下图：

![img](https://pic1.zhimg.com/80/v2-069231f3f58e2feccecd2c103a6e98d0_720w.webp)

图 1-2 三者字符串类型之间的转换

在使用UE4的过程中，我们不可避免的会与第三方库打交道，这个时候就需要在内建字符串类型与std::string之间进行转换，样例如下：

```cpp
#include <string>
// std::string -> FString
std::string StdString = "ExampleStdString";
FString FStringFromStdString(StdString.c_str());

// FString -> std::string
FString ExampleString = TEXT("FString");
std::string StdStringFromString_ANSI = TCHAR_TO_ANSI(*ExampleString);
std::string StdStringFromString_UTF8 = TCHAR_TO_UTF8(*ExampleString);
```

## 5. UE4正则表达式（待补充）

## 参考

1. ^[a](https://zhuanlan.zhihu.com/p/163587790#ref_1_0)[b](https://zhuanlan.zhihu.com/p/163587790#ref_1_1)FString https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/StringHandling/FString/index.html
2. ^[a](https://zhuanlan.zhihu.com/p/163587790#ref_2_0)[b](https://zhuanlan.zhihu.com/p/163587790#ref_2_1)FName https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/StringHandling/FName/index.html
3. ^[a](https://zhuanlan.zhihu.com/p/163587790#ref_3_0)[b](https://zhuanlan.zhihu.com/p/163587790#ref_3_1)FText https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/StringHandling/FText/index.html
4. [^](https://zhuanlan.zhihu.com/p/163587790#ref_4_0)Text Localization https://docs.unrealengine.com/en-US/Gameplay/Localization/Formatting/index.html
5. [^](https://zhuanlan.zhihu.com/p/163587790#ref_5_0)FString::Mid https://docs.unrealengine.com/en-US/API/Runtime/Core/Containers/FString/Mid/index.html
6. [^](https://zhuanlan.zhihu.com/p/163587790#ref_6_0)FString::Left https://docs.unrealengine.com/en-US/API/Runtime/Core/Containers/FString/Left/index.html
7. [^](https://zhuanlan.zhihu.com/p/163587790#ref_7_0)FString::LeftChop https://docs.unrealengine.com/en-US/API/Runtime/Core/Containers/FString/LeftChop/index.html
8. [^](https://zhuanlan.zhihu.com/p/163587790#ref_8_0)FString::Right https://docs.unrealengine.com/en-US/API/Runtime/Core/Containers/FString/Right/index.html
9. [^](https://zhuanlan.zhihu.com/p/163587790#ref_9_0)FString::RightChop https://docs.unrealengine.com/en-US/API/Runtime/Core/Containers/FString/RightChop/index.html
10. [^](https://zhuanlan.zhihu.com/p/163587790#ref_10_0)FString::ParseIntoArray https://docs.unrealengine.com/en-US/API/Runtime/Core/Containers/FString/ParseIntoArray/1/index.html
11. [^](https://zhuanlan.zhihu.com/p/163587790#ref_11_0)本地化 https://docs.unrealengine.com/zh-CN/Gameplay/Localization/Overview/index.html
12. [^](https://zhuanlan.zhihu.com/p/163587790#ref_12_0)源码本地化 https://docs.unrealengine.com/zh-CN/Gameplay/Localization/Formatting/index.html