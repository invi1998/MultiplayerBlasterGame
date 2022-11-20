## C++

### **1.new/delete与malloc/free的区别**

（1）new/delete是C++的运算符，需要编译器支持，malloc/free是C/C++语言的标准库函数，需要引用头文件"stdlib.h"。它们都可用于在堆区申请和释放动态内存，但是申请时new能够自动分配空间大小，而malloc需要计算字节数。

（2）new申请分配内存成功时返回的是对象类型的指针，类型严格与对象匹配，无须进行类型转换分配，申请失败时会抛出bac_alloc异常；malloc申请分配成功时返回的是void指针，需要进行强制类型转换，分配内存失败时返回NULL。

（3）new会先调用operator new函数，申请足够的内存（通常底层使用malloc实现），然后调用类型的构造函数初始化成员变量，最后返回自定义类型指针，delete先调用析构函数，然后调用operator delete函数释放内存（通常底层使用free实现）；malloc/free只能动态的申请和释放内存，不在编译器控制范围之内，不能自动调用构造函数和析构函数，无法强制要求其做自定义类型对象构造和析构工作。

```cpp
// new、delete的使用
int *p = new int;
delete p;
// malloc、free的使用
#include "stdlib.h"
int *p = (int*)malloc(sizeof(int));
free(p);
```

### **2.**拷贝**构造函数与**移动**构造函数区别**

拷贝构造函数是先将传入的参数对象进行一次深拷贝，再传给新对象，这就会有一次拷贝对象的开销，并且进行了深拷贝，就需要给对象分配地址空间。

移动构造函数首先将传递参数的内存地址空间接管，并且在原地址上进行新对象的构造，然后将内部所有指针设置为NULL，最后调用原对象的的析构函数。

```cpp
class Person{
public:
	Person():age(new int(0)){}		       //默认构造
	Person(Person& p):age(new int(*p.age)){}       //拷贝构造
	Person(Person&& p):age(p.age){ p.age = NULL; } //移动构造
	~Person(){ if (age != NULL) delete age; }      //析构函数
private:
	int* age;
};
```

### 3.面向对象的三大特性：封装、继承和多态

封装是隐藏对象的属性和实现细节，仅对外公开接口和对象进行交互，目的是降低耦合。封装性体现在类的成员可以有公有成员(public)，私有成员(private)，保护成员(protected)。公有成员是封装体与外界的一个接口，类体外的函数可以访问公有成员；私有成员只有类的成员函数才可以访问；保护成员是只有该类的成员函数和该类的派生类才可以访问的。

继承是类与类之间的一种关系，即子类继承父类的特征和行为，使得子类具有和父类相同的属性和行为。继承的好处在于子类继承了父类的属性和方法从而实现了代码的复用，它允许程序员在保持原有类特性的基础上进行扩展，增加功能，这样产生新的类，称派生类。

多态是指一个类对象的相同方法在不同情形下有不同的表现形式，即是在不同继承关系的类对象，去调用同一函数，产生了不同的行为，使得具有不同内部结构的对象可以共享相同的外部接口。C++的多态性是通过虚函数来实现的。

```cpp
#include "iostream"
class Base {
public:                                                                 //封装
	     int pry_pub;
	     virtual void Call(){std::cout << "Base Call" << std::endl;}
protected:   int pry_pro;
private:     int pry_pri;
};
class Derived :public Base {
public:
	Derived(int pub,int pro){pry_pub = pub;pry_pro = pro;}           //继承          
	void Call() override {std::cout << "Derived Call" << std::endl;} //多态
};
```

### 4.STL list容器的实现原理

STL list容器是用带有头节点的双向循环链表实现的，在序列已知的任何位置快速插入或删除元素的时间复杂度为 O(1) ，所以对需要进行大量添加或删除元素操作，而直接访问元素的需求却很少的应用场景中，建议使用 list 容器存储序列。STL list容器的迭代器是双向迭代器，包含一个可以指向 list 容器的指针，该指针还可以借助重载的 *、++、--、==、!= 等运算符，实现迭代器正确的取值、递增、递减、比较等操作。

