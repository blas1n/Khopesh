// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "KhopeshGameMode.h"
#include "TimerManager.h"
#include "KhopeshCharacter.h"
#include "KhopeshPlayerController.h"
#include "UObject/ConstructorHelpers.h"

void AKhopeshGameMode::PostLogin(APlayerController* NewPlayer)
{	
	Super::PostLogin(NewPlayer);

	auto Controller = Cast<AKhopeshPlayerController>(NewPlayer);

	if (Controller)
	{
		Players.Add(Controller);
	}
}

void AKhopeshGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	auto Controller = Cast<AKhopeshPlayerController>(Exiting);

	if (Controller)
	{
		Players.Remove(Controller);
	}
}

void AKhopeshGameMode::PlayerDead(AKhopeshPlayerController* DeadPlayer)
{
	check(Players.Num() == 2)
	
	AKhopeshPlayerController* WinPlayer = Players[!Players.Find(DeadPlayer)];
	AKhopeshPlayerController* LosePlayer = DeadPlayer;

	WinPlayer->SetUIInputMode();
	LosePlayer->SetUIInputMode();

	FTimerHandle Timer;
	GetWorldTimerManager().SetTimer(Timer, [this, WinPlayer, LosePlayer]()
	{
		ShowResult(WinPlayer, LosePlayer);
	}, ShowResultDelay, false);
}

void AKhopeshGameMode::ShowResult(AKhopeshPlayerController* WinPlayer, AKhopeshPlayerController* LosePlayer)
{
	WinPlayer->ShowResultWidget(WinText);
	LosePlayer->ShowResultWidget(LoseText);
}