// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// 将弹簧臂链接到网格上
	CameraBoom->SetupAttachment(GetMesh());
	// 设置相机臂长度
	CameraBoom->TargetArmLength = 600.f;
	// 把这个设置为true，这样当我们添加鼠标输入时，就可以随着控制器旋转相机吊杆。
	CameraBoom->bUsePawnControlRotation = true;

	// 前置相机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	// 绑定相机臂
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// 因为有相机臂控制旋转，所以不需要设置旋转为true
	FollowCamera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}