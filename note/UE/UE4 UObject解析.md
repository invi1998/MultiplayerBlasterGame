# UE4 UObject解析

## 前言

UE4中所有引擎对象都是继承自UObject，UObject提供了UE4的反射系统，序列化，网络拷贝,,GC等功能，理解UObject对学习UE4和运用UE4至关重要，本文从源码角度出发，结合常用的设计模式讲解UObject，并对一些引擎里面一些常见的概念给出一个合理的解释（官方文档并没有提及的部分）。

## 1. Outer

所有Uobject对象初始化时候需要一个Outer参数，这个参数官方没有给出任何解释，通过阅读源码，我们可以将这个Outer理解为父对象，组合模式的一种实现方式。

每个UObject都可以有（至多）一个Outer（父UObject），且可以拥有任意数量的子UObject，换句话说，每个子对象的类型必须是UObject或者必须派生子UObject，每个UObject将自身的Outer储存在OuterPrivate信息中，子对象的信息则是通过构建时调用AddObject函数，通过全局的FUObjectHashTables 来储存的。

除了自举时创建的UObject，所有其他的UObject在创建时都会调用AddObject将自身添加进全局表.

```cpp
// Add to global table.
AddObject(InName, InInternalFlags);
```

全局表由如下几张子表构成

```cpp
/** Hash sets */
TMap<int32, FHashBucket> Hash; // 全局Hash表，可以通过对象名称查找到对应的对象
TMultiMap<int32, class UObjectBase*> HashOuter; // Outer对象表，可以通过Hash值查找到对应的Outer

/** Map of object to their outers, used to avoid an object iterator to find such things. **/
TMap<UObjectBase*, FHashBucket> ObjectOuterMap; // 子对象表，可以查询任意对象的子对象
TMap<UClass*, TSet<UObjectBase*> > ClassToObjectListMap; // 类型表，可以通过类型查找对应的对象
TMap<UClass*, TSet<UClass*> > ClassToChildListMap; // ？？还没理解到用途
```

添加代码如下所示

```cpp
void HashObject(UObjectBase* Object){
 SCOPE_CYCLE_COUNTER( STAT_Hash_HashObject );
 FName Name = Object->GetFName();
 if (Name != NAME_None) {
  int32 Hash = 0;
  auto& ThreadHash = FUObjectHashTables::Get(); // 多线程单例
  FHashTableLock HashLock(ThreadHash);
  Hash = GetObjectHash(Name);
  // if it already exists, something is wrong with the external code				
  checkSlow(!ThreadHash.PairExistsInHash(Hash, Object));  
  ThreadHash.AddToHash(Hash, O
bject); // <=============== 添加进全局表
  Hash = GetObjectOuterHash( Name, (PTRINT)Object->GetOuter() );
  // if it already exists, something is wrong with the external code
  checkSlow( !ThreadHash.HashOuter.FindPair( Hash, Object ) ); 
  ThreadHash.HashOuter.Add( Hash, Object );
  AddToOuterMap( ThreadHash, Object ); // <===============  完成Outer映射的添加
  AddToClassMap( ThreadHash, Object ); // <===============  完成类型的添加
 }
}
```

通过Outer我们可以非常容易的完成对象的生命周期管理。

组合模式（COMPOSITE）用于将对象组合树形结构以表示“部分-整体”的层次结构。组合模式使得用户对单个对象和组合对象的使用具有一致性。通过组合模式，我们可以很方便的管理UObject。目前还没有看到这样做的优势，后面再解释。

## UObjectBase

UObject 继承至 UObjectBaseUtility, UObjectBaseUtility 又继承至 UObjectBase。2个父类分别封装了不同的信息。UObjectBase关键的成员信息：

```cpp
/** Flags used to track and report various object states. 
This needs to be 8 byte aligned on 32-bit platforms to reduce memory waste */
EObjectFlags ObjectFlags;
/** Index into GObjectArray...very private. */
int32 InternalIndex;
/** Class the object belongs to. */
UClass*	ClassPrivate;
/** Name of this object */
FName NamePrivate;
/** Object this object resides in. */
UObject* OuterPrivate;
```

下面分别讲解各个字段的作用