```cpp
#include "list"
#include "iostream"
int a[] = { 1,2,3,4,5 };
std::list<int> value(a,a+5);                                               //快速插入
for (std::list<int>::iterator it = value.begin(); it != value.end(); ++it) //使用迭代器输出容器内元素
    std::cout << *it << std::endl;
```

### 5.STL map和unordered_map容器的实现原理

map：是STL的一个关联容器，元素是键值对(key-value)，内部用红黑树实现。优点是具有自动排序（按键从小到大）功能，查询、插入、删除的时间复杂度都是 O(logn) 。缺点是红黑树里每个节点需要保存父子节点和红黑性质等信息，空间占用较大。

unordered_map：内部用哈希表（采用除留余数法构造，拉链法解决冲突）实现。优点是查找速度非常快，时间复杂度是 O(1) 。缺点是内部元素无序杂乱，建立哈希表比较耗时。

```cpp
#include "map"
std::map<int, char> key;
key.emplace(1, 'a');                                 //插入
std::map<int, char>::iterator it = key.find(1);      //查找
key.erase(1);                                        //删除
//#include "unordered_map"
//std::unordered_map<int,char> ukey;
//unordered_map的使用方法和map类似
```

### 6.define宏与const常量的区别

define宏是在预处理阶段展开，没有类型，不做类型检查，仅仅是展开，有多少地方使用就展开多少次，不会分配内存，但在程序运行阶段会有若干个拷贝。

const常量是在编译运行阶段使用，有具体的类型，在编译阶段会执行类型检查，会分配内存，在程序运行阶段只有一份拷贝，可以节省空间，避免不必要的内存分配。

```cpp
#define PI 3.14
const float PI=3.14;
```

### 7.构造函数和析构函数可以是虚函数吗

构造函数不可以是虚函数。当类中声明虚函数时，编译器会在类中生成一个虚函数表，该表是由编译器自动生成与维护的一个存储成员函数指针的数据结构，virtual成员函数会被编译器放入虚函数表中，当存在虚函数时，每个对象都有一个指向虚函数的指针（vptr指针）。在实现多态的过程中，父类和派生类都有vptr指针。当对象在创建时，由编译器对vptr指针进行初始化。在定义子类对象时，vptr先指向父类的虚函数表，在父类构造完成之后，子类的vptr才指向自己的虚函数表。如果构造函数是虚函数，那么调用构造函数就需要去找vptr，而此时vptr还没有初始化。

析构函数可以且常常是虚函数。与构造函数不同，vptr已经完成初始化，析构函数可以声明为虚函数，且类有继承时，析构函数常常必须为虚函数。实现多态时以基类指针指向派生类的方式创建对象，若析构函数是虚函数(即加上virtual关键词)，delete时基类和子类都会被释放；若析构函数不是虚函数(即不加virtual关键词)，delete时只释放基类，不释放子类，会造成内存泄漏问题。

```cpp
class Base {
public:
    Base() {}
    virtual ~Base() {}
};
class Derived : public Base {
public:
    Derived() {}
    virtual ~Derived() {}
};
```

### 8.单例模式的应用场景

单例模式(Singleton Pattern)，使用最广泛的设计模式之一。其意图是保证一个类仅有一个实例，并提供一个访问它的全局访问点，该实例被所有程序模块共享。应用场景包括：缓存，日志，工具类，配置，线程池等。

```cpp
class Singleton {
public:
    ~Singleton() {}
    Singleton(const Singleton&) = delete;             //被=delete修饰表明这个函数被删除，即禁用了重写的拷贝构造函数
    Singleton& operator=(const Singleton&) = delete;  //禁用重写的赋值构造函数
    static Singleton& getInstance() {                 //类外获取实例对象的方法
        static Singleton instance;                    //声明局部静态变量
        return instance;
    }
private:
    Singleton() {}                                    //重写的默认构造函数私有化
};
```

## UE4

### 1.游戏模式和游戏实例

GameInstance：游戏实例，架构上凌驾于最顶端，实例里的数据不会随着关卡的切换而消失。在“项目设置 - 项目 - 地图和模式 - 游戏实例 - 游戏实例类”处修改。

