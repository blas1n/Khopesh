// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "KhopeshPlayerController.generated.h"

UCLASS()
class KHOPESH_API AKhopeshPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	virtual void BeginPlay() override;
	
public:
	UFUNCTION(Client, Reliable)
	void ShowResultWidget(bool IsWin);

	UFUNCTION(Client, Reliable)
	void BlockInput();

	UFUNCTION(BlueprintCallable)
	void BackToLobby();

	void PlayerDead();

private:
	void ShowResultWidget_Implementation(bool IsWin);
	void BlockInput_Implementation();

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void OnShowResultWidget(bool IsWin);
};