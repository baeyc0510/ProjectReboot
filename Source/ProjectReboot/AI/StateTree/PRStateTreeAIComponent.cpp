// Fill out your copyright notice in the Description page of Project Settings.

#include "PRStateTreeAIComponent.h"
#include "StateTree.h"

UPRStateTreeAIComponent::UPRStateTreeAIComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bStartLogicAutomatically = false;
}

void UPRStateTreeAIComponent::SetStateTreeAsset(UStateTree* InStateTree)
{
	if (!IsValid(InStateTree))
	{
		UE_LOG(LogTemp, Warning, TEXT("UPRStateTreeComponent: Attempted to set a null StateTree."));
		return;
	}

	// 기존 로직 중지
	const bool bWasRunning = IsRunning();
	if (bWasRunning)
	{
		StopLogic(TEXT("Swapping StateTree Asset"));
	}
	
	StateTreeRef.SetStateTree(InStateTree);

	// 로직 재시작
	if (bWasRunning)
	{
		StartLogic();	
	}
}
