#include "Casing.h"

ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
}