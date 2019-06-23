// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "KhopeshCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "KhopeshAnimInstance.h"
#include "Animation/AnimMontage.h"
#include "TimerManager.h"

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

	LeftWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWeapon"));
	LeftWeapon->SetupAttachment(GetMesh(), TEXT("unequip_sword_l"));

	RightWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWeapon"));
	RightWeapon->SetupAttachment(GetMesh(), TEXT("unequip_sword_r"));

	Speed = 266.66f;
	bFightMode = bStartFight = bStrongMode = bEquiping = bUnequiping = false;
}

void AKhopeshCharacter::BeginPlay()
{
	Super::BeginPlay();

	Anim = Cast<UKhopeshAnimInstance>(GetMesh()->GetAnimInstance());

	Anim->OnSetFightMode.BindLambda([this](bool IsFightMode)
	{
		Anim->SetFightMode(IsFightMode);
		SetEquip(IsFightMode);
		bFightMode = IsFightMode;
	
		if (IsFightMode && !bStartFight)
		{
			Speed += 266.66f;
			bStartFight = true;
		}
	});
}

void AKhopeshCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(
		GetCharacterMovement()->MaxWalkSpeed,
		Speed, DeltaSeconds * 10.0f);

	if (IsEnemyNear())
	{
		if (!Anim->Montage_IsPlaying(nullptr) && !bFightMode && !bEquiping)
		{
			Anim->PlayMontage(EMontage::EQUIP);
			bEquiping = true;
			bUnequiping = false;
		}
	}

	else
	{
		if (!Anim->Montage_IsPlaying(nullptr) && bFightMode && !bUnequiping)
		{
			Anim->PlayMontage(EMontage::UNEQUIP);
			bUnequiping = true;
			bEquiping = false;
		}
	}
}

void AKhopeshCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AKhopeshCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AKhopeshCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AKhopeshCharacter::OnAttack);
	PlayerInputComponent->BindAction("Defense", IE_Pressed, this, &AKhopeshCharacter::OnDefense);

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
	if (!bStartFight || Anim->Montage_IsPlaying(nullptr) || GetCharacterMovement()->IsFalling())
		return;

	float Horizontal = GetInputAxisValue(TEXT("MoveRight"));
	float Vertical = GetInputAxisValue(TEXT("MoveForward"));

	float CharacterRot = Horizontal * 90.0f;
	CharacterRot += Horizontal * Vertical * -45.0f;

	if (Horizontal == 0.0f)
		CharacterRot = (Vertical == -1.0f) ? 180.0f : 0.0f;

	const FRotator Rotation(0.0f, Controller->GetControlRotation().Yaw + CharacterRot, 0.0f);
	SetActorRotation(Rotation);

	Anim->PlayMontage(bFightMode ? EMontage::DODGE_EQUIP : EMontage::DODGE_UNEQUIP);
}

void AKhopeshCharacter::OnAttack()
{

}

void AKhopeshCharacter::OnDefense()
{
	
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
	FName LeftWeaponSocket = IsEquip ? TEXT("equip_sword_l") : TEXT("unequip_sword_l");
	FName RightWeaponSocket = IsEquip ? TEXT("equip_sword_r") : TEXT("unequip_sword_r");

	LeftWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftWeaponSocket);
	RightWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightWeaponSocket);
}

void AKhopeshCharacter::WalkMode()
{
	if (bStartFight)
	{
		Speed -= 266.66f;
	}
}

void AKhopeshCharacter::RunMode()
{
	if (bStartFight)
	{
		Speed += 266.66f;
	}
}

bool AKhopeshCharacter::IsEnemyNear()
{
	FCollisionQueryParams CollisionQueryParam(NAME_None, false, this);
	TArray<FOverlapResult> Out;

	GetWorld()->OverlapMultiByObjectType(
		Out,
		GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(InFightRange),
		CollisionQueryParam);

	return Out.Num() > 0;
}