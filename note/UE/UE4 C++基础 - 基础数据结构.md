# UE4 C++基础 - 基础数据结构

## 简介

UE4是跨平台的引擎，为了尽可能减少跨平台数据大小不一致导致的问题，所以自定义了基本数据类型。使用 **typedef** 将其定义为我们常用的数据类型，并通过编译时检测[[1\]](https://zhuanlan.zhihu.com/p/137638251#ref_1)保证数据大小在各平台的一致性。还提供了常用的聚合数据类型，如数组，集合，哈希表用于满足业务侧开发需求。

## 1. 基本数据类型

```cpp
// 无符号基本数据类型
uint8 // 8位无符号整数
uint16 // 16位无符号整数
uint32 // 32位无符号整数
uint64 // 64位无符号整数

//有符号基本数据类型
int8 // 8位有符号整数
int16 // 16位有符号整数
int32 // 32位有符号整数
int64 // 64位有符号整数

// 字符类型
ANSICHAR // ANSI 字符. 通常是带符号类型
WIDECHAR // 宽字符. 通常是带符号类型
TCHAR // ANSICHAR 或者 WIDECHAR,根据平台是否支持宽字符或者licensee授权决定
UTF8CHAR // UTF-8 字符
UCS2CHAR // UCS2 字符
UTF16CHAR // UTF-16 字符
UTF32CHAR // UTF-32 字符

// 指针整数类型
UPTRINT // An unsigned integer the same size as a pointer
PTRINT // A signed integer the same size as a pointer
SIZE_T // An unsigned integer the same size as a pointer, the same as UPTRINT
SSIZE_T // An integer the same size as a pointer, the same as PTRINT

// 常量类型
TYPE_OF_NULL // NULL 常量
TYPE_OF_NULLPTR // C++ nullptr
```

## 2. 字符串类型

[篮子悠悠：UE4 C++基础教程 - 字符串和本地化48 赞同 · 3 评论文章![img](https://pic4.zhimg.com/v2-15e856c31120ed114ade5d26431b7657_180x120.jpg)](https://zhuanlan.zhihu.com/p/163587790)

## 3. 聚合数据类型

单靠基本的数据类型往往无法满足游戏业务侧开发的需求。假设我们要开发一个任务系统，任务是由一系列的子任务组成，每个任务都有各自的名称和子任务,我们如何储存这些任务信息呢？

为了解决类似的问题，我们需要一种结构化的数据结构，因此引入了聚合数据类型，UE4中称之为***容器**(Container)*，来组织我们的数据。就如同我们的衣柜，用来有序的储存我们各种各样的衣物。

## 3.1 TArray（动态数组）[[2\]](https://zhuanlan.zhihu.com/p/137638251#ref_2)

TArray是几个容器类中使用最频繁的一个，既可以作为列表，也可以作为集合，还可以作为栈来使用。其中储存的元素必须是同一个类型。

## 3.1.1 TArray创建

TArray是一个模板类，接受2个参数，分别是元素类型和分配器。

```cpp
template<typename InElementType, typename InAllocator>
class TArray
```

其中 InElementType 指明存储元素的类型， InAllocator 标识其所采用的内存分配器（TSizedHeapAllocator）。默认元素使用的内存是分配在堆上，而不是栈上。但是使用时不用关心这一点，将TArray当作普通的数值类型（如int32, float）处理即可。TArray销毁时其中的元素也会被销毁。

**初始化**

```cpp
// 普通创建
TArray<int32> IntArray;
// 
```



## TSet （集合）[[3\]](https://zhuanlan.zhihu.com/p/137638251#ref_3)

## TMap（字典或哈希表）[[4\]](https://zhuanlan.zhihu.com/p/137638251#ref_4)



## 参考

1. [^](https://zhuanlan.zhihu.com/p/137638251#ref_1_0)static_assert https://zh.cppreference.com/w/cpp/language/static_assert
2. [^](https://zhuanlan.zhihu.com/p/137638251#ref_2_0)TArray https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/TArrays/index.html
3. [^](https://zhuanlan.zhihu.com/p/137638251#ref_3_0)TSet https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/TSet/index.html
4. [^](https://zhuanlan.zhihu.com/p/137638251#ref_4_0)TMap https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/TMap/index.html