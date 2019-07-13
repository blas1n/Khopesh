// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_TimeSlow.generated.h"

/**
 * 
 */
UCLASS()
class KHOPESH_API UAnimNotify_TimeSlow : public UAnimNotifyState
{
	GENERATED_BODY()

private:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SlowTime, Meta = (AllowPrivateAccess = true))
	float SlowTime;
};
