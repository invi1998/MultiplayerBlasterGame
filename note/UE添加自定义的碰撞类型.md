# UE5中添加自定义的碰撞类型

1：项目设置-引擎-碰撞

![image-20240329183038566](.\image-20240329183038566.png)

2：点击 `新建Object通道`，命名，设置想要的默认的碰撞响应

![image-20240329183201839](.\image-20240329183201839.png)

3：保存后，就可以使用了

![image-20240329183629142](.\image-20240329183629142.png)

4：如果想要将这个自定义碰撞类型加入到ECC枚举中（其实不是添加进ECC中，只是取一个未用的通道，取一个别名），我们可以在游戏文件中使用未使用的通道枚举。如下：

```c++
#pragma once

#include "CoreMinimal.h"

#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1	// 骨骼网格 ECC_GameTraceChannel1 用于射线检测 1 代表的是 ECC_GameTraceChannel1 
#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2		// 命中框 ECC_GameTraceChannel2 用于射线检测 2 代表的是 ECC_GameTraceChannel2

```

5：然后就可以在代码中进行使用了

```c++
for (const auto& HitCollisionBox : HitCollisionBoxes)
{
	if (HitCollisionBox.Value == nullptr) continue;

	HitCollisionBox.Value->SetCollisionObjectType(ECC_HitBox);		// 设置碰撞对象类型，这是我们自定义的碰撞通道
	HitCollisionBox.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	// 设置碰撞响应，忽略所有通道
	HitCollisionBox.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);	// 设置碰撞响应，只响应我们自定义的碰撞通道
	HitCollisionBox.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);	// 设置碰撞启用，不启用碰撞
}
```

