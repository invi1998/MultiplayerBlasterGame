# UE4 C++基础 - 资源常见名词解释

## 背景

由于缺少UE4官方文档对关键概念进行解释，如：

- 什么是Outer？
- Outermost是何含义？
- Package是什么？
- Exporter和Impoter的关系是什么？

该篇文档是根据源码和代码注释，以及一些古老的资料，结合笔者对引擎理解给出的定义，提前理解这些概念，有利于我们更快的学习和阅读源码。如有错误的地方，欢迎指证 ;-) 。

## Object（对象）

所有继承自UObject类并创建的对象都称之为***Object\***。排除通过 new 创建的对象，剩下几乎所有的对象都是Object对象。编辑器中拖动的Actor，蓝图，游戏中各种常见资源如贴图,地图,音频资源等。甚至包括C++源码中通过UClass声明的类，也是Object。不出意外的话，几乎99%的时间都是在和UObject打交道。假想一个FPS游戏中人物死亡的场景，从技术实现角度来讲，这个角色包括：

- 角色骨骼 Skeleton
- 人物材质 MaterialInstanceConstant
- 贴图 Texture2D
- 等等

当角色死亡消失时，我们希望玩家看到的应该是整个角色完整的消失，并不希望将角色这个整体打散对待（例如，剩下一张头像贴图），也不希望把这项清理的工作交给编程人员，要求他们必须硬编码手动释放对象。这应该是一个自然递归的过程。UE4采用组合模式来解决此类问题。

## Outer（父对象）

所有的Object都拥有一个***Outer\***，称之为***父对象\***。每个Object都可以有（至多）一个Outer（父UObject），且可以拥有任意数量的子Object，换句话说，每个子对象的类型必须是UObject或者必须派生自UObject，每个Object将自身的Outer储存在OuterPrivate信息中，子对象的信息则是构建Object调用AddObject函数，通过全局的FUObjectHashTables 来储存的。除了自举时（bootstrapping）创建的UObject，所有其他的UObject在创建时都会调用AddObject将自身添加进全局表。

```cpp
// Add to global table.
AddObject(InName, InInternalFlags);
```

全局表由以下几张子表组成：

```cpp
/** Hash sets */
TMap<int32, FHashBucket> Hash; // 全局Hash表，可以通过对象名称查找到对应的对象
TMultiMap<int32, class UObjectBase*> HashOuter; // Outer对象表，可以通过Hash值查找到对应的Outer

/** Map of object to their outers, used to avoid an object iterator to find such things. **/
TMap<UObjectBase*, FHashBucket> ObjectOuterMap; // 子对象表，可以查询任意对象的子对象
TMap<UClass*, TSet<UObjectBase*> > ClassToObjectListMap; // 类型表，可以通过类型查找对应的对象
TMap<UClass*, TSet<UClass*> > ClassToChildListMap; // ？？还没理解到用途
```

## Package（包）[[1\]](https://zhuanlan.zhihu.com/p/152201635#ref_1)

UE4中的Package是一个抽象的概念，一个Package是一个文件（.uasset或者.umap），它包含一些可以UE4进行操作和访问的二进制数据文件。Package包含了各种各样的游戏资源，包括

- 贴图 Textures
- 静态网格物体 static meshes
- 骨架网格物体 skeletal meshes
- 物理资源 physics assets
- UI场景 等等...

凡是可以添加进游戏引擎的文件，都可以包含进Package，反之也成立。一个Object一定属于某一个Package，Object的Package名称称之为**PackageName**。路径形式如下

```text
/Engine/EngineDamageTypes/DmgTypeBP_Environmental
```

对象之间可以存在引用关系（例如，材质依赖于贴图）。

- **Exports** （导出资源）表示存在于该包中的对象。
- **Imports**（导入资源）表示对其他包对象的引用。

例如：如果包Ａ中有一个材质 MatX，并且它引用包A中的TexY和包B中的TexZ，则关系如下所示：

