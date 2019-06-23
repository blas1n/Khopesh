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
#include "UnrealNetwork.h"
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
	GetCharacterMovement()->MaxWalkSpeed = IncreaseSpeed;
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

	Speed = IncreaseSpeed;
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
			Speed += IncreaseSpeed;
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

	if (!HasAuthority()) return;

	if (IsEnemyNear())
	{
		if (!Anim->IsPlayMontage() && !bFightMode && !bEquiping)
		{
			PlayEquip(true);
			bEquiping = true;
			bUnequiping = false;
		}
	}

	else
	{
		if (!Anim->IsPlayMontage() && bFightMode && !bUnequiping)
		{
			PlayEquip(false);
			bUnequiping = true;
			bEquiping = false;
		}
	}
}

void AKhopeshCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AKhopeshCharacter, Speed);
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
	if (!bStartFight || Anim->IsPlayMontage() || GetCharacterMovement()->IsFalling())
		return;

	FRotator NewRot = GetRotatorByInputKey();
	StepImpl(NewRot);
	Step_Server(NewRot);
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
	if (Role != ROLE_AutonomousProxy)
	{
		StepImpl(NewRotation);
	}
}

void AKhopeshCharacter::Attack()
{
	if (!bFightMode || Anim->IsPlayMontage()) return;

	AttackImpl();
	Attack_Server();
}

void AKhopeshCharacter::Attack_Server_Implementation()
{
	Attack_Multicast();
}

bool AKhopeshCharacter::Attack_Server_Validate()
{
	return true;
}

void AKhopeshCharacter::Attack_Multicast_Implementation()
{
	if (Role != ROLE_AutonomousProxy)
	{
		AttackImpl();
	}
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

void AKhopeshCharacter::WalkMode_Implementation()
{
	if (bStartFight)
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
	if (bStartFight)
	{
		Speed += IncreaseSpeed;
	}
}

bool AKhopeshCharacter::RunMode_Validate()
{
	return true;
}

void AKhopeshCharacter::OnAttack()
{
	AttackCheck();
}

void AKhopeshCharacter::AttackCheck_Implementation()
{
	TArray<FHitResult> Out;

	bool bResult = GetWorld()->SweepMultiByObjectType(
		Out,
		GetActorLocation(),
		GetActorLocation() + GetActorForwardVector() * AttackRange,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(AttackRange),
		FCollisionQueryParams(NAME_None, false, this)
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
	DrawDebugSphere(GetWorld(), GetActorLocation(), InFightRange, 32, FColor::Black);

	return GetWorld()->OverlapAnyTestByObjectType(
		GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Pawn),
		FCollisionShape::MakeSphere(InFightRange),
		FCollisionQueryParams(NAME_None, false, this)
	);
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

void AKhopeshCharacter::PlayEquip_Implementation(bool IsEquip)
{
	Anim->PlayMontage(IsEquip ? EMontage::EQUIP : EMontage::UNEQUIP);
}

void AKhopeshCharacter::AttackImpl()
{
	Anim->PlayAttackMontage(bStrongMode ? EMontage::ATTACK_STRONG : EMontage::ATTACK_WEAK, CurrentSection++);
	CurrentSection %= MaxSection;
}

void AKhopeshCharacter::StepImpl(const FRotator& NewRotation)
{
	SetActorRotation(NewRotation);
	Anim->PlayMontage(bFightMode ? EMontage::DODGE_EQUIP : EMontage::DODGE_UNEQUIP);
}