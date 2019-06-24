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
	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = true))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = true))
	class UStaticMeshComponent* LeftWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = true))
	class UStaticMeshComponent* RightWeapon;

public:
	// Constructor
	AKhopeshCharacter();

private:
	// Virtual Function
	virtual void BeginPlay() override;
	virtual void Tick(float DelatSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// Binding Function (Excluding dash bind function because it include RPC)
	void MoveForward(float Value);
	void MoveRight(float Value);

	void Attack();
	void Defense();
	void Step();

	void OnAttack();

	// RPC Function Declaration
	UFUNCTION(Server, Reliable, WithValidation)
	void Attack_Server(FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Attack_Multicast(FRotator NewRotation);

	UFUNCTION(Server, Reliable, WithValidation)
	void ApplyDamage(AActor* DamagedActor);

	UFUNCTION(NetMulticast, Reliable)
	void Damage_Multicast(EMontage HitMontage);

	UFUNCTION(Server, Reliable, WithValidation)
	void Step_Server(FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Step_Multicast(FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void PlayEquipMontage(bool IsEquip);

	UFUNCTION(Server, Reliable, WithValidation)
	void WalkMode();

	UFUNCTION(Server, Reliable, WithValidation)
	void RunMode();

	// RPC Function Implementation
	void Attack_Server_Implementation(FRotator NewRotation);
	bool Attack_Server_Validate(FRotator NewRotation);

	void Attack_Multicast_Implementation(FRotator NewRotation);

	void ApplyDamage_Implementation(AActor* DamagedActor);
	bool ApplyDamage_Validate(AActor* DamagedActor);

	void Damage_Multicast_Implementation(EMontage HitMontage);

	void Step_Server_Implementation(FRotator NewRotation);
	bool Step_Server_Validate(FRotator NewRotation);

	void Step_Multicast_Implementation(FRotator NewRotation);

	void PlayEquipMontage_Implementation(bool IsEquip);

	void WalkMode_Implementation();
	bool WalkMode_Validate();

	void RunMode_Implementation();
	bool RunMode_Validate();

	// RPC Function's Implement Function
	void AttackImpl(const FRotator& NewRotation);
	void StepImpl(const FRotator& NewRotation);

	// Other Function
	void Move(EAxis::Type Axis, float Value);
	void PlayEquip(bool IsEquip);
	void SetEquip(bool IsEquip);

	bool IsEnemyNear() const;

	FRotator GetRotationByAim() const;
	FRotator GetRotationByInputKey() const;
	EMontage GetHitMontageByDir(float Dir) const;

public:
	// Getters
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE float GetComboDelay() const { return ComboDelay; }

private:
	// Blueprint Property
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Replicated, Meta = (AllowPrivateAccess = true))
	uint8 HP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float CombatSwapRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float AttackRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float WeakAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float StrongAttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	uint8 MaxCombo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float ComboDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float IncreaseSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float SpeedRate;

	// Replicated Property (HP exclude here. Because it include Blueprint Property.)
	UPROPERTY(Replicated)
	float Speed;

	// Animation Instance
	class UKhopeshAnimInstance* Anim;

	// Other Variable
	uint8 CurrentCombo;

	// Flag Variable
	bool IsCombatMode;
	bool IsStrongMode;
	bool IsStartCombat;
	bool IsEquipingNow;
	bool IsUnequipingNow;
};