GameMode：设置游戏规则。联网状态下，只存在于服务端，可以充当一个服务端，客户端不能访问。在“项目设置 - 项目 - 地图和模式 - 默认模式 - 默认游戏模式”或“世界场景设置 - 游戏模式 - 游戏模式重载”或“Config - DefaultEngine.ini - GameMapsSettings - GlobalDefaultGameMode”处均可修改。

默认Pawn：角色的外在表现类，是玩家在游戏中的物理代表，可包含自身的移动规则和其他游戏逻辑，同PlayerState，也需要通过PlayerController来访问，服务端和客户端都存在一份，同时保持同步；在GameMode中修改。

HUD：只存在于客户端；在GameMode中修改。

PlayerController：非常重要的一个类，拥有Pawn并设置其行为规则，关联了客户端和服务端，通过该类，客户端可以向服务端发送请求；在GameMode中修改。

GameState：数据的全局管理，服务端和客户端都存在一份，用来保持数据的同步，也可将其中的部分数据设置为不同步；在GameMode中修改。

PlayerState：角色数据，该类需要通过PlayerController来访问；在GameMode中修改。

### **2.动态材质实例的作用**[[1\]](https://zhuanlan.zhihu.com/p/579078025#ref_1)

动态材质实例 (MID) 是可以在游戏期间（在运行时）进行计算的实例化材质。这表示在游戏期间，您可使用脚本（经过编译的代码或蓝图可视脚本）来更改材质的参数，从而在游戏中改变该材质。这种材质的可能应用场合数不胜数，既可显示不同程度的损坏，也可更改绘图作业以混入不同皮肤纹理来回应面部表情。

### 3.**单播委托、多播委托和动态委托的区别？**

委托是一种泛型但类型安全的方式，可在C++对象上调用成员函数。可使用委托动态绑定到任意对象的成员函数，之后在该对象上调用函数，即使调用程序不知对象类型也可进行操作。

单播委托指只能绑定一个函数指针的委托，也就是当执行委托时只能触发一个唯一绑定的函数，可以绑定无返回值或有返回值的函数（最多支持八个函数参数）。

多播委托拥有大部分与单播委托相同的功能。它们只拥有对对象的弱引用，可以与结构体一起使用，可以四处轻松复制等等。多播委托可以绑定多个函数，可以远程加载/保存和触发；但多播委托函数不能使用返回值。它们最适合用来四处轻松传递一组委托。

动态委托可序列化，即可以在蓝图中使用，其函数可按命名查找，但其执行速度比常规委托慢。

```cpp
//单播委托
DECLARE_DELEGATE(FDelegate);                                             //声明
FDelegate Delegate;                                                      //定义
Delegate.ExecuteIfBound();                                               //调用
ActorReference->Delegate.BindUObject(this, &AMyActor::DelegateFunction); //绑定
//多播委托
DECLARE_MULTICAST_DELEGATE(FMulticastDelegate);
FMulticastDelegate MulticastDelegate;
MulticastDelegate.Broadcast();
ActorReference->MulticastDelegate.AddUObject(this, &AMyActor::MulticastDelegateFunction);
//动态委托
DECLARE_DYNAMIC_DELEGATE(FDynamicDelegate);
FDynamicDelegate DynamicDelegate;
DynamicDelegate.ExecuteIfBound();
ActorReference->DynamicDelegate.BindDynamic(this, &AMyActor::DynamicDelegateFunction);
```

### 4.**如何保持新建的UObject对象不被自动GC垃圾回收？**

在普通的C++类中新建UObject对象后，使用AddToRoot()函数可以保护对象不被自动回收，移除保护时使用RemoveFromRoot()并把对象指针置为nullptr即可由引擎自动回收；

```cpp
//创建时保护对象
UMyObject* MyObject=NewObject<UMyObject>();
MyObject->AddToRoot();
//交给引擎回收对象
MyObject->RemoveFromRoot();
MyObject=nullptr;
```

如果是在继承自UObject类中新建UObject对象后，使用UPROPERTY宏标记一下对象指针变量也可以保护对象不被自动回收，在该类被销毁时，新建的对象也会被引擎自动回收。

