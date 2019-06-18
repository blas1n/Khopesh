// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "KhopeshGameMode.h"
#include "KhopeshCharacter.h"
#include "UObject/ConstructorHelpers.h"

AKhopeshGameMode::AKhopeshGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
