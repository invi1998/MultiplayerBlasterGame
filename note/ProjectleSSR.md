# Projectile Server Side Rewind

```c++
#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 投射武器类，派生自	Weapon
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;	// 子弹类

	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;	// 服务端倒带子弹类
};

```

如上所示，我们的投射类武器（冲锋枪为例），它有两个子弹类，一个是属性复制的，一个不复制的，我们需要决定何时生成（或者说何时使用某个具体的类）

![image-20240329175041524](.\image-20240329175041524.png)

如图，我们有一个服务器和客户端，并且在服务器上的弹丸，要么是本地控制的，要么是非本地控制的。如果我们在服务器上，并且在本地控制托管玩家的位置，我们就会在服务端控制我们的角色，并且其他所有在服务器上的角色都会由其他的客户端控制。

现在如果我们在客户端上，我们要么是本地控制的，要么不是本地控制的。如果是本地控制的，我们就是在控制我们自己的角色，而所有其他的角色都是代理模拟的。

现在是否生成属性复制的弹丸取决于几件事。我们知道我们的武器本身有一个使用过的服务器端倒带布尔值 `bool bUseServerSideRewind = false;// 是否使用服务端倒带`，所以，我们的武器具有服务器端倒带控制变量，但是同时我们的投射类弹丸也是如此，这是因为弹丸可能想要使用服务器端倒带。

如果我们是在服务器上，我们的的武器设置为使用服务器端倒带，并且，我们是本地控制的，我们任然只是生成一个复制的弹丸，而没有在弹丸上启用服务端倒带。这是因为服务器上的本地控制角色不需要使用功能服务端倒带，服务器本地的所有计算都是权威可信的，它发射一枚射弹，并且根据弹头事件，他会立即在服务器上照成伤害，并将结果复制到客户端。

现在，对于在服务器上使用服务端倒带从武器上发射射弹的角色，如果我们不是本地控制的，那么发射炮弹的玩家就是其他客户端的角色。对于这种情况，我们只会生成一个不可复制的射弹，并且不启用服务端倒带。这是因为该客户端现在将生成他们自己本地的非复制射弹，这些射弹将会使用服务端倒带，然后根据倒带结果，客户端上的弹丸将会发送服务器的得分请求。

对于启用了Server Side Rewind(SSR)武器的客户端本地控制角色，如果该角色是本地控制的，他将会生成一个不可复制的射弹用于在该弹丸击中其计算机上的本地某人时，用来做服务端倒带计算。他将向服务端发送得分请求，以便服务端可以对其进行验证。

但是，向非本地控制的客户端角色发射的武器是为了响应多播RPG中的“发射武器”，这意味着武器是在其他人的机器上发射的。因此，即使武器在非本地控制的客户端上使用服务器端倒带，我们也会产生一个不使用服务器端倒带的炮弹。

现在，如果武器本身被设置为不使用服务器端倒带，那么在服务器上，无论我们是否受到本地控制，我们仍然会产生一个复制的投射物。未启用服务器端倒带。因此，即使客户端发射武器，炮弹仍将在服务器上产生并复制到客户端。

如果我们的武器没有设置为使用SSR，并且武器在客户端上开火，那么客户端上就不会产生炮弹，因为我们知道服务器最终会产生一个复制的炮弹，我们会在我们的机器上看到它。现在我们知道了什么时候产生复制的和非复制的射弹。



# WithValidation

**UFunction** 是虚幻引擎4（UE4）反射系统可识别的C++函数。 `UObject `或蓝图函数库可将成员函数声明为UFunction，方法是将 `UFUNCTION `宏放在头文件中函数声明上方的行中。宏将支持 **函数说明符** 更改UE4解译和使用函数的方式。

```
UFUNCTION([specifier1=setting1, specifier2, ...], [meta(key1="value1", key2, ...)])
ReturnType FunctionName([Parameter1, Parameter2, ..., ParameterN1=DefaultValueN1, ParameterN2=DefaultValueN2]) [const];
```

可利用函数说明符将UFunction对 [蓝图可视化脚本 ](https://docs.unrealengine.com/4.27/zh-CN/ProgrammingAndScripting/Blueprints)图表公开，以便开发者从蓝图资源调用或扩展UFunction，而无需更改C++代码。在类的默认属性中，UFunction可绑定到 [委托 ](https://docs.unrealengine.com/4.27/zh-CN/ProgrammingAndScripting/ProgrammingWithCPP/UnrealArchitecture/Delegates)，从而能够执行一些操作（例如将操作与用户输入相关联）。它们还可以充当网络回调，这意味着当某个变量受网络更新影响时，用户可以将其用于接收通知并运行自定义代码。用户甚至可创建自己的控制台命令（通常也称 *debug* 、 *configuration* 或 *cheat code* 命令），并能在开发版本中从游戏控制台调用这些命令，或将拥有自定义功能的按钮添加到关卡编辑器中的游戏对象。

WithValidation 用于声明名称与主函数相同的附加函数，但是末尾需要添加`_Validate`。此函数使用相同的参数，但是会返回`bool`，以指示是否应继续调用主函数。

它可能作为函数调用或者类方法的一个选项出现，用来确保数据的有效性和完整性。

例如，在处理资产导入、保存或加载时，带有“WithValidation”后缀的方法可能会执行额外的数据验证步骤，以确保数据结构符合预期规则，不包含无效引用或其他可能导致运行时错误的情况。这种验证机制对于避免潜在的游戏崩溃、资源丢失等问题至关重要。

在UE5源代码或API中，开发者可能会看到类似这样的用法：

- `SavePackageWithValidation()`: 这种函数调用会在保存关卡或 actor 包时进行严格的数据验证，确保所有关联的数据都是合法且可序列化的。
- 在蓝图编译或更新过程中，也可能存在类似的验证环节，确保蓝图节点逻辑正确无误。

```c++
	// 创建server RPC
	// 这个函数被设计为从客户端调用，然后再服务端进行执行
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TracerHitTarget, float FireDelay);
```

如上，添加了一WithValidation标识，就需要给该函数添加一个验证函数，返回bool，可以在cpp中或者直接在h中实现

```c++
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TracerHitTarget, float FireDelay)
{
	MuticastFire(TracerHitTarget);
}

// 验证是否可以开火，这个函数是用来验证客户端发送给服务端的开火请求是否合法的
bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TracerHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		// 如果当前客户端的武器和服务端的武器的开火延迟是一样的，那么就可以开火（延迟精度为0.001f）
		if (FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f))
		{
			return true;
		}
	}

	return true;
}
```

