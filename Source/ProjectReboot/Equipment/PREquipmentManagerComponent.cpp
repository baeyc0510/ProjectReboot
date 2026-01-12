// Fill out your copyright notice in the Description page of Project Settings.


#include "PREquipmentManagerComponent.h"

#include "EquipmentInstance.h"
#include "RogueliteBlueprintLibrary.h"
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

	BindToRogueliteSubsystem();
	
	// TODO: 이미 획득한 액션 목록 처리
	ProcessExistingActions();
}

void UPREquipmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnequipAll();
	UnbindFromRogueliteSubsystem();
	
	Super::EndPlay(EndPlayReason);
}


void UPREquipmentManagerComponent::Equip(UPREquipActionData* ActionData)
{
	if (!ActionData)
	{
		return;
	}

	const FGameplayTag SlotTag = ActionData->EquipmentSlot;

	if (Slots.Contains(SlotTag))
	{
		return;
	}

	if (ActionData->bAttachToParentEquipment)
	{
		UEquipmentInstance* ParentInstance = GetInstance(ActionData->ParentEquipmentSlot);
		if (!ParentInstance)
		{
			return;
		}

		ParentInstance->AddVisual(ActionData);

		FEquipmentSlotEntry Entry;
		Entry.Instance = ParentInstance;
		Entry.ActionData = ActionData;
		Slots.Add(SlotTag, Entry);

		RefreshAllVisuals();

		OnEquipped.Broadcast(SlotTag, ParentInstance, ActionData);
	}
	else
	{
		UEquipmentInstance* Instance = CreateInstance(ActionData);
		Instance->Initialize(GetAttachTarget(), ActionData);

		FEquipmentSlotEntry Entry;
		Entry.Instance = Instance;
		Entry.ActionData = ActionData;
		Slots.Add(SlotTag, Entry);

		OnEquipped.Broadcast(SlotTag, Instance, ActionData);
	}
}

void UPREquipmentManagerComponent::Unequip(FGameplayTag SlotTag)
{
	FEquipmentSlotEntry* Entry = Slots.Find(SlotTag);
	if (!Entry)
	{
		return;
	}

	UnequipChildren(SlotTag);

	UEquipmentInstance* Instance = Entry->Instance;
	UPREquipActionData* ActionData = Entry->ActionData;

	if (IsParentEquipmentSlot(SlotTag))
	{
		Instance->Uninitialize();
	}
	else
	{
		Instance->RemoveVisual(ActionData);
		RefreshAllVisuals();
	}

	Slots.Remove(SlotTag);

	OnUnequipped.Broadcast(SlotTag, Instance, ActionData);
}

void UPREquipmentManagerComponent::UnequipAll()
{
	TArray<FGameplayTag> SlotTags;
	Slots.GetKeys(SlotTags);

	// Children first
	SlotTags.Sort([this](const FGameplayTag& A, const FGameplayTag& B)
	{
		const FEquipmentSlotEntry* EntryA = Slots.Find(A);
		const FEquipmentSlotEntry* EntryB = Slots.Find(B);

		bool bAIsChild = EntryA && EntryA->ActionData && EntryA->ActionData->bAttachToParentEquipment;
		bool bBIsChild = EntryB && EntryB->ActionData && EntryB->ActionData->bAttachToParentEquipment;

		return bAIsChild && !bBIsChild;
	});

	for (const FGameplayTag& SlotTag : SlotTags)
	{
		Unequip(SlotTag);
	}
}

UEquipmentInstance* UPREquipmentManagerComponent::GetInstance(FGameplayTag SlotTag) const
{
	const FEquipmentSlotEntry* Entry = Slots.Find(SlotTag);
	return Entry ? Entry->Instance : nullptr;
}

UPREquipActionData* UPREquipmentManagerComponent::GetActionData(FGameplayTag SlotTag) const
{
	const FEquipmentSlotEntry* Entry = Slots.Find(SlotTag);
	return Entry ? Entry->ActionData : nullptr;
}

TArray<UEquipmentInstance*> UPREquipmentManagerComponent::GetAllInstances() const
{
	TArray<UEquipmentInstance*> Instances;

	for (const auto& Pair : Slots)
	{
		if (Pair.Value.Instance && !Instances.Contains(Pair.Value.Instance))
		{
			Instances.Add(Pair.Value.Instance);
		}
	}

	return Instances;
}

bool UPREquipmentManagerComponent::HasEquipment(FGameplayTag SlotTag) const
{
	return Slots.Contains(SlotTag);
}

bool UPREquipmentManagerComponent::IsParentEquipmentSlot(FGameplayTag SlotTag) const
{
	const FEquipmentSlotEntry* Entry = Slots.Find(SlotTag);
	return Entry && Entry->ActionData && !Entry->ActionData->bAttachToParentEquipment;
}