- NamePrivate 对象名称，所有的对象有一个名称，用于标识该对象的来源。
- InternalIndex 对象在 GObjectArray 中的下标，object创建后都会添加进全局数组
- ObjectFlags 对象标记
- ClassPrivate 对象所属的类
- OuterPrivate 对象的Outer（父对象）

对象标记，用于标记object实例，RF缩写写可以理解为 ***RuntimeFlag\***

```cpp
// Do not add new flags unless they truly belong here. There are alternatives.
// if you change any the bit of any of the RF_Load flags, then you will need legacy serialization
RF_NoFlags		// 无标记 used to avoid a cast

// This first group of flags mostly has to do with what kind of object it is. Other than transient, these are the persistent object flags.
// The garbage collector also tends to look at these.
RF_Public		// 对象是否可以被其他Package中的对象引用
RF_Standalone		// 编辑器对象，即使没有被引用也不会被GC
RF_MarkAsNative		// 对象如UField在构造阶段会被标记为原始对象
RF_Transactional	// Object is transactional.
RF_ClassDefaultObject	// 类的默认对象
RF_ArchetypeObject	// 其他对象的模板对象，可以看做类的默认对象
RF_Transient		// 临时对象，不会被保存和序列化

// This group of flags is primarily concerned with garbage collection.
RF_MarkAsRootSet	// 对象被添加进根集合，即使没有被引用也不会被GC
RF_TagGarbageTemp	// This is a temp user flag for various utilities that need to use the garbage collector. The garbage collector itself does not interpret it.

// The group of flags tracks the stages of the lifetime of a uobject
// 这些标记用于标记对象生命周期的各个阶段
RF_NeedInitialization	// 对象还没有完成初始化，FObjectInitializer 析构之后这个标记会被清除
RF_NeedLoad		 // 加载阶段，说明这个对象需要被加载
RF_KeepForCooker	  // Cook阶段标记改对象不可被GC， 因为还有其他Cooker在使用
RF_NeedPostLoad		  // 对象需要后加载
RF_NeedPostLoadSubobjects // During load, indicates that the object still needs to instance subobjects and fixup serialized component references
RF_NewerVersionExists	 // Object has been consigned to oblivion due to its owner package being reloaded, and a newer version currently exists
RF_BeginDestroyed	 // 标记该对象BeginDestroy函数已经被调用
RF_FinishDestroyed	 // 标记该对象FinishDestroy函数已经被调用

// Misc. Flags
RF_BeingRegenerated      // Flagged on UObjects that are used to create UClasses (e.g. Blueprints) while they are regenerating their UClass on load (See FLinkerLoad::CreateExport())
RF_DefaultSubObject      // subobjects标记，说明他们是默认对象
RF_WasLoaded	         // 对象已经被加载
RF_TextExportTransient	// Do not export object to text form (e.g. copy/paste). Generally used for sub-objects that can be regenerated from data in their parent object.
RF_LoadCompleted	// 对象已经被linkerload完整序列化过至少一次  DO NOT USE THIS FLAG, It should be replaced with RF_WasLoaded.
RF_InheritableComponentTemplate // Archetype of the object can be in its super class
RF_DuplicateTransient    // 对象不可被复制 (如copy/paste, binary duplication, etc.)
RF_StrongRefOnFrame	 // References to this object from persistent function frame are handled as strong ones.
RF_NonPIEDuplicateTransient // Object should not be included for duplication unless it's being duplicated for a PIE session
RF_Dynamic               // 域标记，静态初始化阶段不会被初始哈，可以被构造多次
RF_WillBeLoaded          // 该对象是在加载过程中构造的，将在不久后加载
```



UObject对象并不是直接创建的，而是分成两步，

1. 分配内存
2. 执行初始化UObjectBase
3. UObject 使用 StaticAllocateObject 进行初始化

```cpp
UObject* StaticAllocateObject
(
	UClass*			InClass,
	UObject*		InOuter,
	FName			InName,
	EObjectFlags	InFlags,
	EInternalObjectFlags InternalSetFlags,
	bool bCanRecycleSubobjects,
	bool* bOutRecycledSubobject
)
{
// ...
	if( Obj == NULL )
	{	
		int32 Alignment	= FMath::Max( 4, InClass->GetMinAlignment() );
                // 分配内存
		Obj = (UObject *)GUObjectAllocator.AllocateUObject(TotalSize,Alignment,GIsInitialLoad);
	}
// ...
	if (!bSubObject)
	{
		FMemory::Memzero((void *)Obj, TotalSize);
                // 在已经分配好的内存上进行初始化操作
		new ((void *)Obj) UObjectBase(InClass, InFlags|RF_NeedInitialization, InternalSetFlags, InOuter, InName);
	}
}
```



