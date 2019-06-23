// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "KhopeshAnimInstance.generated.h"

DECLARE_DELEGATE(FOnAttack);
DECLARE_DELEGATE(FOnEndCombo);
DECLARE_DELEGATE_OneParam(FOnSetFightMode, bool);

UENUM()
enum class EMontage : uint8
{
	ATTACK_WEAK UMETA(DisplayName = "Attack Weak"),
	ATTACK_STRONG UMETA(DisplayName = "Attack Strong"),
	DODGE_EQUIP UMETA(DisplayName = "Dodge Equip"),
	DODGE_UNEQUIP UMETA(DisplayName = "Dodge Unequip"),
	DEFENSE UMETA(DisplayName = "Defense"),
	HIT_WEAK UMETA(DisplayName = "Hit Weak"),
	HIT_STRONG UMETA(DisplayName = "Hit Strong"),
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
	UKhopeshAnimInstance();
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	void SetFightMode(bool IsFight);

	void PlayMontage(EMontage Montage);
	void PlayAttackMontage(EMontage Montage, uint8 Section);

	FORCEINLINE bool IsPlayMontage() const { return bIsPlayMontage; }

private:
	UFUNCTION()
	void AnimNotify_Equip();

	UFUNCTION()
	void AnimNotify_Unequip();

	UFUNCTION()
	void AnimNotify_Attack();

	UFUNCTION()
	void AnimNotify_NextCombo();

	UFUNCTION()
	void AnimNotify_EndMotion();

public:
	FOnAttack OnAttack;
	FOnEndCombo OnEndCombo;
	FOnSetFightMode OnSetFightMode;

private:
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
	UAnimMontage* HitWeak;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* HitStrong;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Broken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Equip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Unequip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation, Meta = (AllowPrivateAccess = true))
	UAnimMontage* Die;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	float Speed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool IsInAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Pawn, Meta = (AllowPrivateAccess = true))
	bool IsFightMode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack, Meta = (AllowPrivateAccess = true))
	float ComboDelay;

	TMap<EMontage, UAnimMontage*> MontageMap;

	FTimerHandle ComboTimer;
	bool bIsPlayMontage;
};
