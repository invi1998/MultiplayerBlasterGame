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

	// 子弹命中逻辑处理函数
	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* CasingMesh;	// 子弹外壳网格

	UPROPERTY(EditAnywhere)
		float ShellEjectionImpulse;	// 子弹弹壳弹出冲量

	UPROPERTY(EditAnywhere)
		class USoundCue* ShellSound;	// 子弹弹壳声音
};
