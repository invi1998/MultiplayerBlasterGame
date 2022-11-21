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

TSet是一种快速容器类，（通常）用于在排序不重要的情况下存储唯一元素。

`TSet` 类似于 `TMap` 和 `TMultiMap`，但有一个重要区别：`TSet` 是通过对元素求值的可覆盖函数，使用数据值本身作为键，而不是将数据值与独立的键相关联。`TSet` 可以非常快速地添加、查找和删除元素（恒定时间）。默认情况下，`TSet` 不支持重复的键，但使用模板参数可激活此行为。

### TSet

`TSet` 是一种快速容器类，用于在排序不重要的情况下存储唯一元素。在大多数情况下，只需要一种参数——元素类型。但是，`TSet` 可配置各种模板参数来改变其行为，使其更全面。除了可指定从 `DefaultKeyFuncs` 的派生结构体来提供散列功能，还可允许集合中的多个键拥有相同的值。它和其它容器类一样，可设置自定义内存分配器来存储数据。

和 `TArray` 一样，`TSet` 是同质容器。换而言之，其所有元素均完全为相同类型。`TSet` 也是值类型，支持常规复制、赋值和析构函数操作，以及其元素较强的所有权。`TSet` 被销毁时，其元素也将被销毁。键类型也必须是值类型。

`TSet` 使用散列，即如果给出了 `KeyFuncs` 模板参数，该参数会告知集合如何从某个元素确定键，如何比较两个键是否相等，如何对键进行散列，以及是否允许重复键。它们默认只返回对键的引用，使用 `运算符==` 对比相等性，使用非成员函数 `GetTypeHash` 进行散列。默认情况下，集合中不允许有重复的键。如果您的键类型支持这些函数，则可以将其用作集合键，无需提供自定义 `KeyFuncs`。要写入自定义 `KeyFuncs`，可扩展 `DefaultKeyFuncs` 结构体。

最后，`TSet` 可通过任选分配器控制内存分配行为。标准虚幻引擎4（UE4）分配器（如 `FHeapAllocator` 和 `TInlineAllocator`）不能用作 `TSet` 的分配器。实际上，`TSet` 使用集合分配器，该分配器可定义集合中使用的散列桶数量以及用于存储元素的标准UE4分配器。请参见 `TSetAllocator` 了解更多信息。

与 `TArray` 不同的是，内存中 `TSet` 元素的相对排序既不可靠也不稳定，对这些元素进行迭代很可能会使它们返回的顺序和它们添加的顺序有所不同。这些元素也不太可能在内存中连续排列。集合中的后台数据结构是稀疏数组，即在数组中有空位。从集合中移除元素时，稀疏数组中会出现空位。将新的元素添加到阵列可填补这些空位。但是，即便 `TSet` 不会打乱元素来填补空位，指向集元素的指针仍然可能失效，因为如果存储器被填满，又添加了新的元素，整个存储可能会重新分配。

### 创建和填充集合

`TSet` 的创建方法如下：

```c++
TSet<FString> FruitSet;
```

这会创建一个空白 `TSet`，用于存储 `FString` 数据。`TSet` 会直接使用 `运算符==` 比较元素，使用 `GetTypeHash` 对其进行散列，然后使用标准的堆分配器。此时尚未分配内存。

填充集合的标准方法是使用 `Add` 函数并提供键（元素）：

```c++
FruitSet.Add(TEXT("Banana"));
FruitSet.Add(TEXT("Grapefruit"));
FruitSet.Add(TEXT("Pineapple"));
// FruitSet == [ "Banana", "Grapefruit", "Pineapple" ]
```



此处的元素按插入顺序排列，但不保证这些元素在内存中实际保留此排序。如果是新集合，可能会保留插入排序，但插入和删除的次数越多，新元素不出现在末尾的可能性越大。



由于此集合使用了默认分配器，可以确保键是唯一的。如果尝试添加重复键，会发生以下情况：

```c++
FruitSet.Add(TEXT("Pear"));
FruitSet.Add(TEXT("Banana"));
// FruitSet == [ "Banana", "Grapefruit", "Pineapple", "Pear" ]
// Note:Only one banana entry.
```

该集合现在包含4个元素。"Pear"将数量从3增至4，但新的"Banana"没有改变集合中的元素数量，因为它替代了旧的"Banana"条目。

和 `TArray` 一样，还可使用 `Emplace` 代替 `Add`，避免插入集合时创建临时文件：

```c++
FruitSet.Emplace(TEXT("Orange"));
// FruitSet == [ "Banana", "Grapefruit", "Pineapple", "Pear", "Orange" ]
```

此处，参数直接传递给键类型的构造函数。这可以避免为该值创建临时 `FString`。与 `TArray` 不同的是，只能使用单一参数构造函数将元素放到集合中。

也可使用 `Append` 函数进行合并来插入另一个集合中的所有元素：

```c++
TSet<FString> FruitSet2;
FruitSet2.Emplace(TEXT("Kiwi"));
FruitSet2.Emplace(TEXT("Melon"));
FruitSet2.Emplace(TEXT("Mango"));
FruitSet2.Emplace(TEXT("Orange"));
FruitSet.Append(FruitSet2);
// FruitSet == [ "Banana", "Grapefruit", "Pineapple", "Pear", "Orange", "Kiwi", "Melon", "Mango" ]
```

在上述示例中，生成的集合和使用 `Add` 或 `Emplace` 进行单个添加是相同的。源集合中的重复键将会替代目标集合中相应的键。

### 编辑UPROPERTY TSet