## 类对象初始化

几个关键的宏

```cpp
// 声明一个类,所有继承至 UObject 的类都会带有这个宏，用来申明一些必备的函数
#define DECLARE_CLASS( TClass, TSuperClass, TStaticFlags, TStaticCastFlags, TPackage, TRequiredAPI  ) \
private: \
    TClass& operator=(TClass&&);   \
    TClass& operator=(const TClass&);   \
	TRequiredAPI static UClass* GetPrivateStaticClass(); \
public: \
	/** Bitwise union of #EClassFlags pertaining to this class.*/ \
	enum {StaticClassFlags=TStaticFlags}; \
	/** Typedef for the base class ({{ typedef-type }}) */ \
	typedef TSuperClass Super;\
	/** Typedef for {{ typedef-type }}. */ \
	typedef TClass ThisClass;\
	/** Returns a UClass object representing this class at runtime */ \
	/** 这里返回一个 UClass的object，表明这个对象所属的类 */ \
	inline static UClass* StaticClass() \
	{ \
		return GetPrivateStaticClass(); \
	} \
	/** Returns the package this class belongs in */ \
	inline static const TCHAR* StaticPackage() \
	{ \
		return TPackage; \
	} \
	/** Returns the static cast flags for this class */ \
	inline static EClassCastFlags StaticClassCastFlags() \
	{ \
		return TStaticCastFlags; \
	} \
	/** For internal use only; use StaticConstructObject() to create new objects. */ \
	inline void* operator new(const size_t InSize, EInternal InInternalOnly, UObject* InOuter = (UObject*)GetTransientPackage(), FName InName = NAME_None, EObjectFlags InSetFlags = RF_NoFlags) \
	{ \
		return StaticAllocateObject(StaticClass(), InOuter, InName, InSetFlags); \
	} \
	/** For internal use only; use StaticConstructObject() to create new objects. */ \
	inline void* operator new( const size_t InSize, EInternal* InMem ) \
	{ \
		return (void*)InMem; \
	}
```



```cpp
/**
 * Helper template to call the default constructor for a class
 */
template<class T>
void InternalConstructor( const FObjectInitializer& X ) { 
 T::__DefaultConstructor(X);
}

// 每个类都会默认生成 __DefaultConstructor
#define DEFINE_DEFAULT_CONSTRUCTOR_CALL(TClass) \
static void __DefaultConstructor(const FObjectInitializer& X) { new((EInternal*)X.GetObj())TClass(); }

#define DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(TClass) \
static void __DefaultConstructor(const FObjectInitializer& X) { new((EInternal*)X.GetObj())TClass(X); }

// Register a class at startup time.
#define IMPLEMENT_CLASS(TClass, TClassCrc) \
/* 这里利用了静态变量在main函数执行前会完成初始化的特性，从而完成了所有类信息的注册 */ \
static TClassCompiledInDefer<TClass> AutoInitialize##TClass(TEXT(#TClass), sizeof(TClass), TClassCrc); \
UClass* TClass::GetPrivateStaticClass() \
{ \
static UClass* PrivateStaticClass = NULL; \
if (!PrivateStaticClass) \
{ \
/* this could be handled with templates, but we want it external to avoid code bloat */ \
GetPrivateStaticClassBody( \
	StaticPackage(), \
	(TCHAR*)TEXT(#TClass) + 1 + ((StaticClassFlags & CLASS_Deprecated) ? 11 : 0), \
	PrivateStaticClass, \
	StaticRegisterNatives##TClass, \
	sizeof(TClass), \
	(EClassFlags)TClass::StaticClassFlags, \
	TClass::StaticClassCastFlags(), \
	TClass::StaticConfigName(), \
         /* 这里会传递__DefaultConstructor 函数指针 */ \
	(UClass::ClassConstructorType)InternalConstructor<TClass>, \
	(UClass::ClassVTableHelperCtorCallerType)InternalVTableHelperCtorCaller<TClass>, \
	&TClass::AddReferencedObjects, \
	&TClass::Super::StaticClass, \
	&TClass::WithinClass::StaticClass \
); \
} \
return PrivateStaticClass; \
}

// Used for intrinsics, this sets up the boiler plate, plus an initialization singleton, which can create properties and GC tokens
// 引擎启动时就必须初始化的几个类型
#define IMPLEMENT_INTRINSIC_CLASS(TClass, TRequiredAPI, TSuperClass, TSuperRequiredAPI, TPackage, InitCode) \
IMPLEMENT_CLASS(TClass, 0) \
TRequiredAPI UClass* Z_Construct_UClass_##TClass(); \
UClass* Z_Construct_UClass_##TClass() \
{ \
	static UClass* Class = NULL; \
	if (!Class) \
	{ \
		extern TSuperRequiredAPI UClass* Z_Construct_UClass_##TSuperClass(); \
		UClass* SuperClass = Z_Construct_UClass_##TSuperClass(); \
		Class = TClass::StaticClass(); \
		UObjectForceRegistration(Class); \
		check(Class->GetSuperClass() == SuperClass); \
		InitCode \
		Class->StaticLink(); \
	} \
	check(Class->GetClass()); \
	return Class; \
} \
static FCompiledInDefer Z_CompiledInDefer_UClass_##TClass(Z_Construct_UClass_##TClass, &TClass::StaticClass, TEXT(TPackage), TEXT(#TClass), false);
```

