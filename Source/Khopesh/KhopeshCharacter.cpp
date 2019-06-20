// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "KhopeshCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ConstructorHelpers.h"
#include "KhopeshAnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"
#include "Weapon.h"
#include "DrawDebugHelpers.h"

AKhopeshCharacter::AKhopeshCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = 266.66f;
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 88.0f);
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	Speed = 266.66f;
	FightSwapDelay = 0.0f;
	bFightMode = false;
}

void AKhopeshCharacter::OnSetFightMode(bool IsFightMode)
{
	AnimInstance->SetFightMode(IsFightMode);
	SetEquip(IsFightMode);
	bFightMode = IsFightMode;

	if (IsFightMode && !bStartFight)
	{
		Speed += 266.66f;
		bStartFight = true;
	}
}

void AKhopeshCharacter::BeginPlay()
{
	Super::BeginPlay();

	AnimInstance = Cast<UKhopeshAnimInstance>(GetMesh()->GetAnimInstance());

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = SpawnParams.Instigator = this;

	LeftWeapon = GetWorld()->SpawnActor<AWeapon>(LeftWeaponClass.Get(), SpawnParams);
	RightWeapon = GetWorld()->SpawnActor<AWeapon>(RightWeaponClass.Get(), SpawnParams);

	SetEquip(false);
}

void AKhopeshCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(
		GetCharacterMovement()->MaxWalkSpeed,
		Speed, DeltaSeconds * 10.0f);

	FCollisionQueryParams CollisionQueryParam(NAME_None, false, this);
	TArray<FOverlapResult> Out;

	GetWorld()->OverlapMultiByObjectType(
		Out,
		GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(InFightRange),
		CollisionQueryParam);

	DrawDebugSphere(GetWorld(), GetActorLocation(), InFightRange, 32, FColor::Black);

	if (Out.Num() > 0 && !bFightMode && !GetWorldTimerManager().TimerExists(EquipTimer))
	{
		GetWorldTimerManager().ClearTimer(UnequipTimer);

		GetWorldTimerManager().SetTimer(EquipTimer, [this]() {
			AnimInstance->PlayMontageUnique(Equip);
		}, FightSwapDelay, false);
	}

	else if (Out.Num() == 0 && bFightMode && !GetWorldTimerManager().TimerExists(UnequipTimer))
	{
		GetWorldTimerManager().ClearTimer(EquipTimer);

		GetWorldTimerManager().SetTimer(UnequipTimer, [this]() {
			AnimInstance->PlayMontageUnique(Unequip);
		}, FightSwapDelay, false);
	}
}

void AKhopeshCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AKhopeshCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AKhopeshCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Step", IE_Pressed, this, &AKhopeshCharacter::Step);

	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AKhopeshCharacter::RunMode);
	PlayerInputComponent->BindAction("Dash", IE_Released, this, &AKhopeshCharacter::WalkMode);
}

void AKhopeshCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (!FMath::IsNearlyEqual(Value, 0.0f, 0.1f)))
	{
		Move(EAxis::X, Value);
	}
}

void AKhopeshCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (!FMath::IsNearlyEqual(Value, 0.0f, 0.1f)))
	{
		Move(EAxis::Y, Value);
	}
}

void AKhopeshCharacter::Step()
{
	if (!bStartFight) return;

	float Horizontal = GetInputAxisValue(TEXT("MoveRight"));
	float Vertical = GetInputAxisValue(TEXT("MoveForward"));

	float CharacterRot = Horizontal * 90.0f;
	CharacterRot += Horizontal * Vertical * -45.0f;

	if (Horizontal == 0.0f)
		CharacterRot = (Vertical == -1.0f) ? 180.0f : 0.0f;

	const FRotator Rotation(0.0f, Controller->GetControlRotation().Yaw + CharacterRot, 0.0f);
	SetActorRotation(Rotation);

	UAnimMontage* Dodge = bFightMode ? DodgeEquip : DodgeUnequip;
	AnimInstance->PlayMontageUnique(Dodge);
}

void AKhopeshCharacter::Move(EAxis::Type Axis, float Value)
{
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);
	AddMovementInput(Direction, Value);
}

void AKhopeshCharacter::SetEquip(bool IsEquip)
{
	if (LeftWeapon)
	{
		LeftWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	if (RightWeapon)
	{
		RightWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}

	FName LeftWeaponSocket = IsEquip ? TEXT("equip_sword_l") : TEXT("unequip_sword_l");
	FName RightWeaponSocket = IsEquip ? TEXT("equip_sword_r") : TEXT("unequip_sword_r");

	LeftWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftWeaponSocket);
	RightWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightWeaponSocket);
}

void AKhopeshCharacter::WalkMode()
{
	if (bStartFight) {
		Speed -= 266.66f;
	}
}

void AKhopeshCharacter::RunMode()
{
	if (bStartFight) {
		Speed += 266.66f;
	}
}