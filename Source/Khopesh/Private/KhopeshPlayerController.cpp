// Fill out your copyright notice in the Description page of Project Settings.

#include "KhopeshPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "KhopeshGameMode.h"
#include "UnrealNetwork.h"
#include "Engine/World.h"

void AKhopeshPlayerController::BeginPlay()
{
	Super::BeginPlay();
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

void AKhopeshPlayerController::BlockInput_Implementation()
{
	StopMovement();
	SetIgnoreMoveInput(true);
	SetIgnoreLookInput(true);
	SetInputMode(FInputModeUIOnly());
	bShowMouseCursor = true;
}