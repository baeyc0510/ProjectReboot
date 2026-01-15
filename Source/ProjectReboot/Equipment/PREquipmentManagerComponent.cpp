// Fill out your copyright notice in the Description page of Project Settings.


#include "PREquipmentManagerComponent.h"

#include "EquipmentInstance.h"
#include "RogueliteBlueprintLibrary.h"
#include "RogueliteSubsystem.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Roguelite/PREquipActionData.h"


// Sets default values for this component's properties
UPREquipmentManagerComponent::UPREquipmentManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	ActionTagsToManage.AddTag(TAG_Equipment);
}

// Called when the game starts
void UPREquipmentManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	BindToRogueliteSubsystem();
	ProcessExistingActions();
}

void UPREquipmentManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnequipAll();
	UnbindFromRogueliteSubsystem();
	
	Super::EndPlay(EndPlayReason);
}


void UPREquipmentManagerComponent::Equip(UPREquipActionData* ActionData, bool bRefreshVisuals)
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
	
	// 애니메이션 링크
	if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(GetAttachTarget()))
	{
		TArray<TSubclassOf<UAnimInstance>>& AnimLayersToLink = ActionData->EquipmentVisualSettings.AnimLayersToLink;
		for (auto& Link : AnimLayersToLink)
		{
			SkeletalMeshComp->LinkAnimClassLayers(Link);
		}
	}

	// 자식 Equipment
	if (ActionData->bAttachToParentEquipment)
	{
		UEquipmentInstance* ParentInstance = GetEquipmentInstance(ActionData->ParentEquipmentSlot);
		if (!ParentInstance)
		{
			UE_LOG(LogTemp,Warning,TEXT("ActionData %s 가 부모 Equipment를 필요로 하지만 아직 부모 EquipmentInstance가 없음."), *ActionData->GetName());
			return;
		}

		ParentInstance->AttachPart(ActionData);
		if (bRefreshVisuals)
		{
			ParentInstance->RefreshVisuals();	
		}
		
		FEquipmentSlotEntry Entry;
		Entry.Instance = ParentInstance;
		Entry.ActionData = ActionData;
		
		Slots.Add(SlotTag, Entry);
		OnEquipped.Broadcast(SlotTag, ParentInstance, ActionData);
	}
	// 부모 Equipment
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

void UPREquipmentManagerComponent::UnequipByAction(UPREquipActionData* ActionData, bool bRefreshVisuals)
{
	if (!IsValid(ActionData))
	{
		return;
	}
	
	if (ActionData != GetActionData(ActionData->EquipmentSlot))
	{
		return;
	}
	
	Unequip(ActionData->EquipmentSlot);
}


