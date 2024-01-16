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

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));		// ���������
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapShpere"));		// ������ײ����
	OverlapSphere->SetupAttachment(RootComponent);		// ����ײ���帽�ӵ������
	OverlapSphere->SetSphereRadius(150.f);		// ������ײ����뾶
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);		// ������ײ����Ϊ��ѯ��ײ
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);		// ������ײ���������ͨ������ײ��ӦΪ����
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);		// ������ײ�����Pawnͨ������ײ��ӦΪ�ص�

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));		// ����ʰȡ����ľ�̬�������
	PickupMesh->SetupAttachment(OverlapSphere);		// ��ʰȡ����ľ�̬����������ӵ���ײ����
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);		// ����ʰȡ����ľ�̬�������Ϊ����ײ

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