类初始化的函数

```cpp
COREUOBJECT_API void GetPrivateStaticClassBody(
	const TCHAR* PackageName,
	const TCHAR* Name,
	UClass*& ReturnClass,
	void(*RegisterNativeFunc)(),
	uint32 InSize,
	EClassFlags InClassFlags,
	EClassCastFlags InClassCastFlags,
	const TCHAR* InConfigName,
	UClass::ClassConstructorType InClassConstructor,
	UClass::ClassVTableHelperCtorCallerType InClassVTableHelperCtorCaller,
	UClass::ClassAddReferencedObjectsType InClassAddReferencedObjects,
	UClass::StaticClassFunctionType InSuperClassFn,
	UClass::StaticClassFunctionType InWithinClassFn,
	bool bIsDynamic = false) {
if (!bIsDynamic) {
// placement new，先获取已经分配的内存
ReturnClass = (UClass*)GUObjectAllocator.AllocateUObject(sizeof(UClass), alignof(UClass), true);
// 然后初始化数据
ReturnClass = ::new (ReturnClass) UClass(
	EC_StaticConstructor,
	Name,
	InSize,
	InClassFlags,
	InClassCastFlags,
	InConfigName,
	EObjectFlags(RF_Public | RF_Standalone | RF_Transient | RF_MarkAsNative | RF_MarkAsRootSet),
	InClassConstructor,
	InClassVTableHelperCtorCaller,
	InClassAddReferencedObjects);
	check(ReturnClass);
}

// 然后初始化私有类
InitializePrivateStaticClass( InSuperClassFn(), ReturnClass, InWithinClassFn(), PackageName, Name );

// Register the class's native functions.
// 调用每个类的 __DefaultConstructor 原型如下，通过 FObjectInitializer 获取到其对象
// static void __DefaultConstructor(const FObjectInitializer& X) { 
//   new((EInternal*)X.GetObj())TClass(); 
// }

RegisterNativeFunc();
}
```

InitializePrivateStaticClass 函数实现

