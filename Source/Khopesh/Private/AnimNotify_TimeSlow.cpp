// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimNotify_TimeSlow.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_TimeSlow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	UGameplayStatics::SetGlobalTimeDilation(MeshComp->GetOwner()->GetWorld(), TotalDuration / SlowTime);
}

void UAnimNotify_TimeSlow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	UGameplayStatics::SetGlobalTimeDilation(MeshComp->GetOwner()->GetWorld(), 1.0f);
}