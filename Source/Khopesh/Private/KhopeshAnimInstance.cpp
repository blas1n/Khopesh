// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshAnimInstance.h"
#include "KhopeshCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"

UKhopeshAnimInstance::UKhopeshAnimInstance()
{
	Speed = 0.0f;
	IsInAir = false;
	IsFightMode = false;
	bIsPlayMontage = false;
}

void UKhopeshAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MontageMap.Emplace(EMontage::ATTACK_WEAK, AttackWeak);
	MontageMap.Emplace(EMontage::ATTACK_STRONG, AttackStrong);
	MontageMap.Emplace(EMontage::DODGE_EQUIP, DodgeEquip);
	MontageMap.Emplace(EMontage::DODGE_UNEQUIP, DodgeUnequip);
	MontageMap.Emplace(EMontage::DEFENSE, Defense);
	MontageMap.Emplace(EMontage::HIT_WEAK, HitWeak);
	MontageMap.Emplace(EMontage::HIT_STRONG, HitStrong);
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

void UKhopeshAnimInstance::SetFightMode(bool IsFight)
{
	IsFightMode = IsFight;
}

void UKhopeshAnimInstance::PlayMontage(EMontage Montage)
{
	Montage_Play(MontageMap[Montage]);
	bIsPlayMontage = true;
}

void UKhopeshAnimInstance::PlayAttackMontage(EMontage Montage, uint8 Section)
{
	PlayMontage(Montage);
	Montage_JumpToSection(MontageMap[Montage]->GetSectionName(Section));
	
	TryGetPawnOwner()->GetWorldTimerManager().ClearTimer(ComboTimer);
}

void UKhopeshAnimInstance::AnimNotify_Equip()
{
	OnSetFightMode.Execute(true);
}

void UKhopeshAnimInstance::AnimNotify_Unequip()
{
	OnSetFightMode.Execute(false);
}

void UKhopeshAnimInstance::AnimNotify_Attack()
{
	OnAttack.Execute();
}

void UKhopeshAnimInstance::AnimNotify_NextCombo()
{
	bIsPlayMontage = false;

	TryGetPawnOwner()->GetWorldTimerManager().SetTimer(ComboTimer, [this]()
	{
		OnEndCombo.Execute();
	}, ComboDelay, false);
}

void UKhopeshAnimInstance::AnimNotify_EndMotion()
{
	bIsPlayMontage = false;
}