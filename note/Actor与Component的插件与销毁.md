## Actor的创建与销毁

### 创建

UClass*
TSubclassOf<T>
SpawnActor()

```c++
UPROPERTY(EditAnywhere, Category = "init")
	UClass* bullet0; // 取类

UPROPERTY(EditAnywhere, Category = "init")
	TSubclassOf<Amissile> bullet; //取Amissile类及其派生类

UPROPERTY(EditAnywhere, Category = "init")
	Amissile* bullet2; // 取类的实例
```

### 销毁

直接销毁：Destroy()

```c++
MissileIns->Destroy();
```

### 设置生命周期

构造函数设置InitialLifeSpan

```c++
InitialLifeSpan = 8.0f;
```

BeginPlay里使用SetLifeSpan()设置声明周期

## Component 的创建与销毁

### 创建

构造函数创建：CreateDefaultSubobject<T>
```c++
UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* paddle1;

paddle1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("paddle1"));
```

Runtime创建：NewObject<T>、RegisterComponent()
```c++
Mesh = NewObject<UStaticMeshComponent>(this,TEXT("MyMesh"));
Mesh->SetupAttachment(root);
Mesh->RegisterComponent(); // 注册渲染/物理状态
// 可配合 LoadObject() 
```



### 加载资源

#### FObjectFinder构造函数加载资源

ConstructorHelpers::FObjectFinder<T>

```c++

static ConstructorHelpers::FObjectFinder<UStaticMesh> paddleMesh(TEXT("StaticMesh'/Game/Demo_Drone/SM/paddle.paddle'"));
if (paddleMesh.Succeeded())
{
	paddle1->SetStaticMesh(paddleMesh.Object);
}
```

#### LoadObject加载资源

```c++
UStaticMesh* bodyMesh = LoadObject<UStaticMesh>(this,TEXT("StaticMesh'/Game/Demo_Drone/SM/drone.drone'"));
if (bodyMesh)
{
	body->SetStaticMesh(bodyMesh);
}
```

### 设置组件层级关系

设置根组件RootComponent或者SetRootComponent()
SetupAttachment
AttachToComponent

```c++
RootComponent = outCollison;
SetRootComponent(outCollison);

paddle1->SetupAttachment(body, TEXT("paddle1"));
paddle2->SetupAttachment(body, TEXT("paddle2"));
paddle3->AttachToComponent(body, FAttachmentTransformRules::KeepRelativeTransform, TEXT("paddle3"));
paddle4->AttachToComponent(body, FAttachmentTransformRules::KeepRelativeTransform, TEXT("paddle4"));

childSceneComponent->AttachToComponent(root, FAttachmentTransformRules::SnapToTargetIncludingScale);
```

### 销毁组件

DestoryComponent()
UnregisterComponent()使用RegisterComponent创建，需要先注销再销毁

```c++
Mesh->UnregisterComponent();
Mesh->DestroyComponent();
```

## 各类组件创建与初始化

### UStaticMenshComponent

```c++
paddle1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("paddle1"));
auto paddleMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Game/Demo_Drone/SM/paddle.paddle'"));
if (paddleMesh.Object != nullptr)
{
	paddle1->SetStaticMesh(paddleMesh.Object);
}
```

### UBoxComponent

```c++
outCollison = CreateDefaultSubobject<UBoxComponent>(TEXT("outCollison"));
outCollison->SetBoxExtent(FVector(60, 60, 15));
outCollison->SetSimulatePhysics(true);
outCollison->SetCollisionProfileName(TEXT("WorldDynamic"));
outCollison->SetCollisionEnabled(ECollisionEnabled::NoCollision);
```

### USphereComponent

```c++
sphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
sphereComp->InitSphereRadius(5.0f);
sphereComp->SetCollisionProfileName(TEXT("WorldDynamic"));
sphereComp->SetGenerateOverlapEvents(true);
sphereComp->OnComponentBeginOverlap.AddDynamic(this, &Amissile::Overlaphandler);
// overlap 函数绑定
void Amissile::Overlaphandler(UPrimitiveComponent* OverlappedComponent,
				AActor* OtherActor, 
				UPrimitiveComponent* OtherComp,
				int32 OtherBodyIndex,
				bool bFromSweep,
				const FHitResult& SweepResult){}
```

### USkeletalMeshComponent

```c++
SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
SkeletalMeshComp->SetGenerateOverlapEvents(true);
SkeletalMeshComp->SetSimulatePhysics(true);
```

### USpringArmComponent和UCameraComponent

```c++
springArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
springArmComp->SetupAttachment(RootComponent);
cameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
cameraComp->SetupAttachment(springArmComp);
```

### UPhysicsThrusterComponent

```c++
upThrusterComp = CreateDefaultSubobject<UPhysicsThrusterComponent>(TEXT("upThrusterComp"));
upThrusterComp->SetupAttachment(RootComponent);
upThrusterComp->SetWorldRotation(UKismetMathLibrary::MakeRotFromX(FVector(-this->GetActorUpVector())));
upThrusterComp->ThrustStrength = 980.0f;
upThrusterComp->SetAutoActivate(true);
```

### UTexRenderComponent

```c++
CountdownText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CountdownNumber"));
CountdownText->SetHorizontalAlignment(EHTA_Center);
CountdownText->SetWorldSize(150.0f);
CountdownText->AttachTo(RootComponent);
CountdownTime = 3;
CountdownText->SetText(FString::FromInt(FMath::Max(CountdownTime, 0)));
```

### UProjectileMovementComponent

```c++
projectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("projectileMovement"));
projectileMovementComp->InitialSpeed = 500.0f;
```

