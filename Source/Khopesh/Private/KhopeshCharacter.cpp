// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "KhopeshCharacter.h"
#include "KhopeshPlayerController.h"
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
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

AKhopeshCharacter::AKhopeshCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->TargetArmLength = 450.0f;
	CameraBoom->bEnableCameraLag = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	LeftWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWeapon"));
	LeftWeapon->SetupAttachment(GetMesh(), TEXT("unequip_sword_l"));
	LeftWeapon->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	LeftWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	LeftWeapon->SetGenerateOverlapEvents(false);

	RightWeapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWeapon"));
	RightWeapon->SetupAttachment(GetMesh(), TEXT("unequip_sword_r"));
	RightWeapon->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_No;
	RightWeapon->SetCollisionProfileName(TEXT("NoCollision"));
	RightWeapon->SetGenerateOverlapEvents(false);
}

void AKhopeshCharacter::BeginPlay()
{
	Super::BeginPlay();

	Anim = Cast<UKhopeshAnimInstance>(GetMesh()->GetAnimInstance());

	UAnimMontage* BrokenMontage = Anim->Get(EMontage::BROKEN);
	BrokenPlayRate = BrokenMontage->GetPlayLength() / BrokenDuration;

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

	GetCharacterMovement()->MaxWalkSpeed = Speed = ReadySpeed;
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
	DOREPLIFETIME(AKhopeshCharacter, IsCombatMode);
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

	PlayerInputComponent->BindAction("Dodge", IE_Pressed, this, &AKhopeshCharacter::OnPressDodge);
	PlayerInputComponent->BindAction("Dodge", IE_Released, this, &AKhopeshCharacter::OnReleaseDodge);
}

