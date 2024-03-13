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

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));		// ���������
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapShpere"));		// ������ײ����
	OverlapSphere->SetupAttachment(RootComponent);		// ����ײ���帽�ӵ������
	OverlapSphere->SetSphereRadius(150.f);		// ������ײ����뾶
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);		// ������ײ����Ϊ��ѯ��ײ
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);		// ������ײ���������ͨ������ײ��ӦΪ����
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);		// ������ײ�����Pawnͨ������ײ��ӦΪ�ص�

	// ����ʰȡ������ĳ�ʼλ�ã���ʼ��С
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f));	// ������ײ����ĳ�ʼλ��


	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));		// ����ʰȡ����ľ�̬�������
	PickupMesh->SetupAttachment(OverlapSphere);		// ��ʰȡ����ľ�̬����������ӵ���ײ����
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// ����ʰȡ����ľ�̬�������Ϊ����ײ
	PickupMesh->SetRelativeScale3D(FVector(5.0f, 5.0f, 5.0f));		// ����ʰȡ����ľ�̬��������ĳ�ʼ��С
	PickupMesh->SetRenderCustomDepth(true);	// �����Զ��������Ⱦ
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);	// �����Զ������ֵ

	PickupEffectNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectNiagaraComponent"));		// ����ʰȡ��Ч
	PickupEffectNiagaraComponent->SetupAttachment(RootComponent);		// ����ʰȡ��Ч�ĸ����Ϊ�����

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
		// ��ϣ��ʰȡ�����ʰȡ����һ�����ٵ���Ч�����������ﴴ����һ��NiagaraSystem������NiagaraComponent�Ĵ�����ʽ��ͬ��
		// ��������ʹ����NiagaraFunctionLibrary::SpawnSystemAtLocation()����������NiagaraSystem
		// ���������Ҫ����һ��WorldContextObject���������Ǵ���this����Ϊ����ϣ�������Actor��λ�ô�����Ч
		// Ȼ����NiagaraSystem���������Ǵ�����PickupEffectNiagaraSystem�����������HealthPickup.h�ж���
		// �����λ�ú���ת���������Ǵ�����Ѫ����λ�ú���ת
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
		PickupMesh->AddRelativeRotation(FRotator(0.f, 1.f, 0.f));		// ʰȡ����ľ�̬�������ÿ֡��ת
	}
}

