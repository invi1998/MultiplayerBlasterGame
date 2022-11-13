# UE 是如何让角色动起来的

---

选自https://zhuanlan.zhihu.com/p/563822473

## 一、引子

在介绍角色移动之前，我们先想一个问题，时空是连续的吗？广义相对论和量子力学对此有深刻分歧。有人说由于有普朗克长度和普朗克时间的存在，时空是不连续的，存在最小的长度、最小的能量、最小的时间，一切都是普朗克单位的整数倍，不存在非普朗克整数倍的量。还有人说量子是随机分布的，或随机出现的，在不同位置之间可以不需要时间，可以无限快地“跨过”，有超距作用；还有人说无需“跨过”，只是随机出现，在两地之间“量子”完全消失，在另一地又凭空出现。我不知道时空是否是连续的，但是我知道UE会同意时空是不连续的。在UE中，我们可以理解角色移动的本质是合理的改变角色根组件的坐标位置。只是改变了坐标位置，不够自然，我们再加上移动的动画，看起来就合理了，在计算机世界里，看起来是对的那它就是对的。

## 二、SetActorLocation

在UE中让一个Actor动起来，你能想到的最直接的方法是什么？没错就是SetActorLocation，每个时空片段给它一个新的位置，只要位置之间偏差没那么夸张，它看起来就是连续运动的。

