// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"

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

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));		// 创建拾取物体的静态网格组件
	PickupMesh->SetupAttachment(OverlapSphere);		// 将拾取物体的静态网格组件附加到碰撞球体
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// 设置拾取物体的静态网格组件为无碰撞

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
}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
	}
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