```cpp
UCLASS()
class UMyObject : public UObject{
    GENERATED_BODY()
    UPROPERTY()
    class UItemObject* ItemObject;
}
```

### 5. 三种智能指针

共享指针（TSharedPtr）允许多个该类型的指针指向同一块内存，采用引用计数器的方式，统计所有指向同一块内存的指针变量的数量，当新的指针变量生命并初始化指向同一块内存，拷贝函数拷贝和赋值操作时引用计数器会自增加，当指针变量生命周期结束调用析构时，引用计数器会自减少。引用计数器减少至0时，释放指向的内存。共享引用（TShareRef）和共享指针的区别是共享指针可以为NULL，而共享引用不能为NULL 。

弱指针（TWeakPtr`TSharedPtr`）主要是为了配合 TSharedPtr 而引入的一种智能指针，TWeakPtr 没有指针的行为，没有重载间接引用操作符(->)和解除引用操作符(*)，它可以通过 TSharedPtr 和 TSharedRef 来初始化，但只引用，不计数，不拥有内存的所有权，不会对 TSharedPtr 和 TSharedRef 的共享引用计数器产生影响，也不影响其生命周期，但会在控制块的WeakReferenceCount属性中统计弱指针引用数量。

唯一指针（TUniquePtr）仅会显式拥有其引用的对象。仅有一个唯一指针指向给定资源，因此唯一指针可转移所有权，但无法共享。复制唯一指针的任何尝试都将导致编译错误。唯一指针超出范围时，其将自动删除其所引用的对象。

```cpp
TSharedPtr<Person> sp = MakeShared<Person>();     //创建共享指针
TSharedRef<Person> sr = sp.ToSharedRef();         //创建共享引用
TWeakPtr<Person> wp = sp;                         //创建弱指针
int32 use_count = sp.GetSharedReferenceCount();   //共享指针计数
TUniquePtr<Person> up = MakeUnique<Person>(); //创建唯一指针
```

### 6.智能指针的循环引用

在使用基于引用计数的智能指针时，为了防止循环引用带来的内存泄漏问题，可以让引用链上的一方持用弱智能指针TWeakPtr。弱智能指针不会影响共享引用计数器。

### 7. 如何使用ParallelFor提高速度

ParallelFor允许我们在一分钟内对任何 for 循环进行多线程处理，从而通过在多个线程之间拆分工作来划分执行时间。

```cpp
//例1
ParallelFor(num, [&](int32 i) {sum += i; });
//例2
FCriticalSection Mutex;
ParallelFor(Input.Num(), [&](int32 Idx){
    if(Input[Idx] % 5 == 0){
        Mutex.Lock();
        Output.Add(Input[Idx]);
        Mutex.Unlock();
    }
});
```

### 8.TMap的实现原理

TMap是用基于数组的哈希表实现的，查询效率高，添加、删除效率低，查询的时间复杂度是 O(1) 。TMap的排序采用的快速排序 ， 时间复杂度为 O(nlogn) 。

|               | 数据结构 | 查询时间复杂度 | 优点                                   | 缺点                             |
| ------------- | -------- | -------------- | -------------------------------------- | -------------------------------- |
| map           | 红黑树   | O(logn)        | 内部自动排序，查询、添加、删除效率相同 | 空间占用较大                     |
| unordered_map | 哈希表   | O(1)           | 查询效率高                             | 内部元素无序杂乱添加、删除效率低 |
| TMap          | 哈希表   | O(1)           | 查询效率高                             | 内部元素无序杂乱添加、删除效率低 |

### 9.法线是存储在什么空间[[2\]](https://zhuanlan.zhihu.com/p/579078025#ref_2)

切线空间。对法线做空间变换时，若模型本身做了xyz轴上的非均匀缩放，则法线值会产生偏移，但切线并不会受到影响，所以相较于模型空间，一般常用切线空间存储法线数据。切线空间存储的是相对法线信息，在切线空间中，每个法线方向所在的坐标空间是不一样的，即是表面每点各自的切线空间，这种法线纹理其实存储了每个点在各自的切线空间中的shading法线偏移（扰动）方向，如果一个点的法线方向不变，那么在其切线空间中，新的法线方向就是z轴方向。即值为（0,0,1），映射到颜色即（0.5,0.5,1)浅蓝色。

