# UE4 C++基础 - 编码规范

## 为什么要学习编码规范？

良好的编码规范不仅利于项目维护，也增加了代码辨识度。使我们在阅读代码时能够更加清晰的理解代码意图。维护编码规范不是一件机械化的工作，它更像是一门艺术，让我们在有限的规范内发挥自己的创造力。除此之外，还有如下几个原因：

- 软件生命周期中80%的时间皆需要维护。
- 原开发者几乎不会对软件进行终身维护。
- 代码规则可提高软件可读性，让工程师更加快速透彻地理解新代码。
- 如决定向模组社区开发者公开源代码，则源代码需要易于理解。
- 交叉编译器兼容性实际上需要此类规则。

详细的编码规范可以参考UE4的官方文档[[1\]](https://zhuanlan.zhihu.com/p/137799980#ref_1)，这里我们只摘录其中有利于初学者阅读代码的部分。

## UE4命名规范

## Pascal命名法

每个单词首字母大写，如 `Health`或 `UPrimitiveComponent` 。变量、方法和类的命名应清楚、明了且进行描述。命名的范围越大，一个良好的描述性命名就越重要。避免过度缩写。所有变量应逐个声明，以便对变量的含义提供注释。其同样被JavaDocs格式需要。变量前可使用多行或单行注释，空白行为分组变量可选使用。

## 类型前缀

**类型名称**前有一个额外的大写字母用于区分不同的类型和普通的变量名。如 FSkin 是个类型名，**Skin** 则是 **FSkin** 的实例。下面是各种前缀以及其含义：

- **T** 表示模板类
- **U** 表示类继承自 UObject
- **A** 表示类继承自 AActor
- **S** 表示类继承自 SWidget
- **I** 表示抽象接口类
- **E** 表示枚举
- **b** 表示布尔值
- **F** 表示其他不满足上述规则的类

## 样例

```cpp
float TeaWeight;
int32 TeaCount;
bool bDoesTeaStink;
FName TeaName;
FString TeaFriendlyName;
UClass* TeaClass;
USoundCue* TeaSound;
UTexture* TeaTexture;
```

## 其他

typedef 应该以与其类型相符的字母为前缀，若为结构体，则使用F；若为UObject则使用U，以此类推。此外，使用模板实例化的类不再是模板，也应该加上相应前缀，如：

```cpp
typedef TArray<FMytype> FArrayOfMyTypes;
```

类型和变量的命名为名词。

方法名是动词，用于描述方法的效果或者无副作用的返回值。

返回布尔的函数应发起true/false的询问，如:

```cpp
IsVisible();
ShouldClearBuffer();
```

若函数参数通过引用传递，同时该值会写入函数，建议以“Out”做为函数参数命名的前缀（非必需）。此操作将明确表明传入该参数的值将被函数替换。

若In或Out参数同样为布尔，以b作为In/Out的前缀，如 `bOutResult`。

返回值的函数应描述返回的值.命名应说明函数将返回的值。此规则对布尔函数极为重要。请参考以下两个范例方法：

```cpp
// True的意义是什么？
bool CheckTea(FTea Tea);

// 命名明确说明茶是新鲜的
bool IsTeaFresh(FTea Tea);
```



## 参考

1. [^](https://zhuanlan.zhihu.com/p/137799980#ref_1_0)Coding Standard https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard/index.html