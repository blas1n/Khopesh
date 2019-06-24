// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "KhopeshAnimInstance.generated.h"

DECLARE_DELEGATE(FOnAttack);
DECLARE_DELEGATE(FOnEndCombo);
DECLARE_DELEGATE_OneParam(FOnSetCombatMode, bool);

UENUM()
enum class EMontage : uint8
{
	ATTACK_WEAK UMETA(DisplayName = "Attack Weak"),
	ATTACK_STRONG UMETA(DisplayName = "Attack Strong"),
	DODGE_EQUIP UMETA(DisplayName = "Dodge Equip"),
	DODGE_UNEQUIP UMETA(DisplayName = "Dodge Unequip"),
	DEFENSE UMETA(DisplayName = "Defense"),
	HIT_FRONT UMETA(DisplayName = "Hit Front"),
	HIT_LEFT UMETA(DisplayName = "Hit Left"),
	HIT_BACK UMETA(DisplayName = "Hit Back"),
	HIT_RIGHT UMETA(DisplayName = "Hit Right"),
	BROKEN UMETA(DisplayName = "Broken"),
	EQUIP UMETA(DisplayName = "Equip"),
	UNEQUIP UMETA(DisplayName = "Unequip"),
	DIE UMETA(DisplayName = "Die"),
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
	void PlayAttackMontage(EMontage Montage, uint8 Section);
	void SetCombatMode(bool IsCombat);
	
	// Getter
	FORCEINLINE bool IsPlayMontage() const { return IsMontagePlay; }

private:
	// Animation Notify
	UFUNCTION()
	void AnimNotify_Attack();

	UFUNCTION()
	void AnimNotify_NextCombo();

	UFUNCTION()
	void AnimNotify_Equip();

	UFUNCTION()
	void AnimNotify_Unequip();

	UFUNCTION()
	void AnimNotify_EndMotion();

public:
	// Delegate
	FOnAttack OnAttack;
	FOnEndCombo OnEndCombo;
	FOnSetCombatMode OnSetCombatMode;

private:
	// Animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackWeak;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* AttackStrong;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DodgeEquip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* DodgeUnequip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Defense;

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool IsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool IsCombatMode;

	// Other Variable
	TMap<EMontage, UAnimMontage*> MontageMap;

	FTimerHandle ComboTimer;
	bool IsMontagePlay;
	float ComboDelay;
};
