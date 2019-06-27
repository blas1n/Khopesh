// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshPlayerController.h"
#include "KhopeshGameMode.h"
#include "Engine/World.h"

void AKhopeshPlayerController::PlayerDead()
{
	auto World = Cast<AKhopeshGameMode>(GetWorld()->GetAuthGameMode());
	World->PlayerDead(this);
}

void AKhopeshPlayerController::ShowResultWidget_Implementation(FText const& ResultText)
{
	OnShowResultWidget(ResultText);
}

void AKhopeshPlayerController::SetGameInputMode_Implementation()
{
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void AKhopeshPlayerController::SetUIInputMode_Implementation()
{
	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}