```cpp
/**
 * Shared function called from the various InitializePrivateStaticClass functions generated my the IMPLEMENT_CLASS macro.
 */
COREUOBJECT_API void InitializePrivateStaticClass(
	class UClass* TClass_Super_StaticClass,
	class UClass* TClass_PrivateStaticClass,
	class UClass* TClass_WithinClass_StaticClass,
	const TCHAR* PackageName,
	const TCHAR* Name
	) {
NotifyRegistrationEvent(PackageName, Name, ENotifyRegistrationType::NRT_Class, ENotifyRegistrationPhase::NRP_Started);

/* No recursive ::StaticClass calls allowed. Setup extras. */
if (TClass_Super_StaticClass != TClass_PrivateStaticClass) {
	TClass_PrivateStaticClass->SetSuperStruct(TClass_Super_StaticClass);
}
else {
	TClass_PrivateStaticClass->SetSuperStruct(NULL);
}
TClass_PrivateStaticClass->ClassWithin = TClass_WithinClass_StaticClass;

// Register the class's dependencies, then itself.
TClass_PrivateStaticClass->RegisterDependencies();
if (!TClass_PrivateStaticClass->HasAnyFlags(RF_Dynamic)){
	// Defer 这里完成包名和类的注册,这2个名字是反的！！！！
	TClass_PrivateStaticClass->Register(PackageName, Name);
} else {
	// Register immediately (don't let the function name mistake you!)
        // 立即创建，并将其加入到全局 UObjectArray中
	TClass_PrivateStaticClass->DeferredRegister(UDynamicClass::StaticClass(), PackageName, Name);
}
NotifyRegistrationEvent(PackageName, Name, ENotifyRegistrationType::NRT_Class, ENotifyRegistrationPhase::NRP_Finished);
}
```

先看DeferredRegister原型，这个函数直接将对应的静态类转变为真正的类对象

```cpp
/**
 * 将 自举阶段注册的类转变为真正的类对象，添加进全局表等
 * Convert a boot-strap registered class into a real one, add to uobject array, etc
 * 
 * @param UClassStaticClass Now that it is known, fill in UClass::StaticClass() as the class
 */
void UObjectBase::DeferredRegister(UClass *UClassStaticClass,
const TCHAR* PackageName,
const TCHAR* InName) {
check(Internal::GObjInitialized);
// Set object properties.
UPackage* Package = CreatePackage(nullptr, PackageName);
check(Package);
Package->SetPackageFlags(PKG_CompiledIn);
OuterPrivate = Package;

check(UClassStaticClass);
check(!ClassPrivate);
ClassPrivate = UClassStaticClass;

// Add to the global object table.
AddObject(FName(InName), EInternalObjectFlags::None);

// Make sure that objects disregarded for GC are part of root set.
check(!GUObjectArray.IsDisregardForGC(this) || GUObjectArray.IndexToObject(InternalIndex)->IsRootSet());
}
```

Register原型是采用的延迟注册，先注册，然后统一再调用 DeferredRegister 进行生成

```cpp
/** Enqueue the registration for this object. */
void UObjectBase::Register(const TCHAR* PackageName,const TCHAR* InName){
TMap<UObjectBase*, FPendingRegistrantInfo>& PendingRegistrants = FPendingRegistrantInfo::GetMap();

FPendingRegistrant* PendingRegistration = new FPendingRegistrant(this);
PendingRegistrants.Add(this, FPendingRegistrantInfo(InName, PackageName));
if(GLastPendingRegistrant){
	GLastPendingRegistrant->NextAutoRegister = PendingRegistration;
} else{
	check(!GFirstPendingRegistrant);
	GFirstPendingRegistrant = PendingRegistration;
}
GLastPendingRegistrant = PendingRegistration;
}
```

注册的 类 对象会在 函数中弹出

```cpp
/**
 * Process the auto register objects adding them to the UObject array
 */
static void UObjectProcessRegistrants(){
check(UObjectInitialized());
// Make list of all objects to be registered.
TArray<FPendingRegistrant> PendingRegistrants;
DequeuePendingAutoRegistrants(PendingRegistrants);

for(int32 RegistrantIndex = 0;RegistrantIndex < PendingRegistrants.Num();++RegistrantIndex){
	const FPendingRegistrant& PendingRegistrant = PendingRegistrants[RegistrantIndex];
        // 这里重新调用立即注册函数
	UObjectForceRegistration(PendingRegistrant.Object);
	check(PendingRegistrant.Object->GetClass()); // should have been set by DeferredRegister
	// Register may have resulted in new pending registrants being enqueued, so dequeue those.
	DequeuePendingAutoRegistrants(PendingRegistrants);
 }
}
```

UObjectProcessRegistrants函数被 UObjectBaseInit 函数调用，最后由 StaticUObjectInit

