// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Blaster/Weapon/WeaponTypes.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));		// 创建根组件
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapShpere"));		// 创建碰撞球体
	OverlapSphere->SetupAttachment(RootComponent);		// 将碰撞球体附加到根组件
	OverlapSphere->SetSphereRadius(150.f);		// 设置碰撞球体半径
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);		// 设置碰撞球体为查询碰撞
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);		// 设置碰撞球体对所有通道的碰撞响应为忽略
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);		// 设置碰撞球体对Pawn通道的碰撞响应为重叠

	// 设置拾取网格体的初始位置，初始大小
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));	// 设置碰撞球体的初始位置


	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));		// 创建拾取物体的静态网格组件
	PickupMesh->SetupAttachment(OverlapSphere);		// 将拾取物体的静态网格组件附加到碰撞球体
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// 设置拾取物体的静态网格组件为无碰撞
	PickupMesh->SetRelativeScale3D(FVector(5.0f, 5.0f, 5.0f));		// 设置拾取物体的静态网格组件的初始大小
	PickupMesh->SetRenderCustomDepth(true);	// 开启自定义深度渲染
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);	// 设置自定义深度值

	PickupEffectNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectNiagaraComponent"));		// 创建拾取特效
	PickupEffectNiagaraComponent->SetupAttachment(RootComponent);		// 设置拾取特效的父组件为根组件

}

void APickup::Destroyed()
{
	Super::Destroyed();

	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}

	if (PickupEffectNiagaraSystem)
	{
		// 我希望拾取组件被拾取后，有一个销毁的特效，所以我这里创建了一个NiagaraSystem，这与NiagaraComponent的创建方式不同，
		// 这里我们使用了NiagaraFunctionLibrary::SpawnSystemAtLocation()方法来创建NiagaraSystem
		// 这个方法需要传入一个WorldContextObject，这里我们传入this，因为我们希望在这个Actor的位置创建特效
		// 然后传入NiagaraSystem，这里我们传入了PickupEffectNiagaraSystem，这个变量在HealthPickup.h中定义
		// 最后传入位置和旋转，这里我们传入了血宝的位置和旋转
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffectNiagaraSystem, GetActorLocation(), GetActorRotation());
	}
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(BindOverlapTimer, this, &APickup::BindOverlapTimerFinished, BindOverlapTime);
	}

	
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

void APickup::BindOverlapTimerFinished()
{
	if (HasAuthority())
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
	}
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickupMesh)
	{
		PickupMesh->AddRelativeRotation(FRotator(0.f, 1.f, 0.f));		// 拾取物体的静态网格组件每帧旋转
	}
}