```ada
Package A:
    Exports:
          MatX
          TexY
    Imports:
          B.TexZ

Package B:
    Exports:
          TexZ
```

当我们在编辑器或者游戏中加载Package时，它会加载引用Package中的objects来解决引用关系。但是并不会加载引用Package中的所有objects。仅从引用包中必须加载的objects。

**关卡**（***Levels\***）是一种特殊的资源。关卡不包含具体的资源，它只包含Package中的资源引用。这样可以允许多个关卡来共享资源，并允许美术工作人员仅需将包文件中的资源改变一次，所有引用该资源的关卡将会自动地进行更新。

## **Package Types（包类型）**

包可以有不同的扩展名，但是它们在内部是完全一致的。虚幻引擎只关心包中的内容，而不包含任何处理特定扩展名的特殊情况。 然而，通用命名习惯：

- uasset 通用资源扩展名
- umap 关卡地图默认扩展名

## Groups（组）

包可以包含多层的组。可以把它们想象成您硬盘上文件结构中的子目录。 这些组的使用完全是为了使人们可以搞清包内容的意思；引擎不会关心您是否使用这些组。UE4支持多层次的组结构；但是，尽管您可以按照您的意愿设计组层次的深度，但通常推荐最多建立 3 或 4 层。否则，当使用内容浏览器包的树结构时，它将会变得特别不实用。以下是使用多个组来组织装甲资源的例子：

