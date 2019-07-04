// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UKhopeshAnimInstance::UKhopeshAnimInstance()
{
	Speed = 0.0f;
	IsInAir = false;
	IsCombatMode = false;
}

void UKhopeshAnimInstance::NativeBeginPlay()
{
	Super::NativeInitializeAnimation();

	MontageMap.Emplace(EMontage::ATTACK_WEAK, AttackWeak);
	MontageMap.Emplace(EMontage::ATTACK_STRONG, AttackStrong);
	MontageMap.Emplace(EMontage::DODGE_EQUIP, DodgeEquip);
	MontageMap.Emplace(EMontage::DODGE_UNEQUIP, DodgeUnequip);
	MontageMap.Emplace(EMontage::DEFENSE, Defense);
	MontageMap.Emplace(EMontage::HIT_FRONT, HitFront);
	MontageMap.Emplace(EMontage::HIT_LEFT, HitLeft);
	MontageMap.Emplace(EMontage::HIT_BACK, HitBack);
	MontageMap.Emplace(EMontage::HIT_RIGHT, HitRight);
	MontageMap.Emplace(EMontage::BROKEN, Broken);
	MontageMap.Emplace(EMontage::EQUIP, Equip);
	MontageMap.Emplace(EMontage::UNEQUIP, Unequip);
	MontageMap.Emplace(EMontage::DIE, Die);
}

void UKhopeshAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto Owner = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(Owner)) return;

	Speed = Owner->GetVelocity().Size();
	IsInAir = Owner->GetCharacterMovement()->IsFalling();
}

void UKhopeshAnimInstance::PlayMontage(EMontage Montage)
{
	Montage_Play(MontageMap[Montage]);
}

void UKhopeshAnimInstance::JumpToSection(EMontage Montage, FName const& Section)
{
	Montage_JumpToSection(Section, MontageMap[Montage]);
}

bool UKhopeshAnimInstance::IsMontagePlay() const
{
	return Montage_IsPlaying(nullptr);
}

bool UKhopeshAnimInstance::IsMontagePlay(EMontage Montage) const
{
	return Montage_IsPlaying(MontageMap[Montage]);
}

UAnimMontage* UKhopeshAnimInstance::Get(EMontage Montage) const
{
	return MontageMap[Montage];
}

void UKhopeshAnimInstance::AnimNotify_Attack()
{
	OnAttack.ExecuteIfBound();
}

void UKhopeshAnimInstance::AnimNotify_NextCombo()
{
	Montage_Stop(0.25f, AttackWeak);
	Montage_Stop(0.25f, AttackStrong);
	OnNextCombo.ExecuteIfBound();
}

void UKhopeshAnimInstance::AnimNotify_Equip()
{
	OnSetCombatMode.ExecuteIfBound(true);
	IsCombatMode = true;
}

void UKhopeshAnimInstance::AnimNotify_Unequip()
{
	OnSetCombatMode.ExecuteIfBound(false);
	IsCombatMode = false;
}