// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshAnimInstance.h"
#include "KhopeshCharacter.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

UKhopeshAnimInstance::UKhopeshAnimInstance()
{
	Speed = 0.0f;
	IsInAir = false;
	IsCombatMode = false;
	IsMontagePlay = false;
	ComboDelay = 0.0f;
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

	ComboDelay = Cast<AKhopeshCharacter>(TryGetPawnOwner())->GetComboDelay();
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
	IsMontagePlay = true;
}

void UKhopeshAnimInstance::PlayAttackMontage(EMontage Montage, uint8 Section)
{
	PlayMontage(Montage);
	Montage_JumpToSection(MontageMap[Montage]->GetSectionName(Section));
	
	TryGetPawnOwner()->GetWorldTimerManager().ClearTimer(ComboTimer);
}

void UKhopeshAnimInstance::SetCombatMode(bool IsCombat)
{
	IsCombatMode = IsCombat;
}

void UKhopeshAnimInstance::AnimNotify_Attack()
{
	OnAttack.Execute();
}

void UKhopeshAnimInstance::AnimNotify_NextCombo()
{
	IsMontagePlay = false;

	TryGetPawnOwner()->GetWorldTimerManager().SetTimer(ComboTimer, [this]()
	{
		OnEndCombo.Execute();
	}, ComboDelay, false);
}

void UKhopeshAnimInstance::AnimNotify_Equip()
{
	OnSetCombatMode.Execute(true);
}

void UKhopeshAnimInstance::AnimNotify_Unequip()
{
	OnSetCombatMode.Execute(false);
}

void UKhopeshAnimInstance::AnimNotify_EndMotion()
{
	IsMontagePlay = false;
}