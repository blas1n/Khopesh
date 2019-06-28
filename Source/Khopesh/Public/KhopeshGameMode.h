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
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

public:
	void PlayerDead(class AKhopeshPlayerController* DeadPlayer);

protected:
	UFUNCTION(BlueprintCallable)
	AActor* GetPlayerStart(AController* Player);

private:
	void ShowResult(class AKhopeshPlayerController* WinPlayer, class AKhopeshPlayerController* LosePlayer);

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, Meta = (AllowPrivateAccess = true))
	TArray<class AKhopeshPlayerController*> Players;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Player, Meta = (AllowPrivateAccess = true))
	TArray<AActor*> Spawns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Spawn, Meta = (AllowPrivateAccess = true))
	TMap<class AKhopeshPlayerController*, AActor*> TakenSpawns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Result, Meta = (AllowPrivateAccess = true))
	float ShowResultDelay;
};