// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshAnimInstance.h"
#include "KhopeshCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UKhopeshAnimInstance::UKhopeshAnimInstance()
{
	Speed = 0.0f;
	IsInAir = false;
	IsFightMode = false;
	EquipAnim = nullptr;
}

void UKhopeshAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UKhopeshAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto Owner = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(Owner)) return;

	Speed = Owner->GetVelocity().Size();
	IsInAir = Owner->GetCharacterMovement()->IsFalling();
}

void UKhopeshAnimInstance::SetFightMode(bool IsFight)
{
	IsFightMode = IsFight;
}

void UKhopeshAnimInstance::PlayMontage(UAnimMontage* Montage)
{
	Montage_Play(Montage);
	FOnMontageEnded MontageEndDelegate;
	MontageEndDelegate.BindUObject(this, &UKhopeshAnimInstance::MontageEnd);
	Montage_SetEndDelegate(MontageEndDelegate, Montage);
}

bool UKhopeshAnimInstance::PlayMontageUnique(UAnimMontage* Montage)
{
	if (!Montage_IsPlaying(nullptr))
	{
		PlayMontage(Montage);
		return true;
	}

	return false;
}

void UKhopeshAnimInstance::PlayMontageEquip(UAnimMontage* EquipMontage)
{
	if (!PlayMontageUnique(EquipMontage))
	{
		EquipAnim = EquipMontage;
	}
}

void UKhopeshAnimInstance::AnimNotify_Equip()
{
	auto Owner = Cast<AKhopeshCharacter>(TryGetPawnOwner());
	Owner->OnSetFightMode(true);
}

void UKhopeshAnimInstance::AnimNotify_Unequip()
{
	auto Owner = Cast<AKhopeshCharacter>(TryGetPawnOwner());
	Owner->OnSetFightMode(false);
}

void UKhopeshAnimInstance::MontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("Bind"));

	if (EquipAnim != nullptr)
	{
		Montage_Play(EquipAnim);
		EquipAnim = nullptr;
	}
}