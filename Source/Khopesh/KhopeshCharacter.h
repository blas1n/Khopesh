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

public:
	AKhopeshCharacter();

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Step();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

private:
	void Move(EAxis::Type Axis, float Value);

private:
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, Meta = (AllowPrivateAccess = true))
	//class AWeapon* Weapon;
};

