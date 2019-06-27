// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KhopeshPlayerController.generated.h"

UCLASS()
class KHOPESH_API AKhopeshPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Client, Reliable)
	void ShowResultWidget(FText const& ResultText);

	UFUNCTION(Client, Reliable)
	void SetGameInputMode();

	UFUNCTION(Client, Reliable)
	void SetUIInputMode();

	void PlayerDead();

private:
	void ShowResultWidget_Implementation(FText const& ResultText);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnShowResultWidget(FText const& ResultText);

private:
	void SetGameInputMode_Implementation();
	void SetUIInputMode_Implementation();
};
