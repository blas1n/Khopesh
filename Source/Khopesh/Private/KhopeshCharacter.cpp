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

	LeftWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWeapon"));
	LeftWeapon->SetupAttachment(GetMesh(), TEXT("unequip_sword_l"));
	LeftWeapon->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	LeftWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	LeftWeapon->SetGenerateOverlapEvents(false);
	LeftWeapon->SetEnableGravity(false);

	RightWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWeapon"));
	RightWeapon->SetupAttachment(GetMesh(), TEXT("unequip_sword_r"));
	RightWeapon->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	RightWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	RightWeapon->SetGenerateOverlapEvents(false);
	RightWeapon->SetEnableGravity(false);

	Speed = 266.66f;
	bFightMode = bStartFight = bStrongMode = bEquiping = bUnequiping = false;
	CurrentSection = 0;
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

	Anim->OnAttack.BindUObject(this, &AKhopeshCharacter::OnAttack);
}

void AKhopeshCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(
		GetCharacterMovement()->MaxWalkSpeed,
		Speed, DeltaSeconds * 10.0f);

	if (IsEnemyNear())
	{
		if (!Anim->IsPlayMontage() && !bFightMode && !bEquiping)
		{
			Anim->PlayMontage(EMontage::EQUIP);
			bEquiping = true;
			bUnequiping = false;
		}
	}

	else
	{
		if (!Anim->IsPlayMontage() && bFightMode && !bUnequiping)
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

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AKhopeshCharacter::Attack);
	PlayerInputComponent->BindAction("Defense", IE_Pressed, this, &AKhopeshCharacter::Defense);

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
	if (bStartFight && !Anim->IsPlayMontage() && !GetCharacterMovement()->IsFalling())
	{
		Step_Server(GetRotatorByInputKey());
	}
}

void AKhopeshCharacter::Step_Server_Implementation(FRotator NewRotation)
{
	Step_Multicast(NewRotation);
}

bool AKhopeshCharacter::Step_Server_Validate(FRotator NewRotation)
{
	return true;
}

void AKhopeshCharacter::Step_Multicast_Implementation(FRotator NewRotation)
{
	SetActorRotation(NewRotation);
	Anim->PlayMontage(bFightMode ? EMontage::DODGE_EQUIP : EMontage::DODGE_UNEQUIP);
}

void AKhopeshCharacter::Attack()
{
	if (!bFightMode || Anim->IsPlayMontage()) return;

	Anim->PlayAttackMontage(bStrongMode ? EMontage::ATTACK_STRONG : EMontage::ATTACK_WEAK, CurrentSection++);
	CurrentSection %= MaxSection;
}

void AKhopeshCharacter::Defense()
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

	CurrentSection = 0;
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

void AKhopeshCharacter::OnAttack()
{
	FCollisionObjectQueryParams CollisionObjectQueryParams(ECollisionChannel::ECC_Pawn);
	FCollisionQueryParams CollisionQueryParams(NAME_None, false, this);

	TArray<FHitResult> Out;

	bool bResult = GetWorld()->SweepMultiByObjectType(
		Out,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * AttackRange,
		FQuat::Identity,
		CollisionObjectQueryParams,
		FCollisionShape::MakeSphere(AttackRange),
		CollisionQueryParams
	);

#if ENABLE_DRAW_DEBUG
	FVector TraceVec = GetActorForwardVector() * AttackRange;
	FVector Center = GetActorLocation() + TraceVec * 0.5f;
	float HalfHeight = AttackRange * 0.5f + AttackRadius;
	FQuat CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	FColor DrawColor = bResult ? FColor::Green : FColor::Red;
	float DebugLifeTime = 5.0f;

	DrawDebugCapsule(GetWorld(),
		Center,
		HalfHeight,
		AttackRadius,
		CapsuleRot,
		DrawColor,
		false,
		DebugLifeTime);
#endif

	for (const auto& Result : Out)
	{
		Result.GetActor()->TakeDamage(
			bStrongMode ? StrongAttackDamage : WeakAttackDamage,
			FDamageEvent(),
			GetController(),
			this);
	}
}

bool AKhopeshCharacter::IsEnemyNear() const
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

FRotator AKhopeshCharacter::GetRotatorByInputKey() const
{
	float Horizontal = GetInputAxisValue(TEXT("MoveRight"));
	float Vertical = GetInputAxisValue(TEXT("MoveForward"));

	float CharacterRot = Horizontal * 90.0f;
	CharacterRot += Horizontal * Vertical * -45.0f;

	if (Horizontal == 0.0f)
		CharacterRot = (Vertical == -1.0f) ? 180.0f : 0.0f;

	return FRotator(0.0f, GetControlRotation().Yaw + CharacterRot, 0.0f);
}