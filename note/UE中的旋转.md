# UE5中的旋转：三个欧拉角Picth、Yaw、Roll及FRotator

三维变换是游戏开发中经常需要打交道的内容。平移、缩放、旋转是三种最常用的三维变换。平移用于确定物体的位置，缩放用于确定物体的大小，旋转用于确定物体的朝向。可见，这三种变换对于在游戏世界中摆放各个物体是何等的重要。

相对而言，平移、缩放较简单，但旋转较难，更需仔细研究。本文将介绍 UE 中旋转相关的知识。

## 一，三个欧拉角Picth、Yaw、Roll

UE 向开发者提供旋转物体的方式是三个欧拉角：Picth、Yaw、Roll。Pitch 是俯仰角、Yaw 是偏航角、Roll 是滚动角。通过这三个角就能控制物体的旋转，确定一个物体的朝向。

### 1.1 俯仰角Pitch

在 UE 中，俯仰角 Pitch 是指绕绿色 Y 轴（也就是方向朝右的那个轴）旋转的角。如下面所示，图1是三个欧拉角都为0时的原始图，图2是把俯仰角 Pitch 设为30度后的对比图。从图中可以看出，人物确实绕着 Y 轴旋转了。俯仰角旋转的效果，可以想象成使人前倾或后仰。

![img](https://pic4.zhimg.com/80/v2-4fd9b2056043a141cd267d1ccb1fc717_720w.webp)

图1，未旋转时的状态

![img](https://pic1.zhimg.com/80/v2-b2cb95105ad8e9e5dd72ac6db5328f88_720w.webp)

图2，俯仰角Pitch的旋转效果

## 1.2 偏航角Yaw

在 UE 中，偏航角 Yaw 是指绕蓝色 Z 轴（也就是方向朝上的那个轴）旋转的角。图3是把偏航角 Yaw 设为30度后的效果图。对比图1可以发现，偏航角 Yaw 使人物绕着 Z 轴旋转。偏航角旋转的效果，可以想象成人在原地转动。

![img](https://pic4.zhimg.com/80/v2-0e4c49f66adcf1f8317ec8742a71c0cb_720w.webp)

图3，偏航角Yaw的旋转效果

### 1.3 滚动角Roll

在 UE 中，滚动角 Roll 是指绕红色X 轴（也就是方向朝前的那个轴）旋转的角。图4是把滚动角 Roll 设为30度后的效果图。对比图1可以发现，滚动角 Roll 使人物绕着 X 轴旋转。滚动角旋转的效果，可以想象成把人放在转盘上转动。

![img](https://pic2.zhimg.com/80/v2-2127f75eac519a73d6db3350ef17e331_720w.webp)

图4，滚动角Roll的旋转效果

## 二，旋转类FRotator

FRotator 是 UE 用来封装三个欧拉角 Picth、Yaw、Roll 的类。开发者经常需要与 FRotator 打交道，就像需要和表示物体位置的 FVector 打交道一样。

### 2.1 FRotator 是三个欧拉角Picth、Yaw、Roll的封装类

FRotator 有三个成员变量：Picth、Yaw、Roll，正好是三个欧拉角。因此，我们可以说 FRotator 是封装三个欧拉角 Picth、Yaw、Roll 的类。

```cpp
struct FRotator
{
public:
    /** Rotation around the right axis (around Y axis), Looking up and down (0=Straight Ahead, +Up, -Down) */
    float Pitch; 

    /** Rotation around the up axis (around Z axis), Running in circles 0=East, +North, -South. */
    float Yaw; 

    /** Rotation around the forward axis (around X axis), Tilting your head, 0=Straight, +Clockwise, -CCW. */
    float Roll;
}
```

如下是它的构造函数，是根据三个欧拉角 Picth、Yaw、Roll 来构造出一个 FRotator。

```cpp
FORCEINLINE FRotator::FRotator( float InF ) : Pitch(InF), Yaw(InF), Roll(InF) 
{
    DiagnosticCheckNaN();
}


FORCEINLINE FRotator::FRotator( float InPitch, float InYaw, float InRoll ) : Pitch(InPitch), Yaw(InYaw), Roll(InRoll) 
{
    DiagnosticCheckNaN();
}
```

### 2.2 FRotator 多用于对外接口，FQuat 多用于内部实现

在游戏引擎内部，通常使用四元数来表示旋转，因为四元数能轻易地串接旋转以及进行旋转插值。UE 也不例外。在 UE 内部，使用 FQuat 表示四元数，并用来进行旋转计算。但是四元数并不容易直观理解，在一点上不如欧拉角。为了平衡这一矛盾，UE 就多使用 FRotator 对外提供接口，而 FQuat 多用于实现 UE 内部的功能。换言之，游戏开发者多关注 FRotator，而引擎开发者更注重 FQuat。FRotator 与 FQuat 之间的关系，可以概括为 FRotator 多用于对外接口，FQuat 多用于内部实现，且二者之间可以相互转换。

FRotator 向 FQuat 的转换，是通过 FRotator 的成员函数Quaternion()完成，其声明如下：

```cpp
    /**
     * Get Rotation as a quaternion.
     *
     * @return Rotation as a quaternion.
     */
    CORE_API FQuat Quaternion() const;
```

FQuat 向 FRotator 的转换，是通过 FQuat 的成员函数 Rotator()完成，其声明如下：

```cpp
    /** Get the FRotator representation of this Quaternion. */
    CORE_API FRotator Rotator() const;
```

在 FRotator 求逆的时候，是先转换成 FQuat，再对 FQuat 求逆，完成后再转换回 FRotator 。在这一点的实现上，FRotator 是一个对外的接口，FQuat 充当了内部实现的工具。

```cpp
FRotator FRotator::GetInverse() const
{
    return Quaternion().Inverse().Rotator();
}
```