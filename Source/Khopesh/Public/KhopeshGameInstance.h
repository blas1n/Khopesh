// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "KhopeshGameInstance.generated.h"

UCLASS()
class KHOPESH_API UKhopeshGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UKhopeshGameInstance(FObjectInitializer const& ObjectInitializer);
};