float AKhopeshCharacter::TakeDamage(
	float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (IsDefensing && FMath::Abs(GetActorRotation().Yaw - DamageCauser->GetActorRotation().Yaw) >= 112.5f)
	{
		Break(Cast<AKhopeshCharacter>(DamageCauser));
		return 0.0f;
	}

	float FinalDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	HP = FMath::Clamp<float>(HP - FinalDamage, 0.0f, 100.0f);
	Cast<AKhopeshCharacter>(DamageCauser)->ApplyEnemyHP(HP);
	PlayHitSound();

	if (HP > 0.0f)
	{
		auto Dir = GetActorRotation().Yaw - DamageCauser->GetActorRotation().Yaw;
		Dir = (FMath::Abs(Dir) > 180.0f) ? (Dir - (360.0f * FMath::Sign(Dir))) : Dir;
		PlayHitMontage(GetHitMontageByDir(Dir));
		ShowHitEffect();
	}
	else { Die(); }

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

void AKhopeshCharacter::OnPressDodge()
{
	if (IsCombatMode)
	{
		IsReadyDodge = true;

		GetWorldTimerManager().SetTimer(DodgeTimer, [this]
		{
			Dodge_Request(GetRotationByInputKey(), true);
			IsReadyDodge = false;
		}, LongDodgeDelay, false);
	}
	else
	{
		Dodge_Request(GetRotationByInputKey(), true);
	}
}

void AKhopeshCharacter::OnReleaseDodge()
{
	if (IsCombatMode && IsReadyDodge)
	{
		Dodge_Request(GetRotationByInputKey(), false);
		GetWorldTimerManager().ClearTimer(DodgeTimer);
		IsReadyDodge = false;
	}
}

void AKhopeshCharacter::OnAttack()
{
	ShowAttackEffect();
	FHitResult Out;

	GetWorld()->SweepSingleByObjectType(
		Out,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * AttackRange,
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel1,
		FCollisionShape::MakeSphere(AttackRadius),
		FCollisionQueryParams(NAME_None, false, this)
	);

	if (Out.bBlockingHit)
	{
		bool IsStrongAttack = Anim->IsMontagePlay(EMontage::ATTACK_STRONG);
		float AttackDamage = IsStrongAttack ? StrongAttackDamage : WeakAttackDamage;
		auto const& HitNum = IsStrongAttack ? StrongAttackHitNum : WeakAttackHitNum;

		int32 Idx = CurrentCombo - 1;
		if (Idx < 0) Idx = HitNum.Num() - 1;

		AttackDamage /= HitNum[Idx];
		Out.GetActor()->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
	}
}

void AKhopeshCharacter::SetCombat(bool IsCombat)
{
	SetWeapon(IsCombat);
	IsCombatMode = IsCombat;
	CurrentCombo = 0;

	if (!IsStartCombat && IsCombat)
	{
		Speed = FightSpeed;
		IsStartCombat = true;
		ShowCombatEffect();
	}
}

void AKhopeshCharacter::Attack_Request_Implementation(FRotator NewRotation)
{
	if (!IsCombatMode || Anim->IsMontagePlay()) return;

	EMontage Montage = IsStrongMode ? EMontage::ATTACK_STRONG : EMontage::ATTACK_WEAK;
	FName Section = *FString::Printf(TEXT("Attack_%d"), ++CurrentCombo);
	Attack_Response(Montage, Section, NewRotation);
	GetWorldTimerManager().ClearTimer(ComboTimer);
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

void AKhopeshCharacter::Dodge_Request_Implementation(FRotator NewRotation, bool IsLongDodge)
{
	if (!IsStartCombat || !CanDodge() || Anim->IsMontagePlay() || GetCharacterMovement()->IsFalling())
		return;

	Dodge_Response(NewRotation, IsLongDodge);
	NextDodgeTime = GetWorld()->GetTimeSeconds() + DodgeDelay;
}

bool AKhopeshCharacter::Dodge_Request_Validate(FRotator NewRotation, bool IsLongDodge)
{
	return true;
}

void AKhopeshCharacter::Dodge_Response_Implementation(FRotator NewRotation, bool IsLongDodge)
{
	SetActorRotation(NewRotation);
	Anim->PlayMontage(IsLongDodge ? EMontage::DODGE_LONG : EMontage::DODGE_SHORT);
}

void AKhopeshCharacter::ShowCombatEffect_Implementation()
{
	OnShowCombatEffect();
}

void AKhopeshCharacter::ShowAttackEffect_Implementation()
{
	OnShowAttackEffect();
}

void AKhopeshCharacter::ShowHitEffect_Implementation()
{
	OnShowHitEffect();
}

void AKhopeshCharacter::ShowParryingEffect_Implementation()
{
	OnShowParryingEffect();
}

void AKhopeshCharacter::ApplyEnemyHP_Implementation(float HP)
{
	OnApplyEnemyHP(HP);
}

void AKhopeshCharacter::PlayHitSound_Implementation()
{
	OnPlayHitSound();
}

void AKhopeshCharacter::PlayHitMontage_Implementation(EMontage Montage)
{
	Anim->PlayMontage(Montage);
}

void AKhopeshCharacter::EndDefenseMontage_Implementation(bool IsSuccess, FRotator Rotator)
{
	Anim->JumpToSection(EMontage::DEFENSE, IsSuccess ? TEXT("Success") : TEXT("Fail"));

	if (IsSuccess)
	{
		SetActorRotation(Rotator);
	}
}

void AKhopeshCharacter::PlayBroken_Implementation()
{
	Anim->PlayMontage(EMontage::BROKEN);
	Anim->Montage_SetPlayRate(Anim->Get(EMontage::BROKEN), BrokenPlayRate);
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

void AKhopeshCharacter::PlayDie_Implementation()
{
	Anim->PlayMontage(EMontage::DIE);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (IsLocallyControlled())
	{
		DisableInput(Cast<APlayerController>(GetController()));
	}
}

void AKhopeshCharacter::Move(EAxis::Type Axis, float Value)
{
	FRotator Rotation = GetRotationByAim();
	FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(Axis);
	AddMovementInput(Direction, Value);
}

void AKhopeshCharacter::Break(AKhopeshCharacter* Target)
{
	GetWorldTimerManager().ClearTimer(DefenseTimer);
	auto Rotator = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
	EndDefenseMontage(true, Rotator);
	ShowParryingEffect();
	Target->PlayBroken();
	IsDefensing = false;
	IsStrongMode = true;

	GetWorldTimerManager().SetTimer(BrokenTimer, [this]()
	{
		IsStrongMode = false;
	}, BrokenDuration, false);
}

void AKhopeshCharacter::Die()
{
	auto MyController = Cast<AKhopeshPlayerController>(GetController());
	MyController->PlayerDead();
	PlayDie();
}

bool AKhopeshCharacter::CanDodge() const
{
	return (FMath::IsNearlyEqual(NextDodgeTime, 0.0f) || NextDodgeTime <= GetWorld()->GetTimeSeconds());
}

bool AKhopeshCharacter::IsEnemyNear() const
{
	return GetWorld()->OverlapAnyTestByObjectType(
		GetActorLocation(),
		FQuat::Identity,
		ECollisionChannel::ECC_GameTraceChannel1,
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
	UE_LOG(LogTemp, Warning, TEXT("After : %f"), Dir);
	EMontage Montage = (Dir > 0.0f) ? EMontage::HIT_LEFT : EMontage::HIT_RIGHT;

	if (FMath::Abs(Dir) <= 45.0f)
	{
		Montage = EMontage::HIT_BACK;
	}
	else if (FMath::Abs(Dir) >= 135.0f)
	{
		Montage = EMontage::HIT_FRONT;
	}

	return Montage;
}