// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "KhopeshCharacter.h"
#include "KhopeshAnimInstance.h"
#include "UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"

DECLARE_DELEGATE_OneParam(FSetMoveMode, int32)

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
}

void AKhopeshCharacter::BeginPlay()
{
	Super::BeginPlay();

	Anim = Cast<UKhopeshAnimInstance>(GetMesh()->GetAnimInstance());

	if (!HasAuthority()) return;

	Anim->OnSetCombatMode.BindUObject(this, &AKhopeshCharacter::SetCombat);
	Anim->OnAttack.BindUObject(this, &AKhopeshCharacter::OnAttack);
	Anim->OnNextCombo.BindLambda([this]()
	{
		GetWorldTimerManager().SetTimer(ComboTimer, [this]()
		{
			CurrentCombo = 0;
		}, ComboDuration, false);
	});

	Speed = IncreaseSpeed;
}

void AKhopeshCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	GetCharacterMovement()->MaxWalkSpeed = FMath::Lerp(
		GetCharacterMovement()->MaxWalkSpeed,
		Speed, DeltaSeconds * SpeedRate);

	if (!HasAuthority() || Anim->IsMontagePlay())
		return;

	bool IsCombat = IsEnemyNear();

	if (IsCombat && !IsCombatMode)
	{
		PlayEquip(true);
	}
	else if (!IsCombat && IsCombatMode)
	{
		PlayEquip(false);
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

	PlayerInputComponent->BindAction<FSetMoveMode>("Dash", IE_Pressed, this, &AKhopeshCharacter::SetMoveMode, 1);
	PlayerInputComponent->BindAction<FSetMoveMode>("Dash", IE_Released, this, &AKhopeshCharacter::SetMoveMode, -1);
}

float AKhopeshCharacter::TakeDamage(
	float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDefensing)
	{
		GetWorldTimerManager().ClearTimer(DefenseTimer);
		EndDefenseMontage(true);
		IsDefensing = false;
		IsStrongMode = true;
		Cast<AKhopeshCharacter>(DamageCauser)->PlayBroken();
		
		GetWorldTimerManager().SetTimer(BrokenTimer, [this, DamageCauser]()
		{
			IsStrongMode = false;
		}, BrokenDuration, false);

		return 0.0f;
	}

	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	HP = FMath::Clamp<uint8>(HP - FinalDamage, 0, 100);
	(HP == 0) ? Destroy() : PlayHitMontage(DamageCauser->GetActorRotation().Yaw);
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
	Attack_Request(GetRotationByAim());
}

void AKhopeshCharacter::Defense()
{
	Defense_Request(GetRotationByAim());
}

void AKhopeshCharacter::Step()
{
	Step_Request(GetRotationByInputKey());
}

void AKhopeshCharacter::OnAttack()
{
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
		float AttackDamage = Anim->IsMontagePlay(EMontage::ATTACK_STRONG) ? StrongAttackDamage : WeakAttackDamage;
		Out.GetActor()->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
	}
}

void AKhopeshCharacter::SetCombat(bool IsCombat)
{
	SetWeapon(IsCombat);
	IsCombatMode = IsCombat;
	CurrentCombo = 0;

	if (IsCombat && !IsStartCombat)
	{
		Speed += IncreaseSpeed;
		IsStartCombat = true;
	}
}

void AKhopeshCharacter::Attack_Request_Implementation(FRotator NewRotation)
{
	if (!IsCombatMode || Anim->IsMontagePlay()) return;

	EMontage Montage = IsStrongMode ? EMontage::ATTACK_STRONG : EMontage::ATTACK_WEAK;
	FName Section = *FString::Printf(TEXT("Attack_%d"), ++CurrentCombo);
	Attack_Response(Montage, Section, NewRotation);
	CurrentCombo %= MaxCombo;
	IsStrongMode = false;
}

bool AKhopeshCharacter::Attack_Request_Validate(FRotator NewRotation)
{
	return true;
}

void AKhopeshCharacter::Attack_Response_Implementation(EMontage Montage, FName Section, FRotator NewRotation)
{
	SetActorRotation(NewRotation);
	Anim->PlayMontage(Montage);
	Anim->JumpToSection(Montage, Section);
}

void AKhopeshCharacter::Defense_Request_Implementation(FRotator NewRotation)
{
	if (!IsCombatMode || Anim->IsMontagePlay()) return;

	Defense_Response(NewRotation);
	IsDefensing = true;

	GetWorldTimerManager().SetTimer(DefenseTimer, [this]()
	{
		EndDefenseMontage(false);
		IsDefensing = false;
	}, DefenseDuration, false);
}

bool AKhopeshCharacter::Defense_Request_Validate(FRotator NewRotation)
{
	return true;
}

void AKhopeshCharacter::Defense_Response_Implementation(FRotator NewRotation)
{
	SetActorRotation(NewRotation);
	Anim->PlayMontage(EMontage::DEFENSE);
}

void AKhopeshCharacter::Step_Request_Implementation(FRotator NewRotation)
{
	if (!IsStartCombat || Anim->IsMontagePlay() || GetCharacterMovement()->IsFalling())
		return;

	Step_Response(IsCombatMode ? EMontage::DODGE_EQUIP : EMontage::DODGE_UNEQUIP, NewRotation);
}

bool AKhopeshCharacter::Step_Request_Validate(FRotator NewRotation)
{
	return true;
}

void AKhopeshCharacter::Step_Response_Implementation(EMontage Montage, FRotator NewRotation)
{
	SetActorRotation(NewRotation);
	Anim->PlayMontage(Montage);
}

void AKhopeshCharacter::PlayHitMontage_Implementation(float Direction)
{
	Anim->PlayMontage(GetHitMontageByDir(Direction > 180.0f ? Direction - 360.0f : Direction));
}

void AKhopeshCharacter::EndDefenseMontage_Implementation(bool IsSuccess)
{
	Anim->JumpToSection(EMontage::DEFENSE, IsSuccess ? TEXT("Success") : TEXT("Fail"));
}

void AKhopeshCharacter::PlayBroken_Implementation()
{
	Anim->PlayMontage(EMontage::BROKEN);
}

void AKhopeshCharacter::PlayEquip_Implementation(bool IsEquip)
{
	Anim->PlayMontage(IsEquip ? EMontage::EQUIP : EMontage::UNEQUIP);
}

void AKhopeshCharacter::SetWeapon_Implementation(bool IsEquip)
{
	FName LeftWeaponSocket = IsEquip ? TEXT("equip_sword_l") : TEXT("unequip_sword_l");
	FName RightWeaponSocket = IsEquip ? TEXT("equip_sword_r") : TEXT("unequip_sword_r");

	LeftWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftWeaponSocket);
	RightWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightWeaponSocket);
}

void AKhopeshCharacter::SetMoveMode_Implementation(int32 MoveMode)
{
	if (IsStartCombat)
	{
		Speed += IncreaseSpeed * MoveMode;
	}
}

bool AKhopeshCharacter::SetMoveMode_Validate(int32 MoveMode)
{
	return MoveMode == 1 || MoveMode == -1;
}

void AKhopeshCharacter::Move(EAxis::Type Axis, float Value)
{
	FRotator Rotation = GetRotationByAim();
	FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(Axis);
	AddMovementInput(Direction, Value);
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