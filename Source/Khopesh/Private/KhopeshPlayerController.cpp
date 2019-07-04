// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "KhopeshGameMode.h"
#include "UnrealNetwork.h"
#include "Engine/World.h"

void AKhopeshPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetGameInputMode();
}

void AKhopeshPlayerController::PlayerDead()
{
	auto World = Cast<AKhopeshGameMode>(GetWorld()->GetAuthGameMode());
	World->PlayerDead(this);
}

void AKhopeshPlayerController::BackToLobby()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Lobby"));
}

void AKhopeshPlayerController::ShowResultWidget_Implementation(bool IsWin)
{
	OnShowResultWidget(IsWin);
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