UEquipmentInstance* UPREquipmentManagerComponent::CreateInstance(UPREquipActionData* ActionData)
{
	TSubclassOf<UEquipmentInstance> InstanceClass = ActionData->EquipmentInstanceType;
	if (!InstanceClass)
	{
		InstanceClass = UEquipmentInstance::StaticClass();
	}

	return NewObject<UEquipmentInstance>(GetOwner(), InstanceClass);
}

USceneComponent* UPREquipmentManagerComponent::GetAttachTarget() const
{
	return nullptr;
}

void UPREquipmentManagerComponent::RefreshAllVisuals()
{
}

void UPREquipmentManagerComponent::UnequipChildren(FGameplayTag ParentSlotTag)
{
	TArray<FGameplayTag> ChildSlots = FindChildSlots(ParentSlotTag);
	for (const FGameplayTag& ChildSlot : ChildSlots)
	{
		Unequip(ChildSlot);
	}
}

TArray<FGameplayTag> UPREquipmentManagerComponent::FindChildSlots(FGameplayTag ParentSlotTag) const
{
	TArray<FGameplayTag> ChildSlots;

	for (const auto& Pair : Slots)
	{
		if (Pair.Value.ActionData &&
			Pair.Value.ActionData->bAttachToParentEquipment &&
			Pair.Value.ActionData->ParentEquipmentSlot == ParentSlotTag)
		{
			ChildSlots.Add(Pair.Key);
		}
	}

	return ChildSlots;
}



void UPREquipmentManagerComponent::BindToRogueliteSubsystem()
{
	URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this);
	if (!IsValid(RogueliteSubsystem))
	{
		return;
	}

	ActionAcquiredHandle =  RogueliteSubsystem->BindActionAcquiredByTags(ActionTagsToReceive,
		FRogueliteActionFilteredSignature::FDelegate::CreateUObject(this, &ThisClass::HandleActionAcquired));
	ActionRemovedHandle = RogueliteSubsystem->BindActionRemovedByTags(ActionTagsToReceive,
		FRogueliteActionFilteredSignature::FDelegate::CreateUObject(this, &ThisClass::HandleActionRemoved));
	ActionStackChangedHandle = RogueliteSubsystem->BindStackChangedByTags(ActionTagsToReceive,
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
	
	RogueliteSubsystem->UnbindActionAcquiredByTags(ActionTagsToReceive, ActionAcquiredHandle);
	RogueliteSubsystem->UnbindActionRemovedByTags(ActionTagsToReceive, ActionRemovedHandle);
	RogueliteSubsystem->UnbindStackChangedByTags(ActionTagsToReceive, ActionStackChangedHandle);
	RogueliteSubsystem->OnRunEnded.RemoveDynamic(this, &ThisClass::HandleRunEnded);
}

void UPREquipmentManagerComponent::ProcessExistingActions()
{
	TArray<URogueliteActionData*> AllActions = URogueliteBlueprintLibrary::GetAllAcquired(this);
	for (URogueliteActionData* Action : AllActions)
	{
		if (UPREquipActionData* EquipAction = Cast<UPREquipActionData>(Action))
		{
			HandleActionAcquired(Action,0,1);
		}
	}
}

URogueliteSubsystem* UPREquipmentManagerComponent::GetRogueliteSubsystem() const
{
	return URogueliteSubsystem::Get(this);
}


void UPREquipmentManagerComponent::HandleActionAcquired(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPREquipActionData* EquipAction = Cast<UPREquipActionData>(Action);
	if (!IsValid(EquipAction))
	{
		return;
	}
	
	FGameplayTag SlotToEquip = EquipAction->EquipmentSlot;
	
	// 이미 슬롯이 사용중?
	if (HasEquipment(SlotToEquip))
	{
		// Remove Action -> HandleActionRemoved 에서 장착 해제
		UPREquipActionData* EquippedAction = GetActionData(SlotToEquip);
		URogueliteBlueprintLibrary::RemoveAction(this, EquippedAction);
	}
	
	Equip(EquipAction);
}

void UPREquipmentManagerComponent::HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPREquipActionData* EquipActionData = Cast<UPREquipActionData>(Action);
	if (!IsValid(EquipActionData))
	{
		return;
	}
	
	FGameplayTag SlotToUnequip = EquipActionData->EquipmentSlot;
	Unequip(SlotToUnequip);
}

void UPREquipmentManagerComponent::HandleActionStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPREquipActionData* EquipActionData = Cast<UPREquipActionData>(Action);
	if (!IsValid(EquipActionData))
	{
		return;
	}
	
	// TODO: 스택 지원 EquipAction?
}

void UPREquipmentManagerComponent::HandleRunEnded(bool bCompleted)
{
}
