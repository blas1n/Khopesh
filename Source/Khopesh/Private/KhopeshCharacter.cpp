// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "KhopeshCharacter.h"
#include "KhopeshAnimInstance.h"
#include "UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

AKhopeshCharacter::AKhopeshCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->MaxWalkSpeed = IncreaseSpeed;
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.0f;
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

	IsCombatMode = IsStartCombat = IsStrongMode = IsEquipingNow = IsUnequipingNow = false;
	HP = SpeedRate = ComboDelay = CurrentCombo = 0;
}

void AKhopeshCharacter::BeginPlay()
{
	Super::BeginPlay();

	Anim = Cast<UKhopeshAnimInstance>(GetMesh()->GetAnimInstance());

	Anim->OnSetCombatMode.BindLambda([this](bool IsCombat)
	{
		Anim->SetCombatMode(IsCombat);
		SetEquip(IsCombat);
		IsCombatMode = IsCombat;
	
		if (IsCombat && !IsStartCombat)
		{
			Speed += IncreaseSpeed;
			IsStartCombat = true;
		}
	});

	Anim->OnAttack.BindUObject(this, &AKhopeshCharacter::OnAttack);

	Anim->OnEndCombo.BindLambda([this]()
	{
		CurrentCombo = 0;
	});

	Speed = IncreaseSpeed;
}

void AKhopeshCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(
		GetCharacterMovement()->MaxWalkSpeed,
		Speed, DeltaSeconds * SpeedRate);

	if (!HasAuthority() || Anim->IsPlayMontage()) return;

	if (IsEnemyNear())
	{
		if (!IsCombatMode && !IsEquipingNow)
		{
			PlayEquip(true);
		}
	}
	else
	{
		if (IsCombatMode && !IsUnequipingNow)
		{
			PlayEquip(false);
		}
	}
}

void AKhopeshCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKhopeshCharacter, HP);
	DOREPLIFETIME(AKhopeshCharacter, Speed);
}

void AKhopeshCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

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

float AKhopeshCharacter::TakeDamage(
	float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	HP = FMath::Clamp<uint8>(HP - FinalDamage, 0, 100);

	if (HP == 0)
	{
		Destroy();
		return FinalDamage;
	}

	float HitDir = DamageCauser->GetActorRotation().Yaw;
	EMontage Montage = GetHitMontageByDir(HitDir > 180.0f ? HitDir - 360.0f : HitDir);
	Damage_Multicast(Montage);

	return FinalDamage;
}

void AKhopeshCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		Move(EAxis::X, Value);
	}
}

void AKhopeshCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		Move(EAxis::Y, Value);
	}
}

void AKhopeshCharacter::Attack()
{
	if (!IsCombatMode || Anim->IsPlayMontage()) return;

	FRotator NewRotation = GetRotationByAim();
	AttackImpl(NewRotation);
	Attack_Server(NewRotation);
}

void AKhopeshCharacter::Defense()
{

}

void AKhopeshCharacter::Step()
{
	if (!IsStartCombat || Anim->IsPlayMontage() || GetCharacterMovement()->IsFalling())
		return;

	FRotator NewRotation = GetRotationByInputKey();
	StepImpl(NewRotation);
	Step_Server(NewRotation);
}

void AKhopeshCharacter::OnAttack()
{
	if (!IsLocallyControlled()) return;

	FHitResult Out;

	GetWorld()->SweepSingleByObjectType(
		Out,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * AttackRange,
		FQuat::Identity,
		ECollisionChannel::ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRadius),
		FCollisionQueryParams(NAME_None, false, this)
	);

	if (Out.bBlockingHit)
	{
		ApplyDamage(Out.GetActor());
	}
}

void AKhopeshCharacter::Attack_Server_Implementation(FRotator NewRotation)
{
	Attack_Multicast(NewRotation);
}

bool AKhopeshCharacter::Attack_Server_Validate(FRotator NewRotation)
{
	return true;
}

void AKhopeshCharacter::Attack_Multicast_Implementation(FRotator NewRotation)
{
	if (!IsLocallyControlled())
	{
		AttackImpl(NewRotation);
	}
}