void UPREquipmentManagerComponent::Unequip(FGameplayTag SlotTag, bool bRefreshVisuals)
{
	FEquipmentSlotEntry* Entry = Slots.Find(SlotTag);
	if (!Entry)
	{
		return;
	}

	UnequipChildren(SlotTag);

	UEquipmentInstance* Instance = Entry->Instance;
	UPREquipActionData* ActionData = Entry->ActionData;

	// 애니메이션 언링크
	if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(GetAttachTarget()))
	{
		TArray<TSubclassOf<UAnimInstance>>& AnimLayersToLink = ActionData->EquipmentVisualSettings.AnimLayersToLink;
		for (auto& Link : AnimLayersToLink)
		{
			SkeletalMeshComp->UnlinkAnimClassLayers(Link);
		}
	}
	
	if (IsParentEquipmentSlot(SlotTag))
	{
		Instance->Uninitialize();
	}
	else
	{
		Instance->DetachPart(ActionData);
		if (bRefreshVisuals)
		{
			Instance->RefreshVisuals();	
		}
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

UEquipmentInstance* UPREquipmentManagerComponent::GetEquipmentInstance(FGameplayTag SlotTag) const
{
	const FEquipmentSlotEntry* Entry = Slots.Find(SlotTag);
	return Entry ? Entry->Instance : nullptr;
}

TArray<UEquipmentInstance*> UPREquipmentManagerComponent::GetAllEquipmentInstances() const
{
	TArray<UEquipmentInstance*> Instances;

	for (const auto& Pair : Slots)
	{
		if (Pair.Value.Instance)
		{
			Instances.AddUnique(Pair.Value.Instance);
		}
	}
	
	return Instances;
}

UPREquipActionData* UPREquipmentManagerComponent::GetActionData(FGameplayTag SlotTag) const
{
	const FEquipmentSlotEntry* Entry = Slots.Find(SlotTag);
	return Entry ? Entry->ActionData : nullptr;
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

void UPREquipmentManagerComponent::RefreshAllVisuals()
{
	TArray<UEquipmentInstance*> AllInstances = GetAllEquipmentInstances();
	for (UEquipmentInstance* Instance : AllInstances)
	{
		if (IsValid(Instance))
		{
			Instance->RefreshVisuals();
		}
	}
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
	// Owner 액터가 캐릭터인 경우
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			return Mesh;
		}
	}
	
	// Fallback: SkeletalMeshComponent를 직접 찾음
	if (GetOwner())
	{
		if (USkeletalMeshComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			return Mesh;
		}
	}
	
	return nullptr;
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

	ActionAcquiredHandle =  RogueliteSubsystem->BindActionAcquiredByTags(ActionTagsToManage,
		FRogueliteActionFilteredSignature::FDelegate::CreateUObject(this, &ThisClass::HandleActionAcquired));
	ActionRemovedHandle = RogueliteSubsystem->BindActionRemovedByTags(ActionTagsToManage,
		FRogueliteActionFilteredSignature::FDelegate::CreateUObject(this, &ThisClass::HandleActionRemoved));
	ActionStackChangedHandle = RogueliteSubsystem->BindStackChangedByTags(ActionTagsToManage,
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
	
	RogueliteSubsystem->UnbindActionAcquiredByTags(ActionTagsToManage, ActionAcquiredHandle);
	RogueliteSubsystem->UnbindActionRemovedByTags(ActionTagsToManage, ActionRemovedHandle);
	RogueliteSubsystem->UnbindStackChangedByTags(ActionTagsToManage, ActionStackChangedHandle);
	RogueliteSubsystem->OnRunEnded.RemoveDynamic(this, &ThisClass::HandleRunEnded);
}

void UPREquipmentManagerComponent::ProcessExistingActions()
{
	TArray<URogueliteActionData*> AllActions = URogueliteBlueprintLibrary::GetAllAcquired(this);
	TArray<UPREquipActionData*> SortedActions = GetSortedActionsByDependency(AllActions);
	
	for (UPREquipActionData* EquipAction : SortedActions)
	{
		// RefreshVisuals는 한번에 처리
		HandleActionAcquired_Internal(EquipAction, false);
	}
	
	RefreshAllVisuals();
}

TArray<UPREquipActionData*> UPREquipmentManagerComponent::GetSortedActionsByDependency(TArray<URogueliteActionData*>& InActions) const
{
	TArray<UPREquipActionData*> Result;
	
	for (URogueliteActionData* RawAction : InActions)
	{
		UPREquipActionData* EquipAction = Cast<UPREquipActionData>(RawAction);
		if (!IsValid(EquipAction))
		{
			continue;
		}
		
		if (ActionTagsToManage.IsEmpty() || EquipAction->HasAnyTags(ActionTagsToManage))
		{
			Result.Add(EquipAction);
		}
	}
	
	// 부모 Equipment가 앞으로 오도록 정렬
	Result.Sort([](const UPREquipActionData& A, const UPREquipActionData& B)
	{
		if (!A.bAttachToParentEquipment && B.bAttachToParentEquipment)
		{
			return true;
		}
		if (A.bAttachToParentEquipment && !B.bAttachToParentEquipment)
		{
			return false;
		}
		return false;
	});
	
	return Result;
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
	HandleActionRemoved_Internal(EquipAction,true);
}

void UPREquipmentManagerComponent::HandleActionRemoved(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPREquipActionData* EquipAction = Cast<UPREquipActionData>(Action);
	if (!IsValid(EquipAction))
	{
		return;
	}
	HandleActionRemoved_Internal(EquipAction,true);
}

void UPREquipmentManagerComponent::HandleActionStackChanged(URogueliteActionData* Action, int32 OldStacks, int32 NewStacks)
{
	UPREquipActionData* EquipAction = Cast<UPREquipActionData>(Action);
	if (!IsValid(EquipAction))
	{
		return;
	}
	
	// TODO: 스택 지원 EquipAction?
}

void UPREquipmentManagerComponent::HandleRunEnded(bool bCompleted)
{
}

void UPREquipmentManagerComponent::HandleActionAcquired_Internal(UPREquipActionData* EquipAction, bool bRefreshVisuals)
{
	FGameplayTag SlotToEquip = EquipAction->EquipmentSlot;
	
	// 이미 슬롯이 사용중?
	if (HasEquipment(SlotToEquip))
	{
		// Remove Action -> HandleActionRemoved 에서 장착 해제
		UPREquipActionData* EquippedAction = GetActionData(SlotToEquip);
		URogueliteBlueprintLibrary::RemoveAction(this, EquippedAction);
	}
	
	Equip(EquipAction, bRefreshVisuals);
}

void UPREquipmentManagerComponent::HandleActionRemoved_Internal(UPREquipActionData* EquipAction, bool bRefreshVisuals)
{
	FGameplayTag SlotToUnequip = EquipAction->EquipmentSlot;
	Unequip(SlotToUnequip, bRefreshVisuals);
}
