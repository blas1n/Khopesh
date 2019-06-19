// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshAnimInstance.h"
#include "GameFramework/Character.h"
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

	auto Owner = Cast<ACharacter>(TryGetPawnOwner());
	if (!IsValid(Owner)) return;

	Speed = Owner->GetVelocity().Size();
	IsInAir = Owner->GetCharacterMovement()->IsFalling();
}

void UKhopeshAnimInstance::EnableFightMode()
{
	IsFightMode = true;
}

void UKhopeshAnimInstance::PlayMontage(UAnimMontage* Montage)
{
	Montage_Play(Montage);
}