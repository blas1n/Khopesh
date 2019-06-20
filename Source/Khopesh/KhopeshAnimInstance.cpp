// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshAnimInstance.h"
#include "KhopeshCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UKhopeshAnimInstance::UKhopeshAnimInstance()
{
	Speed = 0.0f;
	IsInAir = false;
	IsFightMode = false;
}

void UKhopeshAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto Owner = Cast<AKhopeshCharacter>(TryGetPawnOwner());
	if (!IsValid(Owner)) return;

	Speed = Owner->GetSpeed();
	IsInAir = Owner->GetCharacterMovement()->IsFalling();
}

void UKhopeshAnimInstance::SetFightMode(bool IsFight)
{
	IsFightMode = IsFight;
}

void UKhopeshAnimInstance::PlayMontageUnique(UAnimMontage* Montage)
{
	if (!Montage_IsPlaying(nullptr))
		Montage_Play(Montage);
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