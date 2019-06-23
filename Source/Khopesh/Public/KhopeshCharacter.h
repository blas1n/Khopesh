// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KhopeshCharacter.generated.h"

UCLASS(config=Game)
class AKhopeshCharacter : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = true))
	class UStaticMeshComponent* LeftWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = true))
	class UStaticMeshComponent* RightWeapon;

public:
	AKhopeshCharacter();

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Step();

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DelatSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	void Attack();
	void Defense();

	void Move(EAxis::Type Axis, float Value);
	void SetEquip(bool IsEquip);

	void WalkMode();
	void RunMode();

	void OnAttack();
	bool IsEnemyNear();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, Meta = (AllowPrivateAccess = true))
	class UKhopeshAnimInstance* Anim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Fight, Meta = (AllowPrivateAccess = true))
	float InFightRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Fight, Meta = (AllowPrivateAccess = true))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Fight, Meta = (AllowPrivateAccess = true))
	float AttackRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Fight, Meta = (AllowPrivateAccess = true))
	float WeakAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Fight, Meta = (AllowPrivateAccess = true))
	float StrongAttackDamage;

	float Speed;

	bool bFightMode;
	bool bStrongMode;
	bool bStartFight;
	bool bEquiping;
	bool bUnequiping;

	constexpr static uint8 MaxSection = 5;
	uint8 CurrentSection;
};