![img](https://pic4.zhimg.com/80/v2-acaf796218aa7567df62cb3ada487acf_720w.webp)

这个例子中的包是Pickups（可能在硬盘的Pickups.uasset中）。包内可能有很多组用于组织它内部的资源。比如，假设这个选中的资源，获得指定资源的路径是`Pickups.Armor.Materials`。类似于访问硬盘（"C:\Pickups\Armor\Materials"）。每个组都可以包含资源，就像目录可以包含文件一样。

## 资源路径格式名称和转换函数[[2\]](https://zhuanlan.zhihu.com/p/152201635#ref_2)

各种路径的格式如下所示:

```ada
ObjectPath: /Engine/EngineDamageTypes/DmgTypeBP_Environmental.DmgTypeBP_Environmental
PackageName: /Engine/EngineDamageTypes/DmgTypeBP_Environmental
ObjectName: DmgTypeBP_Environmental
Filename: ../../../Engine/Content/EngineDamageTypes/DmgTypeBP_Environmental
PackagePath: /Engine/EngineDamageTypes
ShortName: DmgTypeBP_Environmental
AssetName: DmgTypeBP_Environmental
AssetPackageExtension: .uasset
MapPackageExtension: .umap
```

路径转换函数代码

```cpp
// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#include "BlankProgram.h"
#include "RequiredProgramMainCPPInclude.h"
#include "Misc/PackageName.h"

DEFINE_LOG_CATEGORY_STATIC(LogBlankProgram, Log, All);
IMPLEMENT_APPLICATION(BlankProgram, "BlankProgram");

INT32_MAIN_INT32_ARGC_TCHAR_ARGV() {
GEngineLoop.PreInit(ArgC, ArgV);

FString ObjectPath = TEXT("/Engine/EngineDamageTypes/DmgTypeBP_Environmental.DmgTypeBP_Environmental");
FString PackageName = FPackageName::ObjectPathToPackageName(ObjectPath);
FString ObjectName = FPackageName::ObjectPathToObjectName(ObjectPath);
FString Filename = FPackageName::LongPackageNameToFilename(PackageName);
FString PackageName2 = FPackageName::FilenameToLongPackageName(Filename);
FString PackagePath = FPackageName::GetLongPackagePath(PackageName);
FString ShortName = FPackageName::GetLongPackageAssetName(PackageName);
FString AssetName = FPackageName::GetShortName(PackageName);
FString AssetPackageExtension = FPackageName::GetAssetPackageExtension();
FString MapPackageExtension = FPackageName::GetMapPackageExtension();

UE_LOG(LogBlankProgram, Display, TEXT("ObjectPath: %s"), *ObjectPath);
UE_LOG(LogBlankProgram, Display, TEXT("PackageName: %s"), *PackageName);
UE_LOG(LogBlankProgram, Display, TEXT("ObjectName: %s"), *ObjectName);
UE_LOG(LogBlankProgram, Display, TEXT("Filename: %s"), *Filename);
UE_LOG(LogBlankProgram, Display, TEXT("PackagePath: %s"), *PackagePath);
UE_LOG(LogBlankProgram, Display, TEXT("ShortName: %s"), *ShortName);
UE_LOG(LogBlankProgram, Display, TEXT("AssetName: %s"), *AssetName);
UE_LOG(LogBlankProgram, Display, TEXT("AssetPackageExtension: %s"), *AssetPackageExtension);
UE_LOG(LogBlankProgram, Display, TEXT("MapPackageExtension: %s"), *MapPackageExtension);

return 0;
}
```

输出如下

```abap
ObjectPath: /Engine/EngineDamageTypes/DmgTypeBP_Environmental.DmgTypeBP_Environmental
PackageName: /Engine/EngineDamageTypes/DmgTypeBP_Environmental
ObjectName: DmgTypeBP_Environmental
Filename: ../../../Engine/Content/EngineDamageTypes/DmgTypeBP_Environmental
PackagePath: /Engine/EngineDamageTypes
ShortName: DmgTypeBP_Environmental
AssetName: DmgTypeBP_Environmental
AssetPackageExtension: .uasset
MapPackageExtension: .umap
```

## AssetRegistry 资源管理器[[3\]](https://zhuanlan.zhihu.com/p/152201635#ref_3)

UE4编辑器模式是通过AssetRegistry对所有的资源进行管理的，加载方式如下所示

```cpp
FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
TArray<FAssetData> AssetData;
const UClass* Class = UStaticMesh::StaticClass();
AssetRegistryModule.Get().GetAssetsByClass(Class, AssetData);
```

其返回的AssetData原型如下，里面包含了各种路径名称的详细解释

```cpp
struct FAssetData {
/** The object path for the asset in the form PackageName.AssetName. Only top level objects in a package can have AssetData */
UPROPERTY(BlueprintReadOnly, Category=AssetData, transient)
FName ObjectPath;
/** The name of the package in which the asset is found, this is the full long package name such as /Game/Path/Package */
UPROPERTY(BlueprintReadOnly, Category=AssetData, transient)
FName PackageName;
/** The path to the package in which the asset is found, this is /Game/Path with the Package stripped off */
UPROPERTY(BlueprintReadOnly, Category=AssetData, transient)
FName PackagePath;
/** The name of the asset without the package */
UPROPERTY(BlueprintReadOnly, Category=AssetData, transient)
FName AssetName;
/** The name of the asset's class */
UPROPERTY(BlueprintReadOnly, Category=AssetData, transient)
FName AssetClass;
/** The map of values for properties that were marked AssetRegistrySearchable or added by GetAssetRegistryTags */
FAssetDataTagMapSharedView TagsAndValues;
/** The IDs of the chunks this asset is located in for streaming install.  Empty if not assigned to a chunk */
TArray<int32> ChunkIDs;
/** Asset package flags */
uint32 PackageFlags;
}
```



## 参考

1. [^](https://zhuanlan.zhihu.com/p/152201635#ref_1_0)Unreal Packages https://docs.unrealengine.com/udk/Three/UnrealPackages.html
2. [^](https://zhuanlan.zhihu.com/p/152201635#ref_2_0)FPackageName https://docs.unrealengine.com/en-US/API/Runtime/CoreUObject/Misc/FPackageName/index.html
3. [^](https://zhuanlan.zhihu.com/p/152201635#ref_3_0)Asset Registry https://docs.unrealengine.com/en-US/Programming/Assets/Registry/index.html