// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KhopeshGameMode.generated.h"

UCLASS(minimalapi)
class AKhopeshGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

public:
	void PlayerDead(class AKhopeshPlayerController* DeadPlayer);

private:
	void ShowResult(class AKhopeshPlayerController* WinPlayer, class AKhopeshPlayerController* LosePlayer);

private:
	TArray<class AKhopeshPlayerController*> Players;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Result, Meta = (AllowPrivateAccess = true))
	FText WinText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Result, Meta = (AllowPrivateAccess = true))
	FText LoseText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Result, Meta = (AllowPrivateAccess = true))
	float ShowResultDelay;
};