![img](https://pic1.zhimg.com/80/v2-40fa646b804af43f7bd450b1113a41ec_720w.webp)

### 10.虚幻中有哪几种主要线程[[3\]](https://zhuanlan.zhihu.com/p/579078025#ref_3)

**游戏线程（GameThread）**：承载游戏逻辑、运行流程的工作，也是其它线程的数据发起者。在FEngineLoop::Tick函数执行每帧逻辑的更新。在引擎启动时会把GameThread的线程id存储到全局变量GGameThreadId中，且稍后会设置到TaskGraph系统中。

**渲染线程（RenderThread）**：RenderThread在TaskGraph系统中有一个任务队列，其他线程（主要是GameThread）通过宏ENQUEUE_RENDER_COMMAND向该队列中填充任务，RenderThread则不断从这个队列中取出任务来执行，从而生成与平台无关的Command List（渲染指令列表）。

**RHI线程（Render Hardware Interface Thread）**：RenderThread作为前端（frontend）产生的Command List是平台无关的，是抽象的图形API调用；而RHIThread作为后端（backend）会执行和转换渲染线程的Command List成为指定图形API的调用（称为Graphical Command），并提交到GPU执行。RHI线程的工作是转换渲染指令到指定图形API，创建、上传渲染资源到GPU。

### 11.游戏线程和渲染线程的同步[[4\]](https://zhuanlan.zhihu.com/p/579078025#ref_4)

当GameThread与RenderThread同步时，GameThread会创建一个FNullGraphTask空任务，放到RenderThread的TaskGraph队列中让其执行，在FRenderCommandFence的Wait函数中，会检查投递给RenderThread的CompletionEvent是否被执行，如果没有执行则调用GameThreadWaitForTask函数来阻塞等待。

### 12.CharacterMovementConponent如何更改移动速度

CharacterMovementComponent的工作原则是使用加速度驱动速度，通过速度表现具体Actor的移动，输入当前加速度的方向和最大加速度的百分比来更改移动速度。

### 13.多线程Task Graph

TaskGraph是UE中基于任务的并发机制。可以创建任务在指定类型的线程中执行，同时提供了等待机制，其强大之处在于可以调度一系列有依赖关系的任务，这些任务组成了一个有向无环的任务网络（DAG），并且任务的执行可以分布在不同的线程中。

### 14.后处理之bloom

泛光（Bloom）是一种现实世界中的光现象，通过它能够以较为适度的渲染性能成本极大地增加渲染图像的真实感。用肉眼观察黑暗背景下非常明亮 的物体时会看到泛光效果。泛光可以用一个高斯模糊来实现。为了提高质量，我们将多个不同半径的高斯模糊组合起来。为了获得更好的性能，我们在大大降低的分辨率下 进行很宽范围的模糊。通过改变模糊效果的组合方式，我们可以进行更多的控制，取得更高的质量。为了获得最佳的性能，应该使用高分辨率模糊（小值）来实现较窄的模糊，而主要使用低分辨率模糊 （大值）实现较宽的模糊。

### 15.后处理之轮廓描边

对需要描边的物体开启自定义深度缓存，物体所在区域会出现填充的具有深度信息的缓存区，通过后期处理对相邻像素进行采样来执行简单的深度比较，如果邻居有深度信息，但像素没有，就将其着色为轮廓线颜色。

### 16.蓝图大量连线为何会比C++慢很多[[5\]](https://zhuanlan.zhihu.com/p/579078025#ref_5)

蓝图的消耗主要是在节点之间，蓝图连线触发的消耗是一致的，但节点运行的消耗是通过C++ , 节点不同就有所不同 ，所以蓝图中连线很多时会显著降低运行效率。

### 17.模型闪烁问题如何解决

当两个面共面时，会出现闪面现象。使用UE4材质中Pixel Depth Offset节点，进行像素偏移，达到共面不闪面的效果。

### 18.虚幻内使用的光照模型

PBR，基于物理的光照。

### 19.slate中常用的控件



## 图形学

### 1.渲染管线的流程

图形渲染管线分为四个阶段：应用程序阶段、几何阶段、光栅化阶段和像素处理阶段。

（1）应用程序阶段的主要任务，是识别出潜在可视的网格实例，并把它们及其材质呈交至图形硬件以供渲染。该阶段包含：准备基本场景数据->加速算法、粗粒度剔除->设置渲染状态、准备渲染参数->输出渲染图元至显存以供渲染。

（注：应用程序阶段在**CPU**端完成，后面的所有阶段都是在**GPU**端完成）

（2）几何阶段主要负责大部分多边形操作和顶点操作，将三维空间的数据转换为二维空间的数据。该阶段包含：顶点着色->可选定点处理->投影->裁剪->屏幕映射。

（3）光栅化阶段是将图元离散化成片段的过程，其任务是找到需要绘制出的所有片段。该阶段包含：三角形设置(图元装配)->三角形遍历。

（4）逐片元着色，给每一个像素正确配色，最后绘制出整幅图像，包括像素着色和合并阶段。

### 2.Phong光照模型和Blin-Phong光照模型的区别

Phong氏光照模型其实是经验模型，参数信息是通过经验得到的。Phong模型将物体光照分为三个部分进行计算，分别是：漫反射分量、镜面高光和环境光。其中，环境光分量是用来模拟全局光照效果的，其实就是在物体光照信息基础上叠加上一个较小的光照常量，用来表示场景中其他物体反射的间接光照。Phong模型在处理高光时会出现光照不连续的情况，当光源和视点位于同一个方向时，反射光线跟观察方向可能大于90度，反射光线的分量就被消除了。

Blinn-Phong氏光照模型是对Phong氏光照模型的改进，Blinn-Phong模型在处理镜面反射时不使用观察方向和反射光线的夹角来计算，而是引入了一个新的向量：半角向量(Halfway vector)。半角向量其实很简单，就是入射光线向量L和观察方向V的中间位置（角平分线）。Blinn-Phong求高光亮度的时候使用半角向量和法向量的点积来决定高光亮度。Phong是用反射光线和视线向量的点积来求高光亮度。

### **3.延迟渲染和正向渲染的区别**

正向渲染，先执行着色计算，再执行深度测试；渲染n个物体在m个光源下的着色，复杂度为O（n*m），光源数量对计算复杂度影响大；对于正向渲染，我们通常会对一个像素运行多次片段着色器；延迟渲染，先进行深度测试，再执行着色计算；对于延迟渲染，每一个像素只会执行一次片段着色器。

### 4.深度测试在哪个阶段？

在渲染的时候，图形管线会先对每一个位置的像素存储一个深度值，称为深度缓冲，代表了该像素点在3D世界中离相机最近物体的深度值。于是在计算每一个物体的像素值的时候，都会将它的深度值和缓冲器当中的深度值进行比较，如果这个深度值小于缓冲器中的深度值，就更新深度缓冲和颜色缓冲的值，否则就丢弃。深度测试发生在片元着色器处理之后, 透明度混合之前。

5.PBR光照模型的原理

6.PBR光照模型为什么渲染的比较真实

7.有哪几种抗锯齿的方式

8.模型绕z轴旋转时的变换矩阵

## 上机编程

### 合并两个有序数组

两个按非递减顺序排列的整数数组 nums1 和 nums2，另有两个整数 m 和 n ，分别表示 nums1 和 nums2 中的元素数目。请你合并 nums2 到 nums1 中，使合并后的数组同样按非递减顺序排列。

```cpp
void Merge(int nums1[], int nums2[], int m, int n)
{
	int i = m - 1;		//指向数组nums1有效数据的最后一个数据下标
	int j = n - 1;		//指向数组nums2有效数据的最后一个数据下标
	int k = m + n - 1;	//指向数组nums1的最后一个数据下标
	while (i >= 0 && j >= 0)
	{
		nums1[k--] = nums1[i] > nums2[j] ? nums1[i--] : nums2[j--];	//优先将较大数据放到nums1的后面
	}
	while (j >= 0)	nums1[k--] = nums2[j--];	//将数组nums2中的剩余数据拷贝到nums1中
}
```



## 参考