如果用 `UPROPERTY` 宏和一个可编辑的关键词（`EditAnywhere`、`EditDefaultsOnly` 或 `EditInstanceOnly`）标记 `TSet`，则可[在虚幻编辑器中添加和编辑元素](https://docs.unrealengine.com/5.1/zh-CN/level-editor-details-panel-in-unreal-engine)。

```c++
UPROPERTY(Category = SetExample, EditAnywhere)
TSet<FString> FruitSet;
```

### 迭代

`TSet` 的迭代类似于 `TArray`。可使用C++的设置范围功能：

```c++
for (auto& Elem :FruitSet)
{
    FPlatformMisc::LocalPrint(
        *FString::Printf(
            TEXT(" \"%s\"\n"),
            *Elem
        )
    );
}
// Output:
//  "Banana"
//  "Grapefruit"
//  "Pineapple"
//  "Pear"
//  "Orange"
//  "Kiwi"
//  "Melon"
//  "Mango"
```

也可以用 `CreateIterator` 和 `CreateConstIterators` 函数来创建迭代器。`CreateIterator` 返回拥有读写访问权限的迭代器，而 `CreateConstIterator` 返回拥有只读访问权限的迭代器。无论哪种情况，均可用这些迭代器的 `Key` 和 `Value` 来检查元素。通过迭代器复制示例中的"fruit"集合产生如下结果：

```c++
for (auto It = FruitSet.CreateConstIterator(); It; ++It)
{
    FPlatformMisc::LocalPrint(
        *FString::Printf(
            TEXT("(%s)\n"),
            *It
        )
    );
}
```

### 查询

调用 `Num` 函数可查询集合中保存的元素数量：

```c++
int32 Count = FruitSet.Num();
// Count == 8
```

要确定集合是否包含特定元素，可按如下所示调用 `Contains` 函数：

```c++
bool bHasBanana = FruitSet.Contains(TEXT("Banana"));
bool bHasLemon = FruitSet.Contains(TEXT("Lemon"));
// bHasBanana == true
// bHasLemon == false
```

使用 `FSetElementId` 结构体可查找集合中某个键的索引。然后，就可使用该索引与 `运算符[]` 查找元素。在非常量集合上调用 `operator[]` 将返回非常量引用，而在常量集合上调用将返回常量引用。

```c++
FSetElementId BananaIndex = FruitSet.Index(TEXT("Banana"));
// BananaIndex is a value between 0 and (FruitSet.Num() - 1)
FPlatformMisc::LocalPrint(
    *FString::Printf(
        TEXT(" \"%s\"\n"),
        *FruitSet[BananaIndex]
    )
);
// Prints "Banana"

FSetElementId LemonIndex = FruitSet.Index(TEXT("Lemon"));
// LemonIndex is INDEX_NONE (-1)
FPlatformMisc::LocalPrint(
    *FString::Printf(
        TEXT(" \"%s\"\n"),
        *FruitSet[LemonIndex]
    )
); // Assert!
```

如果不确定集合中是否包含某个键，可使用 `Contains` 函数和 `运算符[]` 进行检查。但这并非理想的方法，因为同一键需要进行两次查找才能获取成功。使用 `Find` 函数查找一次即可完成这些行为。如果集合中包含该键，`Find` 将返回指向元素数值的指针。如果映射不包含该键，则返回null。对常量集合调用`Find`，返回的指针也将为常量。

```c++
FString* PtrBanana = FruitSet.Find(TEXT("Banana"));
FString* PtrLemon = FruitSet.Find(TEXT("Lemon"));
// *PtrBanana == "Banana"
//  PtrLemon == nullptr
```

`Array` 函数会返回一个 `TArray`，其中填充了 `TSet` 中每个元素的一份副本。被传递的数组在填入前会被清空，因此元素的生成数量将始终等于集合中的元素数量：

```c++
TArray<FString> FruitArray = FruitSet.Array();
// FruitArray == [ "Banana","Grapefruit","Pineapple","Pear","Orange","Kiwi","Melon","Mango" ] (order may vary)
```

### 移除

通过 `Remove` 函数可按索引移除元素，但仅建议在通过元素迭代时使用：Remove函数会返回已删除元素的数量。如果给定的键未包含在集合中，则会返回0。如果 `TSet` 支持重复的键，`Remove` 将移除所有匹配元素。

```c++
FruitSet.Remove(0);
// FruitSet == [ "Grapefruit","Pineapple","Pear","Orange","Kiwi","Melon","Mango" ]
```

移除元素将在数据结构（在Visual Studio的观察窗口中可视化集合时可看到）中留下空位，但为保证清晰度，此处省略。

```c++
int32 RemovedAmountPineapple = FruitSet.Remove(TEXT("Pineapple"));
// RemovedAmountPineapple == 1
// FruitSet == [ "Grapefruit","Pear","Orange","Kiwi","Melon","Mango" ]
FString RemovedAmountLemon = FruitSet.Remove(TEXT("Lemon"));
// RemovedAmountLemon == 0
```

最后，使用 `Empty` 或 `Reset` 函数可将集合中的所有元素移除。

```c++
TSet<FString> FruitSetCopy = FruitSet;
// FruitSetCopy == [ "Grapefruit","Pear","Orange","Kiwi","Melon","Mango" ]

FruitSetCopy.Empty();
// FruitSetCopy == []
Empty` 和 `Reset` 相似，但 `Empty` 可采用参数指示集合中保留的slack量，而 `Reset
```

### 排序

`TSet` 可以排序。排序后，迭代集合会以排序的顺序显示元素，但下次修改集合时，排序可能会发生变化。由于排序不稳定，可能按任何顺序显示集合中支持重复键的等效元素。

`Sort` 函数使用指定排序顺序的二进制谓词，如下所示：

```c++
FruitSet.Sort([](const FString& A, const FString& B) {
    return A > B; // sort by reverse-alphabetical order
});
// FruitSet == [ "Pear", "Orange", "Melon", "Mango", "Kiwi", "Grapefruit" ] (order is temporarily guaranteed)

FruitSet.Sort([](const FString& A, const FString& B) {
    return A.Len() < B.Len(); // sort strings by length, shortest to longest
});
// FruitSet == [ "Pear", "Kiwi", "Melon", "Mango", "Orange", "Grapefruit" ] (order is temporarily guaranteed)
```

### 运算符

和 `TArray` 一样，`TSet` 是常规值类型，可通过标准复制构造函数或赋值运算符进行复制。因为集合严格拥有其元素，复制集合的操作是深层的，所以新集合将拥有其自身的元素副本：

```c++
TSet<FString> NewSet = FruitSet;
NewSet.Add(TEXT("Apple"));
NewSet.Remove(TEXT("Pear"));
// FruitSet == [ "Pear", "Kiwi", "Melon", "Mango", "Orange", "Grapefruit" ]
// NewSet == [ "Kiwi", "Melon", "Mango", "Orange", "Grapefruit", "Apple" ]
```

### Slack

Slack是不包含元素的已分配内存。调用 `Reserve` 可分配内存，无需添加元素；通过非零slack参数调用 `Reset` 或 `Empty` 可移除元素，无需将其使用的内存取消分配。Slack优化了将新元素添加到集合的过程，因为可以使用预先分配的内存，而不必分配新内存。它在移除元素时也十分实用，因为系统不需要将内存取消分配。在清空希望用相同或更少的元素立即重新填充的集合时，此方法尤其有效。

`TSet` 不像 `TArray` 中的 `Max` 函数那样可检查预分配元素的数量。

以下代码可在不取消任何内存的情况下移除集合中的所有元素，从而产生slack：

```c++
FruitSet.Reset();
// FruitSet == [ <invalid>, <invalid>, <invalid>, <invalid>, <invalid>, <invalid> ]
```

使用 `Reserve` 函数可直接创建slack，例如在添加元素之前预分配内存。

```c++
FruitSet.Reserve(10);
for (int32 i = 0; i < 10; ++i)
{
    FruitSet.Add(FString::Printf(TEXT("Fruit%d"), i));
}
// FruitSet == [ "Fruit9", "Fruit8", "Fruit7" ..."Fruit2", "Fruit1", "Fruit0" ]
```



预先分配slack会导致以倒序添加新元素。与数组不同，集合不维护元素排序，处理集合的代码不能指望元素排序稳定或可预测。



使用 `Collapse` 和 `Shrink` 函数可移除 `TSet` 中的全部slack。`Shrink` 将从容器的末端移除所有slack，但这会在中间或开始处留下空白元素。

```c++
// Remove every other element from the set.
for (int32 i = 0; i < 10; i += 2)
{
    FruitSet.Remove(FSetElementId::FromInteger(i));
}
// FruitSet == ["Fruit8", <invalid>, "Fruit6", <invalid>, "Fruit4", <invalid>, "Fruit2", <invalid>, "Fruit0", <invalid> ]

FruitSet.Shrink();
// FruitSet == ["Fruit8", <invalid>, "Fruit6", <invalid>, "Fruit4", <invalid>, "Fruit2", <invalid>, "Fruit0" ]
```

在上述代码中，`Shrink` 只删除了一个无效元素，因为末端只有一个空元素。要移除所有slack，首先应调用 `Compact` 或 `CompactStable` 函数，将空白空间组合在一起，为调用 `Shrink` 做好准备。顾名思义，`CompactStable` 可在合并空元素时保持元素的排序。

```c++
FruitSet.CompactStable();
// FruitSet == ["Fruit8", "Fruit6", "Fruit4", "Fruit2", "Fruit0", <invalid>, <invalid>, <invalid>, <invalid> ]
FruitSet.Shrink();
// FruitSet == ["Fruit8", "Fruit6", "Fruit4", "Fruit2", "Fruit0" ]
```

### DefaultKeyFuncs

只要类型具有 `运算符==` 和非成员 `GetTypeHash` 重载，就可为TSet所用，因为此类型既是元素又是键。然而，不便于重载这些函数时可将类型作为键使用。在这些情况下，可对 `DefaultKeyFuncs` 进行自定义。为键类型创建 `KeyFuncs`，必须定义两个typedef和三个静态函数，如下所示：

- `KeyInitType` —— 用于传递键的类型。通常抽取自ElementType模板参数。
- `ElementInitType` —— 用于传递元素的类型。同样通常抽取自ElementType模板参数，因此与KeyInitType相同。
- `KeyInitType GetSetKey(ElementInitType Element)`——返回元素的键。在集合中，通常是元素本身。
- `bool Matches(KeyInitType A, KeyInitType B)` —— 如果 `A` 和 `B` 等值将返回 `true`，否则返回 `false`。
- `uint32 GetKeyHash(KeyInitType Key)` —— 返回 `Key` 的散列值。

`KeyInitType` 和 `ElementInitType` 是键/元素类型普通传递惯例的typedef。它们通常为浅显类型的一个值和非浅显类型的一个常量引用。请注意，集合的元素类型也是键类型，因此 `DefaultKeyFuncs` 仅使用一种模板参数 `ElementType` 定义两者。

`TSet` 假定在 `DefaultKeyFuncs` 中使用 `Matches` 进行对比结果为相等的两个项也将在 `KeyFuncs` 的 `GetKeyHash` 中返回相同的值。

```c++
DefaultKeyFuncs` 的默认实现时，此规则也适用于 `运算符==` 和 `GetKeyHash
```

### 其他

`CountBytes` 和 `GetAllocatedSize` 函数用于估计内部数组的当前内存使用情况。`CountBytes` 接受 `FArchive` 参数，而 `GetAllocatedSize` 则不接受。这些函数常用于统计报告。

`Dump` 函数接受 `FOutputDevice` 并写出关于集合内容的实现信息。还有一个名为 `DumpHashElements` 的函数，可列出来自所有散列条目的所有元素。这些函数常用于调试。



## TMap（字典或哈希表）[[4\]](https://zhuanlan.zhihu.com/p/137638251#ref_4)

TMap主要由两个类型定义（一个键类型和一个值类型），以关联对的形式存储在映射中。

继 `TArray` 之后，**虚幻引擎4**（UE4）中最常用的容器是 `TMap`。`TMap` 与 `TSet` 类似，它们的结构均基于对键进行散列运算。但与 `TSet` 不同的是，此容器将数据存储为键值对（`TPair<KeyType, ValueType>`），只将键用于存储和获取。

映射有两种类型：`TMap` 和 `TMultiMap`。两者之间的不同点是，`TMap` 中的键是唯一的，而`TMultiMap` 可存储多个相同的键。在 `TMap` 中添加新的键值时，若所用的键与原有的对相同，新对将替换原有的对。在 `TMultiMap` 中，容器可以同时存储新对和原有的对。

## TMap

在 `TMap` 中，键值对被视为映射的元素类型，相当于每一对都是个体对象。在本文中，元素就意味着键值对，而各个组件就被称作元素的键或元素的值。元素类型实际上是 `TPair<KeyType, ElementType>`，但很少需要直接引用 `TPair` 类型。

和 `TArray` 一样，`TMap` 也是同质容器，就是说它所有元素的类型都应完全相同。`TMap` 也是值类型，支持通常的复制、赋值和析构函数运算，以及它的元素的强所有权。在映射被销毁时，它的元素都会被销毁。键和值也必须为值类型。

`TMap` 是散列容器，这意味着键类型必须支持 `GetTypeHash` 函数，并提供 `运算符==` 来比较各个键是否等值。稍后将详细介绍散列。

`TMap` 也可使用任选分配器来控制内存分配行为。但不同于 `TArray`，这些是集合分配器，而不是 `FHeapAllocator` 和 `TInlineAllocator` 之类的标准UE4分配器。集合分配器（`TSetAllocator`类）定义映射应使用的散列桶数量，以及应使用哪个标准UE4分配器来存储散列和元素。

`KeyFuncs` 是最后一个 `TMap` 模板参数，该参数告知映射如何从元素类型获取键，如何比较两个键是否相等，以及如何对键进行散列计算。这些参数有默认值，它们只会返回对键的引用，使用 `运算符==` 确定相等性，并调用非成员 `GetTypeHash` 函数进行散列计算。如果您的键类型支持这些函数，可使用它作为映射键，不需要提供自定义 `KeyFuncs`。

与 `TArray` 不同的是，内存中 `TMap` 元素的相对排序既不可靠也不稳定，对这些元素进行迭代很可能会使它们返回的顺序和它们添加的顺序有所不同。这些元素也不太可能在内存中连续排列。映射的支持数据结构是稀疏数组，这种数组可有效支持元素之间的空位。当元素从映射中被移除时，稀疏数组中就会出现空位。将新的元素添加到数组可填补这些空位。但是，即便 `TMap` 不会打乱元素来填补空位，指向映射元素的指针仍然可能失效，因为如果存储器被填满，又添加了新的元素，整个存储可能会重新分配。

## 创建和填充映射

`TMap` 的创建方法如下：

```c++
TMap<int32, FString> FruitMap;
```

`FruitMap` 现在是一个字符串的空 `TMap`，该字符串由整数键标识。我们既没有指定分配器，也没有指定 `KeyFuncs`，所以映射将执行标准的堆分配，使用 `运算符==` 对键进行对比（`int32` 类型），并使用 `GetTypeHash` 进行散列运算。此时没有分配任何内存。

填充映射的标准方法是调用带一个键和值的 `Add` 函数：

```c++
FruitMap.Add(5, TEXT("Banana"));
FruitMap.Add(2, TEXT("Grapefruit"));
FruitMap.Add(7, TEXT("Pineapple"));
// FruitMap == [
//  { Key:5, Value:"Banana"     },
//  { Key:2, Value:"Grapefruit" },
//  { Key:7, Value:"Pineapple"  }
// ]
```



此处的元素按插入顺序排列，但不保证这些元素在内存中实际保留此排序。如果是新的映射，可能会保留插入排序，但插入和删除的次数越多，新元素不出现在末尾的可能性就越大。



这不是 `TMultiMap`，所以各个键都必定是唯一。如果尝试添加重复键，将发生以下情况：

```c++
FruitMap.Add(2, TEXT("Pear"));
// FruitMap == [
//  { Key:5, Value:"Banana"    },
//  { Key:2, Value:"Pear"      },
//  { Key:7, Value:"Pineapple" }
// ]
```

映射仍然包含3个元素，但之前键值为2的"Grapefruit"已被"Pear"替代。

`Add` 函数可接受不带值的键。调用此重载后的 `Add` 时，值将被默认构建：

```c++
FruitMap.Add(4);
// FruitMap == [
//  { Key:5, Value:"Banana"    },
//  { Key:2, Value:"Pear"      },
//  { Key:7, Value:"Pineapple" },
//  { Key:4, Value:""          }
// ]
```

和 `TArray` 一样，还可使用 `Emplace` 代替 `Add`，防止插入映射时创建临时文件：

```c++
FruitMap.Emplace(3, TEXT("Orange"));
// FruitMap == [
//  { Key:5, Value:"Banana"    },
//  { Key:2, Value:"Pear"      },
//  { Key:7, Value:"Pineapple" },
//  { Key:4, Value:""          },
//  { Key:3, Value:"Orange"    }
// ]
```

此处直接将键和值传递给了各自的构造函数。这对 `int32` 键实际上没有影响，但避免了为该值创建临时 `FString`。与 `TArray` 不同的是，只能通过单一参数构造函数将元素安放到映射中。

也可使用 `Append` 函数合并映射，将一个映射的所有元素移至另一个映射：

```c++
TMap<int32, FString> FruitMap2;
FruitMap2.Emplace(4, TEXT("Kiwi"));
FruitMap2.Emplace(9, TEXT("Melon"));
FruitMap2.Emplace(5, TEXT("Mango"));
FruitMap.Append(FruitMap2);
// FruitMap == [
//  { Key:5, Value:"Mango"     },
//  { Key:2, Value:"Pear"      },
//  { Key:7, Value:"Pineapple" },
//  { Key:4, Value:"Kiwi"      },
//  { Key:3, Value:"Orange"    },
//  { Key:9, Value:"Melon"     }
// ]
// FruitMap2 is now empty.
```

在上面的示例中，生成的映射和使用 `Add` 或 `Emplace` 逐个添加 `FruitMap2` 的元素相同，在该过程完成时会清空 `FruitMap2`。这意味着如果 `FruitMap2` 中任何元素的键与 `FruitMap` 中原有元素的键相同，就会取代该元素。

如果用 `UPROPERTY` 宏和一个可编辑的关键词（`EditAnywhere`、`EditDefaultsOnly` 或 `EditInstanceOnly`）标记 `TMap`，即可[在编辑器中添加和编辑元素](https://docs.unrealengine.com/5.1/zh-CN/level-editor-details-panel-in-unreal-engine)。

```c++
UPROPERTY(Category = MapsAndSets, EditAnywhere)
TMap<int32, FString> FruitMap;
```

## 迭代

`TMaps` 的迭代类似于 `TArrays`。可使用C++的设置范围功能，注意元素类型是 `TPair`：

```c++
for (auto& Elem :FruitMap)
{
    FPlatformMisc::LocalPrint(
        *FString::Printf(
            TEXT("(%d, \"%s\")\n"),
            Elem.Key,
            *Elem.Value
        )
    );
}
// Output:
// (5, "Mango")
// (2, "Pear")
// (7, "Pineapple")
// (4, "Kiwi")
// (3, "Orange")
// (9, "Melon")
```

也可以用 `CreateIterator` 和 `CreateConstIterators` 函数来创建迭代器。`CreateIterator` 返回拥有读写访问权限的迭代器，而 `CreateConstIterator` 返回拥有只读访问权限的迭代器。无论哪种情况，均可用这些迭代器的 `Key` 和 `Value` 来检查元素。使用迭代器显示"fruit"范例映射将产生如下结果：

```c++
for (auto It = FruitMap.CreateConstIterator(); It; ++It)
{
    FPlatformMisc::LocalPrint(
        *FString::Printf(
            TEXT("(%d, \"%s\")\n"),
            It.Key(),   // same as It->Key
            *It.Value() // same as *It->Value
        )
    );
}
```

## 查询

调用 `Num` 函数即可查询映射中保存的元素数量：

```c++
int32 Count = FruitMap.Num();
// Count == 6
```

要确定映射是否包含特定键，可按下方所示调用 `Contains` 函数：

```c++
bool bHas7 = FruitMap.Contains(7);
bool bHas8 = FruitMap.Contains(8);
// bHas7 == true
// bHas8 == false
```

如果知道映射中存在某个特定键，可使用 `运算符[]` 查找相应值，将键用作索引。使用非常量映射执行该操作将返回非常量引用，使用常量映射将返回常量引用。

```c++
运算符[]
FString Val7 = FruitMap[7];
// Val7 == "Pineapple"
FString Val8 = FruitMap[8];
// Assert!
```

如果不确定映射中是否包含某个键，可使用 `Contains` 函数和 `运算符[]` 进行检查。但这并非理想的方法，因为同一键需要进行两次查找才能获取成功。使用 `Find` 函数查找一次即可完成这些行为。如果映射包含该键，`Find` 将返回指向元素数值的指针。如果映射不包含该键，则返回null。在常量映射上调用 `Find`，返回的指针也将为常量。

```c++
FString* Ptr7 = FruitMap.Find(7);
FString* Ptr8 = FruitMap.Find(8);
// *Ptr7 == "Pineapple"
//  Ptr8 == nullptr
```

或为了确保查询的结果有效，可使用 `FindOrAdd` 或 `FindRef`。`FindOrAdd` 将返回对与给定键关联的值的引用。如果映射中不存在该键，`FindOrAdd` 将返回新创建的元素（使用给定键和默认构建值），该元素也会被添加到映射。`FindOrAdd` 可修改映射，因此仅适用于非常量映射。不要被名称迷惑，`FindRef` 会返回与给定键关联的值副本；若映射中未找到给定键，则返回默认构建值。`FindRef` 不会创建新元素，因此既可用于常量映射，也可用于非常量映射。即使在映射中找不到键，`FindOrAdd` 和 `FindRef` 也会成功运行，因此无需执行常规的安全规程（如提前检查 `Contains` 或对返回值进行空白检查）就可安全地调用。

```c++
FString& Ref7 = FruitMap.FindOrAdd(7);
// Ref7     == "Pineapple"
// FruitMap == [
//  { Key:5, Value:"Mango"     },
//  { Key:2, Value:"Pear"      },
//  { Key:7, Value:"Pineapple" },
//  { Key:4, Value:"Kiwi"      },
//  { Key:3, Value:"Orange"    },
//  { Key:9, Value:"Melon"     }
// ]
FString& Ref8 = FruitMap.FindOrAdd(8);
// Ref8     == ""
// FruitMap == [
//  { Key:5, Value:"Mango"     },
//  { Key:2, Value:"Pear"      },
//  { Key:7, Value:"Pineapple" },
//  { Key:4, Value:"Kiwi"      },
//  { Key:3, Value:"Orange"    },
//  { Key:9, Value:"Melon"     },
//  { Key:8, Value:""          }
// ]

FString Val7 = FruitMap.FindRef(7);
FString Val6 = FruitMap.FindRef(6);
// Val7     == "Pineapple"
// Val6     == ""
// FruitMap == [
//  { Key:5, Value:"Mango"     },
//  { Key:2, Value:"Pear"      },
//  { Key:7, Value:"Pineapple" },
//  { Key:4, Value:"Kiwi"      },
//  { Key:3, Value:"Orange"    },
//  { Key:9, Value:"Melon"     },
//  { Key:8, Value:""          }
// ]
```

和示例中初始化 `Ref8` 时一样，`FindOrAdd` 可向映射添加新条目，因此之前获得的指针（来自 `Find`）或引用（来自 `FindOrAdd`）可能会无效。如果映射的后端存储需要扩展以容纳新元素，会执行分配内存和移动现有数据的添加操作，从而导致这一结果。以上示例中，在调用 `FindOrAdd(8)` 之后，`Ref7` 可能会紧随 `Ref8` 失效。

`FindKey` 函数执行逆向查找，这意味着提供的值与键匹配，并返回指向与所提供值配对的第一个键的指针。搜索映射中不存在的值将返回空键。

```c++
const int32* KeyMangoPtr   = FruitMap.FindKey(TEXT("Mango"));
const int32* KeyKumquatPtr = FruitMap.FindKey(TEXT("Kumquat"));
// *KeyMangoPtr   == 5
//  KeyKumquatPtr == nullptr
```

按值查找比按键查找慢（线性时间）。这是因为映射按键排序，而非按值排序。此外，如果映射有多个具有相同值的键，`FindKey` 可返回其中任一键。

`GenerateKeyArray` 和 `GenerateValueArray` 分别使用所有键和值的副本来填充 `TArray`。在这两种情况下，都会在填充前清空所传递的数组，因此产生的元素数量始终等于映射中的元素数量。

```c++
TArray<int32>   FruitKeys;
TArray<FString> FruitValues;
FruitKeys.Add(999);
FruitKeys.Add(123);
FruitMap.GenerateKeyArray  (FruitKeys);
FruitMap.GenerateValueArray(FruitValues);
// FruitKeys   == [ 5,2,7,4,3,9,8 ]
// FruitValues == [ "Mango","Pear","Pineapple","Kiwi","Orange",
//                  "Melon","" ]
```

## 移除

从映射中移除元素的方法是使用 `Remove` 函数并提供要移除元素的键。返回值是被移除元素的数量。如果映射不包含与键匹配的元素，则返回值可为零。

```c++
FruitMap.Remove(8);
// FruitMap == [
//  { Key:5, Value:"Mango"     },
//  { Key:2, Value:"Pear"      },
//  { Key:7, Value:"Pineapple" },
//  { Key:4, Value:"Kiwi"      },
//  { Key:3, Value:"Orange"    },
//  { Key:9, Value:"Melon"     }
// ]
```

移除元素将在数据结构（在Visual Studio的观察窗口中可视化映射时可看到）中留下空位，但为保证清晰度，此处省略。

`FindAndRemoveChecked` 函数可用于从映射移除元素并返回其值。名称的"已检查"部分表示若键不存在，映射将调用 `check`（UE4中等同于 `assert`）。

```c++
FString Removed7 = FruitMap.FindAndRemoveChecked(7);
// Removed7 == "Pineapple"
// FruitMap == [
//  { Key:5, Value:"Mango"  },
//  { Key:2, Value:"Pear"   },
//  { Key:4, Value:"Kiwi"   },
//  { Key:3, Value:"Orange" },
//  { Key:9, Value:"Melon"  }
// ]

FString Removed8 = FruitMap.FindAndRemoveChecked(8);
// Assert!
```

`RemoveAndCopyValue` 函数的作用与 `Remove` 相似，不同点是会将已移除元素的值复制到引用参数。如果映射中不存在指定的键，则输出参数将保持不变，函数将返回 `false`。

```c++
FString Removed;
bool bFound2 = FruitMap.RemoveAndCopyValue(2, Removed);
// bFound2  == true
// Removed  == "Pear"
// FruitMap == [
//  { Key:5, Value:"Mango"  },
//  { Key:4, Value:"Kiwi"   },
//  { Key:3, Value:"Orange" },
//  { Key:9, Value:"Melon"  }
// ]
bool bFound8 = FruitMap.RemoveAndCopyValue(8, Removed);
// bFound8  == false
// Removed  == "Pear", i.e. unchanged
// FruitMap == [
//  { Key:5, Value:"Mango"  },
//  { Key:4, Value:"Kiwi"   },
//  { Key:3, Value:"Orange" },
//  { Key:9, Value:"Melon"  }
// ]
```

最后，使用 `Empty` 或 `Reset` 函数可将映射中的所有元素移除。

```c++
TMap<int32, FString> FruitMapCopy = FruitMap;
// FruitMapCopy == [
//  { Key:5, Value:"Mango"  },
//  { Key:4, Value:"Kiwi"   },
//  { Key:3, Value:"Orange" },
//  { Key:9, Value:"Melon"  }
// ]

FruitMapCopy.Empty();       // We could also have called Reset() here.
// FruitMapCopy == []
Empty` 和 `Reset` 相似，但 `Empty` 可采用参数指示映射中保留的slack量，而 `Reset
```

## 排序

`TMap` 可以进行排序。排序后，迭代映射会以排序的顺序显示元素，但下次修改映射时，排序可能会发生变化。排序是不稳定的，因此等值元素在MultiMap中可能以任何顺序出现。

使用 `KeySort` 或 `ValueSort` 函数可分别按键和值进行排序。两个函数均使用二元谓词来进行排序：

```c++
FruitMap.KeySort([](int32 A, int32 B) {
    return A > B; // sort keys in reverse
});
// FruitMap == [
//  { Key:9, Value:"Melon"  },
//  { Key:5, Value:"Mango"  },
//  { Key:4, Value:"Kiwi"   },
//  { Key:3, Value:"Orange" }
// ]

FruitMap.ValueSort([](const FString& A, const FString& B) {
    return A.Len() < B.Len(); // sort strings by length
});
// FruitMap == [
//  { Key:4, Value:"Kiwi"   },
//  { Key:5, Value:"Mango"  },
//  { Key:9, Value:"Melon"  },
//  { Key:3, Value:"Orange" }
// ]
```

## 运算符

和 `TArray` 一样，`TMap` 是常规值类型，可通过标准复制构造函数或赋值运算符进行复制。因为映射严格拥有其元素，复制映射的操作是深层的，所以新的映射将拥有其自己的元素副本。

```c++
TMap<int32, FString> NewMap = FruitMap;
NewMap[5] = "Apple";
NewMap.Remove(3);
// FruitMap == [
//  { Key:4, Value:"Kiwi"   },
//  { Key:5, Value:"Mango"  },
//  { Key:9, Value:"Melon"  },
//  { Key:3, Value:"Orange" }
// ]
// NewMap == [
//  { Key:4, Value:"Kiwi"  },
//  { Key:5, Value:"Apple" },
//  { Key:9, Value:"Melon" }
// ]
```

`TMap` 支持移动语义，使用 `MoveTemp` 函数可调用这些语义。在移动后，源映射必定为空：

```c++
FruitMap = MoveTemp(NewMap);
// FruitMap == [
//  { Key:4, Value:"Kiwi"  },
//  { Key:5, Value:"Apple" },
//  { Key:9, Value:"Melon" }
// ]
// NewMap == []
```

## Slack

Slack是不包含元素的已分配内存。调用 `Reserve` 可分配内存，无需添加元素；通过非零slack参数调用 `Reset` 或 `Empty` 可移除元素，无需将其使用的内存取消分配。Slack优化了将新元素添加到映射的过程，因为可以使用预先分配的内存，而不必分配新内存。它在移除元素时也十分实用，因为系统不需要将内存取消分配。在清空希望用相同或更少的元素立即重新填充的映射时，此方法尤其有效。

`TMap` 不像 `TArray` 中的 `Max` 函数那样可以检查预分配元素的数量。

在下列代码中，`Reserve` 函数预先分配映射，最多可包含10个元素。

```c++
FruitMap.Reserve(10);
for (int32 i = 0; i < 10; ++i)
{
    FruitMap.Add(i, FString::Printf(TEXT("Fruit%d"), i));
}
// FruitMap == [
//  { Key:9, Value:"Fruit9" },
//  { Key:8, Value:"Fruit8" },
//  ...
//  { Key:1, Value:"Fruit1" },
//  { Key:0, Value:"Fruit0" }
// ]
```

使用 `Collapse` 和 `Shrink` 函数可移除 `TMap` 中的全部slack。`Shrink` 将从容器的末端移除所有slack，但这会在中间或开始处留下空白元素。

```c++
for (int32 i = 0; i < 10; i += 2)
{
    FruitMap.Remove(i);
}
// FruitMap == [
//  { Key:9, Value:"Fruit9" },
//  <invalid>,
//  { Key:7, Value:"Fruit7" },
//  <invalid>,
//  { Key:5, Value:"Fruit5" },
//  <invalid>,
//  { Key:3, Value:"Fruit3" },
//  <invalid>,
//  { Key:1, Value:"Fruit1" },
//  <invalid>
// ]
FruitMap.Shrink();
// FruitMap == [
//  { Key:9, Value:"Fruit9" },
//  <invalid>,
//  { Key:7, Value:"Fruit7" },
//  <invalid>,
//  { Key:5, Value:"Fruit5" },
//  <invalid>,
//  { Key:3, Value:"Fruit3" },
//  <invalid>,
//  { Key:1, Value:"Fruit1" }
// ]
```

在上述代码中，`Shrink` 只删除了一个无效元素，因为末端只有一个空元素。要移除所有slack，首先应调用 `Compact` 函数，将空白空间组合在一起，为调用 `Shrink` 做好准备。

```c++
FruitMap.Compact();
// FruitMap == [
//  { Key:9, Value:"Fruit9" },
//  { Key:7, Value:"Fruit7" },
//  { Key:5, Value:"Fruit5" },
//  { Key:3, Value:"Fruit3" },
//  { Key:1, Value:"Fruit1" },
//  <invalid>,
//  <invalid>,
//  <invalid>,
//  <invalid>
// ]
FruitMap.Shrink();
// FruitMap == [
//  { Key:9, Value:"Fruit9" },
//  { Key:7, Value:"Fruit7" },
//  { Key:5, Value:"Fruit5" },
//  { Key:3, Value:"Fruit3" },
//  { Key:1, Value:"Fruit1" }
// ]
```

## KeyFuncs

只要类型具有 `运算符==` 和非成员 `GetTypeHash` 重载，就可用作 `TMap` 的键类型，不需要任何更改。但是，您可能需要将类型用作键，而不重载这些函数。在这些情况下，可对 `KeyFuncs` 进行自定义。为键类型创建 `KeyFuncs`，必须定义两个typedef和三个静态函数，如下所示：

- `KeyInitType` —— 用于传递键的类型。
- `ElementInitType` —— 用于传递元素的类型。
- `KeyInitType GetSetKey(ElementInitType Element)`——返回元素的键。
- `bool Matches(KeyInitType A, KeyInitType B)` —— 如果 `A` 和 `B` 等值将返回 `true`，否则返回 `false`。
- `uint32 GetKeyHash(KeyInitType Key)` —— 返回 `Key` 的散列值。

`KeyInitType` 和 `ElementInitType` 是键类型和值类型的常规传递约定的typedef。它们通常为浅显类型的一个值，和非浅显类型的一个常量引用。请记住，映射的元素类型是 `TPair`。

自定义 `KeyFuncs` 的示例可能如下所示：

```c++
struct FMyStruct
{
    // String which identifies our key
    FString UniqueID;

    // Some state which doesn't affect struct identity
    float SomeFloat;

    explicit FMyStruct(float InFloat)
        :UniqueID (FGuid::NewGuid().ToString())
        , SomeFloat(InFloat)
    {
    }
};
template <typename ValueType>
struct TMyStructMapKeyFuncs :
    BaseKeyFuncs<
        TPair<FMyStruct, ValueType>,
        FString
    >
{
private:
    typedef BaseKeyFuncs<
        TPair<FMyStruct, ValueType>,
        FString
    > Super;

public:
    typedef typename Super::ElementInitType ElementInitType;
    typedef typename Super::KeyInitType     KeyInitType;

    static KeyInitType GetSetKey(ElementInitType Element)
    {
        return Element.Key.UniqueID;
    }

    static bool Matches(KeyInitType A, KeyInitType B)
    {
        return A.Compare(B, ESearchCase::CaseSensitive) == 0;
    }

    static uint32 GetKeyHash(KeyInitType Key)
    {
        return FCrc::StrCrc32(*Key);
    }
};
```

`FMyStruct` 具有唯一标识符，以及一些与身份无关的其他数据。`GetTypeHash` 和 `运算符==` 不适用于此，因为 `运算符==` 为实现通用目的不应忽略任何类型的数据，但同时又需要如此才能与 `GetTypeHash` 的行为保持一致，后者只关注 `UniqueID` 字段。以下步骤有助于为 `FMyStruct` 创建自定义 `KeyFuncs`：

1. 首先，继承 `BaseKeyFuncs`，因为它可以帮助定义某些类型，包括 `KeyInitType` 和 `ElementInitType`。

   `BaseKeyFuncs` 使用两个模板参数：映射的元素类型和键类型。和所有映射一样，元素类型是 `TPair`，使用 `FMyStruct` 作为其 `KeyType`，`TMyStructMapKeyFuncs` 的模板参数作为其 `ValueType`。将备用 `KeyFuncs` 用作模板，可为每个映射指定 `ValueType`，因此每次要在 `FMyStruct` 上创建键控 `TMap` 时不必定义新的 `KeyFuncs`。第二个 `BaseKeyFuncs` 参数是键类型，不要与元素存储的键区（`TPair` 的 `KeyType`）混淆。因为此映射应使用 `UniqueID`（来自 `FMyStruct`）作为键，所以此处使用 `FString`。

2. 然后，定义三个必需的 `KeyFuncs` 静态函数。第一个是 `GetSetKey`，该函数返回给定元素类型的键。由于元素类型是 `TPair`，而键是 `UniqueID`，所以该函数可直接返回 `UniqueID`。

   第二个静态函数是 `Matches`，该函数接受两个元素的键（由 `GetSetKey` 获取），然后比较它们是否相等。在 `FString` 中，标准的等效测试（`运算符==`）不区分大小写；要替换为区分大小写的搜索，请用相应的大小写对比选项使用 `Compare` 函数。

3. 最后，`GetKeyHash` 静态函数接受提取的键并返回其散列值。由于 `Matches` 函数区分大小写，`GetKeyHash` 也必须区分大小写。区分大小写的 `FCrc` 函数将计算键字符串的散列值。

4. 现在结构已满足 `TMap` 要求的行为，可创建它的实例。

```c++
KeyFuncs` 参数处于最后，所以这个 `TMap
    TMap<
        FMyStruct,
        int32,
        FDefaultSetAllocator,
        TMyStructMapKeyFuncs<int32>
    > MyMapToInt32;

    // Add some elements
    MyMapToInt32.Add(FMyStruct(3.14f), 5);
    MyMapToInt32.Add(FMyStruct(1.23f), 2);

    // MyMapToInt32 == [
    //  {
    //      Key:{
    //          UniqueID:"D06AABBA466CAA4EB62D2F97936274E4",
    //          SomeFloat:3.14f
    //      },
    //      Value:5
    //  },
    //  {
    //      Key:{
    //          UniqueID:"0661218447650259FD4E33AD6C9C5DCB",
    //          SomeFloat:1.23f
    //      },
    //      Value:5
    //  }
    // ]
TMap` 假设两个项目使用 `Matches` 比较的结果相等，则它们会从 `GetKeyHash` 返回相同的值。此外，如果对现有映射元素的键进行的修改将会改变来自这两个函数中任一个的结果，那么系统会将这种修改视作未定义的行为，因为这会使映射的内部散列失效。这些规则也适用于使用默认 `KeyFuncs` 时 `运算符==` 和 `GetKeyHash
```

## 其他

`CountBytes` 和 `GetAllocatedSize` 函数用于估计内部数组的当前内存使用情况。`CountBytes` 接受 `Farchive` 参数，而 `GetAllocatedSize` 则不会。这些函数常用于统计报告。

`Dump` 函数接受 `FOutputDevice`，并写出关于映射内容的实现信息。此函数常用于调试。

## 参考

1. [^](https://zhuanlan.zhihu.com/p/137638251#ref_1_0)static_assert https://zh.cppreference.com/w/cpp/language/static_assert
2. [^](https://zhuanlan.zhihu.com/p/137638251#ref_2_0)TArray https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/TArrays/index.html
3. [^](https://zhuanlan.zhihu.com/p/137638251#ref_3_0)TSet https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/TSet/index.html
4. [^](https://zhuanlan.zhihu.com/p/137638251#ref_4_0)TMap https://docs.unrealengine.com/en-US/Programming/UnrealArchitecture/TMap/index.html