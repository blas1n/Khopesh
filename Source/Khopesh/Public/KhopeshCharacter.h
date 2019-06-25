// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "KhopeshCharacter.generated.h"

enum class EMontage : uint8;

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
	void SetCombat(bool IsEquip);

	// RPC Function Declaration
	UFUNCTION(Server, Reliable, WithValidation)
	void Attack_Request(FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Attack_Response(EMontage Montage, FName Section, FRotator NewRotation);

	UFUNCTION(Server, Reliable, WithValidation)
	void Defense_Request(FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Defense_Response(FRotator NewRotation);

	UFUNCTION(Server, Reliable, WithValidation)
	void Step_Request(FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Step_Response(EMontage Montage, FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void PlayHitMontage(float Direction);

	UFUNCTION(NetMulticast, Reliable)
	void EndDefenseMontage(bool IsSuccess);

	UFUNCTION(NetMulticast, Reliable)
	void PlayBroken();

	UFUNCTION(NetMulticast, Reliable)
	void PlayEquip(bool IsEquip);

	UFUNCTION(NetMulticast, Reliable)
	void SetWeapon(bool IsEquip);

	UFUNCTION(Server, Reliable, WithValidation)
	void SetMoveMode(int32 MoveMode);

	// RPC Function Implementation
	void Attack_Request_Implementation(FRotator NewRotation);
	bool Attack_Request_Validate(FRotator NewRotation);
	void Attack_Response_Implementation(EMontage Montage, FName Section, FRotator NewRotation);

	void Defense_Request_Implementation(FRotator NewRotation);
	bool Defense_Request_Validate(FRotator NewRotation);
	void Defense_Response_Implementation(FRotator NewRotation);

	void Step_Request_Implementation(FRotator NewRotation);
	bool Step_Request_Validate(FRotator NewRotation);
	void Step_Response_Implementation(EMontage Montage, FRotator NewRotation);

	void PlayHitMontage_Implementation(float Direction);
	void EndDefenseMontage_Implementation(bool IsSuccess);
	void PlayBroken_Implementation();
	void PlayEquip_Implementation(bool IsEquip);
	void SetWeapon_Implementation(bool IsEquip);

	void SetMoveMode_Implementation(int32 MoveMode);
	bool SetMoveMode_Validate(int32 MoveMode);

	// Other Function
	void Move(EAxis::Type Axis, float Value);
	bool IsEnemyNear() const;
	FRotator GetRotationByAim() const;
	FRotator GetRotationByInputKey() const;
	EMontage GetHitMontageByDir(float Dir) const;

public:
	// Getters
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	// Animation Instance
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, Meta = (AllowPrivateAccess = true))
	class UKhopeshAnimInstance* Anim;

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
	uint8 CurrentCombo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	uint8 MaxCombo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float ComboDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float DefenseDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float BrokenDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float IncreaseSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat, Meta = (AllowPrivateAccess = true))
	float SpeedRate;

	// Replicated Property (HP exclude here. Because it include Blueprint Property.)
	UPROPERTY(Replicated)
	float Speed;

	// Timer Handle
	FTimerHandle ComboTimer, DefenseTimer, BrokenTimer;

	// Flag Variable
	bool IsCombatMode;
	bool IsStrongMode;
	bool IsStartCombat;
	bool IsDefensing;
};