void AKhopeshCharacter::ApplyDamage_Implementation(AActor* DamagedActor)
{
	DamagedActor->TakeDamage(
		IsStrongMode ? StrongAttackDamage : WeakAttackDamage,
		FDamageEvent(),
		GetController(),
		this);
}

bool AKhopeshCharacter::ApplyDamage_Validate(AActor* DamagedActor)
{
	return DamagedActor != nullptr;
}

void AKhopeshCharacter::Damage_Multicast_Implementation(EMontage HitMontage)
{
	Anim->PlayMontage(HitMontage);
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
	if (!IsLocallyControlled())
	{
		StepImpl(NewRotation);
	}
}

void AKhopeshCharacter::PlayEquipMontage_Implementation(bool IsEquip)
{
	Anim->PlayMontage(IsEquip ? EMontage::EQUIP : EMontage::UNEQUIP);
}

void AKhopeshCharacter::WalkMode_Implementation()
{
	if (IsStartCombat)
	{
		Speed -= IncreaseSpeed;
	}
}

bool AKhopeshCharacter::WalkMode_Validate()
{
	return true;
}

void AKhopeshCharacter::RunMode_Implementation()
{
	if (IsStartCombat)
	{
		Speed += IncreaseSpeed;
	}
}

bool AKhopeshCharacter::RunMode_Validate()
{
	return true;
}

void AKhopeshCharacter::AttackImpl(const FRotator& NewRotation)
{
	SetActorRotation(NewRotation);
	Anim->PlayAttackMontage(IsStrongMode ? EMontage::ATTACK_STRONG : EMontage::ATTACK_WEAK, CurrentCombo);
	++CurrentCombo %= MaxCombo;
}

void AKhopeshCharacter::StepImpl(const FRotator& NewRotation)
{
	SetActorRotation(NewRotation);
	Anim->PlayMontage(IsCombatMode ? EMontage::DODGE_EQUIP : EMontage::DODGE_UNEQUIP);
}

void AKhopeshCharacter::Move(EAxis::Type Axis, float Value)
{
	FRotator Rotation = GetRotationByAim();
	FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(Axis);
	AddMovementInput(Direction, Value);
}

void AKhopeshCharacter::PlayEquip(bool IsEquip)
{
	PlayEquipMontage(IsEquip);
	IsEquipingNow = IsEquip;
	IsUnequipingNow = !IsEquip;
}

void AKhopeshCharacter::SetEquip(bool IsEquip)
{
	FName LeftWeaponSocket = IsEquip ? TEXT("equip_sword_l") : TEXT("unequip_sword_l");
	FName RightWeaponSocket = IsEquip ? TEXT("equip_sword_r") : TEXT("unequip_sword_r");

	LeftWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftWeaponSocket);
	RightWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightWeaponSocket);

	CurrentCombo = 0;
}

bool AKhopeshCharacter::IsEnemyNear() const
{
	return GetWorld()->OverlapAnyTestByObjectType(
		GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(CombatSwapRange),
		FCollisionQueryParams(NAME_None, false, this)
	);
}

FRotator AKhopeshCharacter::GetRotationByAim() const
{
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw = GetControlRotation().Yaw;
	return NewRotation;
}

FRotator AKhopeshCharacter::GetRotationByInputKey() const
{
	float Horizontal = GetInputAxisValue(TEXT("MoveRight"));
	float Vertical = GetInputAxisValue(TEXT("MoveForward"));
	FRotator Rotation = GetRotationByAim();

	if (Horizontal == 0.0f)
	{
		Rotation.Yaw += (Vertical == -1.0f) ? 180.0f : 0.0f;
	}
	else
	{
		Rotation.Yaw += (Horizontal * 90.0f) + (Horizontal * Vertical * -45.0f);
	}

	return Rotation;
}

EMontage AKhopeshCharacter::GetHitMontageByDir(float Dir) const
{
	EMontage Montage = (Dir > 0.0f ? EMontage::HIT_RIGHT : EMontage::HIT_LEFT);

	if (FMath::Abs(Dir) <= 45.0f)
	{
		Montage = EMontage::HIT_FRONT;
	}
	else if (FMath::Abs(Dir) >= 135.0f)
	{
		Montage = EMontage::HIT_BACK;
	}

	return Montage;
}