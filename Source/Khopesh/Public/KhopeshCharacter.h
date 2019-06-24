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

	UFUNCTION(Server, Reliable, WithValidation)
	void Step_Server(FRotator NewRotation);

	void Step_Server_Implementation(FRotator NewRotation);
	bool Step_Server_Validate(FRotator NewRotation);

	UFUNCTION(NetMulticast, Reliable)
	void Step_Multicast(FRotator NewRotation);

	void Step_Multicast_Implementation(FRotator NewRotation);

private:
	virtual void BeginPlay() override;
	virtual void Tick(float DelatSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	void Attack();

	UFUNCTION(Server, Reliable, WithValidation)
	void Attack_Server();

	void Attack_Server_Implementation();
	bool Attack_Server_Validate();

	UFUNCTION(NetMulticast, Reliable)
	void Attack_Multicast();

	void Attack_Multicast_Implementation();

	void Defense();

	void Move(EAxis::Type Axis, float Value);
	void SetEquip(bool IsEquip);

	UFUNCTION(Server, Reliable, WithValidation)
	void WalkMode();

	void WalkMode_Implementation();
	bool WalkMode_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void RunMode();

	void RunMode_Implementation();
	bool RunMode_Validate();

	void OnAttack();

	bool IsEnemyNear() const;

	FRotator GetRotatorByInputKey() const;

	UFUNCTION(NetMulticast, Reliable)
	void PlayEquip(bool IsEquip);

	void PlayEquip_Implementation(bool IsEquip);

	UFUNCTION(NetMulticast, Reliable)
	void Damage_Multicast(bool IsStrongMode);

	void Damage_Multicast_Implementation(bool IsStrongMode);

	UFUNCTION(Server, Reliable, WithValidation)
	void RequestDamage(AActor* DamagedActor);

	void RequestDamage_Implementation(AActor* DamagedActor);
	bool RequestDamage_Validate(AActor* DamagedActor);

	void AttackImpl();
	void StepImpl(const FRotator& NewRotation);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Animation, Meta = (AllowPrivateAccess = true))
	class UKhopeshAnimInstance* Anim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage, Meta = (AllowPrivateAccess = true))
	TSubclassOf<class UDamageType> WeakDamageType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Damage, Meta = (AllowPrivateAccess = true))
	TSubclassOf<class UDamageType> StrongDamageType;

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

	constexpr static float IncreaseSpeed = 211.0f;

	UPROPERTY(Replicated)
	float Speed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Fight, Replicated, Meta = (AllowPrivateAccess = true))
	uint8 HP;

	bool bFightMode;
	bool bStrongMode;
	bool bStartFight;
	bool bEquiping;
	bool bUnequiping;

	constexpr static uint8 MaxSection = 5;
	uint8 CurrentSection;
};

