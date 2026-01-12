// Fill out your copyright notice in the Description page of Project Settings.


#include "PREquipmentManagerComponent.h"

#include "RogueliteSubsystem.h"
#include "ProjectReboot/Roguelite/PREquipActionData.h"


// Sets default values for this component's properties
UPREquipmentManagerComponent::UPREquipmentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UPREquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	BindToRogueliteSubsystem();
	
	// TODO: 이미 획득한 액션 목록 처리
	
	
}

void UPREquipmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UnbindFromRogueliteSubsystem();
}

void UPREquipmentManagerComponent::HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPREquipActionData* EquipActionData = Cast<UPREquipActionData>(Action);
	if (!IsValid(EquipActionData))
	{
		return;
	}
	
	
}

void UPREquipmentManagerComponent::HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPREquipActionData* EquipActionData = Cast<UPREquipActionData>(Action);
	if (!IsValid(EquipActionData))
	{
		return;
	}
}

void UPREquipmentManagerComponent::HandleActionStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPREquipActionData* EquipActionData = Cast<UPREquipActionData>(Action);
	if (!IsValid(EquipActionData))
	{
		return;
	}
	
	
}

void UPREquipmentManagerComponent::HandleRunEnded(bool bCompleted)
{
}


void UPREquipmentManagerComponent::BindToRogueliteSubsystem()
{
	URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this);
	if (!IsValid(RogueliteSubsystem))
	{
		return;
	}

	ActionAcquiredDelegateHandle =  RogueliteSubsystem->BindActionAcquiredByTags(EquipmentActionTags,
		FRogueliteActionFilteredSignature::FDelegate::CreateUObject(this, &ThisClass::HandleActionAcquired));
	ActionRemovedDelegateHandle = RogueliteSubsystem->BindActionRemovedByTags(EquipmentActionTags,
		FRogueliteActionFilteredSignature::FDelegate::CreateUObject(this, &ThisClass::HandleActionRemoved));
	ActionStackChangedDelegateHandle = RogueliteSubsystem->BindStackChangedByTags(EquipmentActionTags,
		FRogueliteActionFilteredSignature::FDelegate::CreateUObject(this,&ThisClass::HandleActionStackChanged));
	
	RogueliteSubsystem->OnRunEnded.AddDynamic(this, &ThisClass::HandleRunEnded);
	
	bIsBoundToSubsystem = true;
}

void UPREquipmentManagerComponent::UnbindFromRogueliteSubsystem()
{
	if (!bIsBoundToSubsystem)
	{
		return;
	}
	
	URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this);
	if (!IsValid(RogueliteSubsystem))
	{
		return;
	}
	
	RogueliteSubsystem->UnbindActionAcquiredByTags(EquipmentActionTags, ActionAcquiredDelegateHandle);
	RogueliteSubsystem->UnbindActionRemovedByTags(EquipmentActionTags, ActionRemovedDelegateHandle);
	RogueliteSubsystem->UnbindStackChangedByTags(EquipmentActionTags, ActionStackChangedDelegateHandle);
	RogueliteSubsystem->OnRunEnded.RemoveDynamic(this, &ThisClass::HandleRunEnded);
}