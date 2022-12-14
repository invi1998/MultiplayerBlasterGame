#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()

public:
	ACasing();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* CasingMesh;
};