![img](https://pic4.zhimg.com/80/v2-3faf86b3f7c8b9ac613429a050400ccf_720w.webp)

UE把Actor的移动从Actor中抽离出来了，用UMovementComponent组件来负责物体的运动、碰撞等与运动相关的工作。同时UE比较贴心，给Character添加了一个UCharacterMovementComponent组件，专门来处理Character的运动工作。**UCharacterMovementComponent**继承自**UPawnMovementComponent**，UPawnMovementComponent又继承自**UNavMovementComponent**，UNavMovementComponent又继承自**UMovementComponent**。 这些移动组件的继承关系如下图。

![img](https://pic3.zhimg.com/80/v2-408f3fbfddfa94ab7429cc35802bda96_720w.webp)

图片来自网络，侵删

我们来看看UMovementComponent的注释说明，发现了一个重要的函数**SafeMoveUpdatedComponent**。

![img](https://pic4.zhimg.com/80/v2-ce0c7a30f5b5ac9465eb951fc0e696fb_720w.webp)

我们不关心碰撞，只看看到底怎么移动的，里面实现移动的是**MoveUpdatedComponent**。

![img](https://pic4.zhimg.com/80/v2-8cf3b1cda47bfece1035a20042d56e0f_720w.webp)

![img](https://pic4.zhimg.com/80/v2-e8cc327210f47ac5709caa78dc90aebf_720w.webp)

MoveUpdatedComponent其实改变的是UpdateComponent组件的位置。UpdateComponent组件是**USceneComponent**类型，我们一开始想要移动的角色的根组件也是USceneComponent类型。我们继续深入UpdateComponent的MoveComponent函数实现了什么。

![img](https://pic3.zhimg.com/80/v2-5f2ed7cc48cce302b9f8a20f04217eaa_720w.webp)

![img](https://pic4.zhimg.com/80/v2-619babeee4a9f01675f2114f30f2715b_720w.webp)

![img](https://pic1.zhimg.com/80/v2-4dc6b7b65b5658178ff1bc936e8fd240_720w.webp)

![img](https://pic3.zhimg.com/80/v2-032e8bc6edb289735bceebf5251f45b2_720w.webp)

![img](https://pic3.zhimg.com/80/v2-1d4ad1025f4bca859d870f67cea293f2_720w.webp)

可见USceneComponent提供了一个基本位置信息**ComponentToWorld**，以及改变自身及其子组件的位置的接口函数**InternalSetWorldLocationAndRotation**。
介绍Actor的底层移动原理，回到UCharacterMovementComponent组件。

[https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/CharacterMovementComponent/docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/CharacterMovementComponent/](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/4.27/en-US/InteractiveExperiences/Networking/CharacterMovementComponent/)

## 三、移动 

这里来分析UE是如何让角色冻起来，在我们的游戏中有三种移动规则：

- 手动移动，通过控制摇杆（或WSAD）来控制主角的移动。
- 自动漫游移动。
- 鼠标点击屏幕角色移动到点击位置。

其中鼠标点击移动就是使用的**SetActorLocation**。让角色从当前位置插值到鼠标点击位置。

![img](https://pic1.zhimg.com/80/v2-717fdb32cbdb06d2797ebf959e33c3a8_720w.webp)

而另外两种则使用的是**AddMovementInput**。

![img](https://pic1.zhimg.com/80/v2-d3dc2803a9113649e2014632b05b6c3c_720w.webp)

![img](https://pic1.zhimg.com/80/v2-5b9415fd87706b2ffc65f246ab2a75d8_720w.webp)

### 1、AddMovementInput

接下来讲讲AddMovementInput到底是怎么回事。先贴一张更为详细的移动组件的相关类图。

![img](https://pic4.zhimg.com/80/v2-14f71aee41919cb1c29b55f3a7e0b757_720w.webp)

图片来自网络，侵删

从上图可以看到Actor绑定了一个**UInputComponent**组件，用来处理一些绑定的按键操作。

![img](https://pic2.zhimg.com/80/v2-4d46670219978c23a01f80aa9dd15ed1_720w.webp)

![img](https://pic2.zhimg.com/80/v2-c424a683ddee21d88396c39d51ae44e5_720w.webp)

我们以BP_Player_Hero（Character对应的蓝图）为例，打开BP_Player_Hero，选中Class Defaults，在Details面板，我们可以看到有一个Input分页，可以用来接受或拒绝按键输入。

![img](https://pic1.zhimg.com/80/v2-2c4c141a2caf5e73a38da39fdeeae854_720w.webp)

因为APlayerController继承自AController，而AController也继承自AActor，所以我们的BP_PC_Arena（APlayerController对应的蓝图）也有一个InputComponent组件。我们的手动操作角色移动正是利用了这个InputComponent组件。

![img](https://pic2.zhimg.com/80/v2-f38ea6e5d00a23f9144c25165f815021_720w.webp)

![img](https://pic1.zhimg.com/80/v2-4f7ea60f048d43accd6d1fe4070d1d8c_720w.webp)

接下来我们InputComponent是怎么让我们的角色移动起来的。
当我们滑动摇杆或者按下WSAD后，InputComponent会接收到信息，进而被APlayerController的**MoveForward**和**MoveRight**接受到。在MoveForward中计算出角色朝前移动的偏移量（Speed * DeltaTime * AxisValue），MoveRight同理。

![img](https://pic3.zhimg.com/80/v2-f6be03b945fe30ea4c3d0de7792872ba_720w.webp)

![img](https://pic1.zhimg.com/80/v2-8b6fbaa120fb5e8fd5df4118a033e898_720w.webp)

我们的角色继承自APawn。APawn封装了一个接口**AddMovementInput**。

![img](https://pic1.zhimg.com/80/v2-97c6d637a86fd2776b49c34d53e134ec_720w.webp)

![img](https://pic3.zhimg.com/80/v2-3e1b4fb07f41e1425da5744c8834b972_720w.webp)

![img](https://pic3.zhimg.com/80/v2-c07065914f46f4f63258ee12ae58bbc2_720w.webp)

![img](https://pic1.zhimg.com/80/v2-b9b930b0aab52690fdbf4cb5dd554d5c_720w.webp)

所以当我们按下按钮，最终会给角色身上**ControlInputVector**加上一段偏移量。

![img](https://pic2.zhimg.com/80/v2-bd65e55b9f53c8900fded8277aaee4fd_720w.webp)

可以看到ControlInputVector会被消费掉。找到是谁在消费。

![img](https://pic1.zhimg.com/80/v2-632f0a3e2d92f1de9e90a0068ebc37d8_720w.webp)

**Internal_ConsumeMovementInputVector**有两处调用。

![img](https://pic4.zhimg.com/80/v2-9b8d430c73fbd5f115b459614fada2a3_720w.webp)

![img](https://pic3.zhimg.com/80/v2-d1f8ef0635b901414995c0d69cf2852a_720w.webp)

其中**ConsumeMovementInputVector**会在APawn的Restart或UnPossessed中调用。而**UPawnMovementComponent::ConsumeInputVector**会在UCharacterMovementComponent的Tick中调用。

![img](https://pic1.zhimg.com/80/v2-c80476d29d9b62fd4fb8f544460390c0_720w.webp)

而这正是我们想找的，**我们按下按钮得到的角色偏移量被角色的UCharacterMovementComponent组件拿到了**。UCharacterMovementComponent拿到偏移量后，满足一些条件后执行**ControlledCharacterMove，**然后把Input的偏移量转换成了加速度**Acceleration**。

![img](https://pic1.zhimg.com/80/v2-72ca82ebf8d7abb749fffc98e34b1440_720w.webp)

![img](https://pic2.zhimg.com/80/v2-1497c8f1e8bc3693b8159ab7fc976e11_720w.webp)

### 2、PerformMovement

到这一步，我们也大概梳理出了移动的框架，但是移动逻辑的细节处理我们还没讲。再次打开官方文档，我们会找到**PerformMovement**。

[角色移动组件docs.unrealengine.com/4.27/zh-CN/InteractiveExperiences/Networking/CharacterMovementComponent/![img](https://pic4.zhimg.com/v2-e55c6e69f964c3de317fb65685fd3737_180x120.jpg)](https://link.zhihu.com/?target=https%3A//docs.unrealengine.com/4.27/zh-CN/InteractiveExperiences/Networking/CharacterMovementComponent/)

![img](https://pic1.zhimg.com/80/v2-841749400b025cdb84d9355800a380a0_720w.webp)

进入PerformMovement，找到两个我们想要的函数，**StartNewPhysics**和**PhysicsRotation**。

![img](https://pic4.zhimg.com/80/v2-95b979d8d846642908b4d2bfb8779e1f_720w.webp)

打开StartNewPhysics，因为我们的游戏是全程飞在空中，所以我们**MovementMode**设置的是**MOVE_Flying**。

![img](https://pic2.zhimg.com/80/v2-321bcfc308732644d146a5f19c77176d_720w.webp)

![img](https://pic1.zhimg.com/80/v2-1405d7dcb7a04a4456431a47f86086bc_720w.webp)

继续回到StartNewPhysics中，找到MOVE_Flying对应的**PhysFlying**。

![img](https://pic3.zhimg.com/80/v2-971e4975d56ce73721fb014f62112c8e_720w.webp)

在PhysFlying我们会找到一个我们已经讲过的函数**SafeMoveUpdatedComponent**。

![img](https://pic1.zhimg.com/80/v2-3e1a79a79baff1dc5e43e0c569dc4fd4_720w.webp)

### 3、Rotation

到这我们终于讲完了角色移动的Position，还差一个角色移动的朝向问题没说。在各种文章里我们经常看到别人说到几个变量：**bUseControllerRotationYaw、bOrientRotationToMovement、bUseControllerDesiredRotation**。我就在这踩过坑，我觉得还是有必要跟大家讲讲。
还是先回到PerformMovement中，我们还有一个函数**PhysicsRotation**没讲，如何在移动过程中改变角色朝向。

![img](https://pic4.zhimg.com/80/v2-d674861ca6fb9175970e094f3c8fb1f3_720w.webp)

![img](https://pic2.zhimg.com/80/v2-7571c03abff2bd03537111ebdf4427f9_720w.webp)

![img](https://pic3.zhimg.com/80/v2-1a9e1ccaf1c449b48fa845fc36e0dca6_720w.webp)

可以看到最后有一句
**MoveUpdatedComponent( FVector::ZeroVector, DesiredRotation, \*/\*bSweep\*/\* false );**
这里只更新角色的朝向Rotation，位置的偏移量是ZeroVector。

- **一旦bOrientRotationToMovement设置为True后，角色的朝向会转向移动的方向。**

![img](https://pic1.zhimg.com/80/v2-33f8a41e7154f17361c87a22ad81c220_720w.webp)

![img](https://pic4.zhimg.com/80/v2-b5b473b58d2a3311e80741383a4baa0f_720w.webp)

- **一旦bOrientRotationToMovement 为False，但是bUseControllerDesiredRotation设置为True。那么在角色移动中，如果Controller的朝向和角色朝向不一致，角色朝向就会平滑的按照RotationRate里配置的各个轴的旋转速率来旋转角色的Rotation到Controller的朝向。**

![img](https://pic2.zhimg.com/80/v2-cf07c90a0355d2fc6af9d304e3d2b7e5_720w.webp)

因为我们的游戏的视角是俯视角锁定视角的，角色的朝向会一直转向移动方向。所以我们采用的是**bOrientRotationToMovement**为True。

![img](https://pic3.zhimg.com/80/v2-f2bd3f4842ada7d5b983a793f3d284ae_720w.webp)

但是别忘了还有一个变量**bUseControllerRotationYaw。前面两种情况的前提是bUseControllerRotationYaw需要设为False。**

![img](https://pic4.zhimg.com/80/v2-53457d01fd0e96eb2e708fd167e2994b_720w.webp)

通常在第一人称或者第三人称，相机跟在角色身后一定位置，玩家的朝向会和Controller的朝向一致，这时候bUseControllerRotationYaw需要设为True。为啥bUseControllerRotationYaw设为True后，玩家的朝向为什么会和Controller的朝向一致？
在**APlayerController::TickActor**中有一段代码。若APlayerController中管理角色输入的对象存在，则会执行**PlayerTick**。

![img](https://pic4.zhimg.com/80/v2-639057f4a13ee79a9ac6e5bfe20fe837_720w.webp)

![img](https://pic1.zhimg.com/80/v2-637722106da24faf76e8bb5227b3c6f8_720w.webp)

在PlayerTick中会先处理玩家的输入**TickPlayerInput**，然后满足条件后会调用**UpdateRotation**。

![img](https://pic1.zhimg.com/80/v2-24485c8fae02d1e11a3966911536ca08_720w.webp)

![img](https://pic2.zhimg.com/80/v2-595a7645059a0b9aeda09bb3c1d7208d_720w.webp)

![img](https://pic1.zhimg.com/80/v2-09d1578301a1a0151a6af71ac3cb16a4_720w.webp)

在UpdateRotation中，先调用**SetControlRotation**给Controller设置好Rotation。紧接着判断Controller绑定的Pawn是否存在，存在则会调用APawn的**FaceRotation**。

![img](https://pic2.zhimg.com/80/v2-b969ebd05b865910314de24056f678f5_720w.webp)

就是在FaceRotation函数中，第三个变量bUseControllerRotationYaw开始了关键的作用。若bUseControllerRotationYaw为True，则设置角色朝向和Controller的朝向一致。

![img](https://pic4.zhimg.com/80/v2-208c25c6e127c46a57f60b8cada4eedf_720w.webp)

![img](https://pic4.zhimg.com/80/v2-d4cbabb53661405f0b92a21cdce4df2f_720w.webp)

至此，我们介绍完了UE是如何让角色移动。