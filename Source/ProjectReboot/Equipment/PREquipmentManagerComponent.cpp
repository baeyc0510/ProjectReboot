// Fill out your copyright notice in the Description page of Project Settings.


#include "PREquipmentManagerComponent.h"

#include "RogueliteSubsystem.h"
#include "ProjectReboot/Roguelite/PRRogueliteEquipActionData.h"


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
}

void UPREquipmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	UnbindFromRogueliteSubsystem();
}

void UPREquipmentManagerComponent::HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPRRogueliteEquipActionData* EquipActionData = Cast<UPRRogueliteEquipActionData>(Action);
	if (!IsValid(EquipActionData))
	{
		return;
	}
	
	
}

void UPREquipmentManagerComponent::HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPRRogueliteEquipActionData* EquipActionData = Cast<UPRRogueliteEquipActionData>(Action);
	if (!IsValid(EquipActionData))
	{
		return;
	}
	
	
}

void UPREquipmentManagerComponent::HandleStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPRRogueliteEquipActionData* EquipActionData = Cast<UPRRogueliteEquipActionData>(Action);
	if (!IsValid(EquipActionData))
	{
		return;
	}
	
	
}

void UPREquipmentManagerComponent::HandleRunEnded(bool bCompleted)
{
}

bool UPREquipmentManagerComponent::EquipActionToSlot(UPRRogueliteEquipActionData* Action, FGameplayTag SlotTag)
{
	return false;
}

void UPREquipmentManagerComponent::UnequipActionFromSlot(UPRRogueliteEquipActionData* Action,
	FGameplayTag SlotTag)
{
}

TArray<UPRRogueliteEquipActionData*> UPREquipmentManagerComponent::GetSlotContents(FGameplayTag SlotTag) const
{
	return TArray<UPRRogueliteEquipActionData*>();
}

int32 UPREquipmentManagerComponent::GetSlotCount(FGameplayTag SlotTag) const
{
	return 0;
}

bool UPREquipmentManagerComponent::IsSlotFull(FGameplayTag SlotTag, int32 MaxCount) const
{
	return false;
}


void UPREquipmentManagerComponent::BindToRogueliteSubsystem()
{
	URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this);
	if (!IsValid(RogueliteSubsystem))
	{
		return;
	}

	// TODO: Tag기반으로 특정 이벤트만 수신
	RogueliteSubsystem->OnActionAcquired.AddDynamic(this, &ThisClass::HandleActionAcquired);
	RogueliteSubsystem->OnActionRemoved.AddDynamic(this, &ThisClass::HandleActionRemoved);
	RogueliteSubsystem->OnStackChanged.AddDynamic(this, &ThisClass::HandleStackChanged);
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
	
	RogueliteSubsystem->OnActionAcquired.RemoveDynamic(this, &ThisClass::HandleActionAcquired);
	RogueliteSubsystem->OnActionRemoved.RemoveDynamic(this, &ThisClass::HandleActionRemoved);
	RogueliteSubsystem->OnStackChanged.RemoveDynamic(this, &ThisClass::HandleStackChanged);
	RogueliteSubsystem->OnRunEnded.RemoveDynamic(this, &ThisClass::HandleRunEnded);
}