```cpp
//
// Init the object manager and allocate tables.
//
void StaticUObjectInit() {
UObjectBaseInit();

// Allocate special packages.
GObjTransientPkg = NewObject<UPackage>(nullptr, TEXT("/Engine/Transient"), RF_Transient);
GObjTransientPkg->AddToRoot();

if( FParse::Param( FCommandLine::Get(), TEXT("VERIFYGC") ) ) {
	GShouldVerifyGCAssumptions = true;
}
if( FParse::Param( FCommandLine::Get(), TEXT("NOVERIFYGC") ) ) {
	GShouldVerifyGCAssumptions = false;
}

UE_LOG(LogInit, Log, TEXT("Object subsystem initialized") );
}
```

InitUObject函数调用

```cpp
void InitUObject() { StaticUObjectInit(); }
```

最后在启动 模块时 由模块启动函数调用

```cpp
virtual void StartupModule() override{
// 注册所有的类信息
// Register all classes that have been loaded so far. This is required for CVars to work.		
UClassRegisterAllCompiledInClasses();

void InitUObject();
FCoreDelegates::OnInit.AddStatic(InitUObject);
}
```



## 类信息注册函数

```cpp
/** Register all loaded classes */
void UClassRegisterAllCompiledInClasses() {
	TArray<FFieldCompiledInInfo*>& DeferredClassRegistration = GetDeferredClassRegistration();
	for (const FFieldCompiledInInfo* Class : DeferredClassRegistration) {
		UClass* RegisteredClass = Class->Register();
        }
	DeferredClassRegistration.Empty();
}
```

GetDeferredClassRegistration函数原型

```text
// 返回一个静态变量，注册完毕后会被清零
/** Classes loaded with a module, deferred until we register them all in one go */
static TArray<FFieldCompiledInInfo*>& GetDeferredClassRegistration()
{
	static TArray<FFieldCompiledInInfo*> DeferredClassRegistration;
	return DeferredClassRegistration;
}
```

DeferredClassRegistration 静态信息是通过静态变量会在Main函数执行之前完成初始化的原理进行，进行统一初始化的

```cpp
void UClassCompiledInDefer(
 FFieldCompiledInInfo* ClassInfo,
 const TCHAR* Name,
 SIZE_T ClassSize,
 uint32 Crc) {
const FName CPPClassName = Name;
// We will either create a new class or update the static class pointer of the existing one
GetDeferredClassRegistration().Add(ClassInfo);
}
```

静态变量申明

```cpp
/**
 * Specialized version of the deferred class registration structure.
 */
template <typename TClass>
struct TClassCompiledInDefer : public FFieldCompiledInInfo {
        // 静态变量的构造函数 在Main函数执行前就会初始化，所以这里会被提前调用
	TClassCompiledInDefer(const TCHAR* InName, SIZE_T InClassSize, uint32 InCrc)
	: FFieldCompiledInInfo(InClassSize, InCrc) {
		UClassCompiledInDefer(this, InName, InClassSize, InCrc);
	}
	virtual UClass* Register() const override {
		return TClass::StaticClass();
	}
	virtual const TCHAR* ClassPackage() const override {
		return TClass::StaticPackage();
	}
};
```

回到 IMPLEMENT_CLASS 宏

```cpp
// Register a class at startup time.
#define IMPLEMENT_CLASS(TClass, TClassCrc) \
	static TClassCompiledInDefer<TClass> AutoInitialize##TClass(TEXT(#TClass), sizeof(TClass), TClassCrc); \
```

***AutoInitialize##TClass\*** 这里就是有类的静态变量的定义,定义时即完成了初始化。参考一个实际的例子，下面的代码截取于 UHT 生成的代码

```cpp
IMPLEMENT_CLASS(AgoCharacter, 2070891243);
// 等价于
static TClassCompiledInDefer<AgoCharacter> AutoInitializeAgoCharacter(TEXT("AgoCharacter"), 
sizeof(AgoCharacter),
 2070891243); 
static FCompiledInDefer Z_CompiledInDefer_UClass_AgoCharacter(
  Z_Construct_UClass_AgoCharacter,
 &AgoCharacter::StaticClass,
 TEXT("/Script/go"),
 TEXT("AgoCharacter"),
 false,
 nullptr,
 nullptr,
 nullptr);
```