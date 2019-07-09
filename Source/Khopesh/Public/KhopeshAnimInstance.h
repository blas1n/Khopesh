// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "KhopeshAnimInstance.generated.h"

DECLARE_DELEGATE(FOnAttack)
DECLARE_DELEGATE(FOnNextCombo)
DECLARE_DELEGATE_OneParam(FOnSetCombatMode, bool)

UENUM()
enum class EMontage : uint8
{
	ATTACK_WEAK,
	ATTACK_STRONG,
	DEFENSE,
	DODGE_SHORT,
	DODGE_LONG,
	HIT_FRONT,
	HIT_LEFT,
	HIT_BACK,
	HIT_RIGHT,
	BROKEN,
	EQUIP,
	UNEQUIP,
	START,
	DIE,
};

UCLASS()
class KHOPESH_API UKhopeshAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Constructor
	UKhopeshAnimInstance();

private:
	// Virtual Function
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// Public Function
	void PlayMontage(EMontage Montage);
	void JumpToSection(EMontage Montage, FName const& Section);

	bool IsMontagePlay() const;
	bool IsMontagePlay(EMontage Montage) const;
	UAnimMontage* Get(EMontage Montage) const;

private:
	// Binding Function
	UFUNCTION()
	void AnimNotify_Attack();

	UFUNCTION()
	void AnimNotify_NextCombo();

	UFUNCTION()
	void AnimNotify_Equip();

	UFUNCTION()
	void AnimNotify_Unequip();

public:
	// Delegate
	FOnAttack OnAttack;
	FOnNextCombo OnNextCombo;
	FOnSetCombatMode OnSetCombatMode;

private:
	// Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackWeak;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackStrong;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Defense;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DodgeShort;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DodgeLong;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* HitFront;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* HitLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* HitBack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* HitRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Broken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Equip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Unequip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Die;
	
	// Blueprint Property
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool IsInAir;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool IsCombatMode;

	// Animation Map
	TMap<EMontage, UAnimMontage*> MontageMap;
};
