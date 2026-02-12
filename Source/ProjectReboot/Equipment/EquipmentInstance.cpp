// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentInstance.h"
#include "Components/MeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"
#include "ProjectReboot/Utils/PRBlueprintFunctionLibrary.h"

void UEquipmentInstance::Initialize(USceneComponent* InAttachTarget, UPREquipActionData* InPrimaryActionData)
{
    AttachTarget = InAttachTarget;
    PrimaryActionData = InPrimaryActionData;

    if (PrimaryActionData)
    {
        AttachPart(PrimaryActionData);
    }

    // 초기 가시성 적용
    SetVisualsVisible(bVisualsVisible);
}

void UEquipmentInstance::Uninitialize()
{
    DestroyAllVisuals();
    AttachTarget.Reset();
    PrimaryActionData = nullptr;
    AttachedActions.Reset();
}

void UEquipmentInstance::AttachPart(UPREquipActionData* InActionData)
{
    if (!InActionData || !AttachTarget.IsValid())
    {
        return;
    }

    if (!AttachedActions.Contains(InActionData))
    {
        AttachedActions.Add(InActionData);
    }

    // 태그 추가
    EquipmentTags.AppendTags(InActionData->EquipmentTags);
    OnEquipmentTagsChanged();

    // 스폰 정보 선택
    FEquipmentMeshSpawnInfo SpawnInfo = SelectSpawnInfo(InActionData->EquipmentVisualSettings);

    if (!SpawnInfo.IsValid())
    {
        // 메시 없어도 태그는 등록됨
        SpawnedVisuals.Add(InActionData, FSpawnedVisualEntry());
        return;
    }

    // 컴포넌트 생성
    bool bIsPrimaryMesh = InActionData == PrimaryActionData;
    USceneComponent* NewComponent = CreateMeshComponent(SpawnInfo,bIsPrimaryMesh);
    ApplyCollisionSettings(NewComponent, InActionData->EquipmentVisualSettings);

    FSpawnedVisualEntry Entry;
    Entry.SpawnedComponent = NewComponent;
    Entry.UsedSpawnInfo = SpawnInfo;

    SpawnedVisuals.Add(InActionData, Entry);

    // 현재 가시성 상태 적용
    if (Entry.SpawnedComponent)
    {
        Entry.SpawnedComponent->SetVisibility(bVisualsVisible, true);
    }
}

void UEquipmentInstance::DetachPart(UPREquipActionData* InActionData)
{
    if (!InActionData)
    {
        return;
    }

    AttachedActions.Remove(InActionData);

    DestroyVisualComponent(InActionData);
    SpawnedVisuals.Remove(InActionData);

    // 태그 제거
    EquipmentTags.RemoveTags(InActionData->EquipmentTags);
    OnEquipmentTagsChanged();
}

TArray<UPREquipActionData*> UEquipmentInstance::GetAllAttachedActions() const
{
    TArray<UPREquipActionData*> Result;
    Result.Reserve(AttachedActions.Num());
    for (const TObjectPtr<UPREquipActionData>& ActionData : AttachedActions)
    {
        if (ActionData)
        {
            Result.Add(ActionData);
        }
    }
    return Result;
}

TArray<UPREquipActionData*> UEquipmentInstance::GetChildPartActions() const
{
    TArray<UPREquipActionData*> Result = GetAllAttachedActions();
    Result.Remove(PrimaryActionData);
    return Result;
}

void UEquipmentInstance::RefreshVisuals()
{
    if (!AttachTarget.IsValid())
    {
        return;
    }

    for (auto& Pair : SpawnedVisuals)
    {
        UPREquipActionData* Data = Pair.Key;
        FSpawnedVisualEntry& Entry = Pair.Value;

        FEquipmentMeshSpawnInfo NewSpawnInfo = SelectSpawnInfo(Data->EquipmentVisualSettings);

        // 변경 여부 체크
        bool bNeedsRefresh = false;

        if (NewSpawnInfo.MeshType != Entry.UsedSpawnInfo.MeshType)
        {
            bNeedsRefresh = true;
        }
        else if (NewSpawnInfo.MeshType == EEquipmentMeshType::StaticMesh &&
                 NewSpawnInfo.StaticMesh != Entry.UsedSpawnInfo.StaticMesh)
        {
            bNeedsRefresh = true;
        }
        else if (NewSpawnInfo.MeshType == EEquipmentMeshType::SkeletalMesh &&
                 NewSpawnInfo.SkeletalMesh != Entry.UsedSpawnInfo.SkeletalMesh)
        {
            bNeedsRefresh = true;
        }
        else if (NewSpawnInfo.MaterialOverrides.OrderIndependentCompareEqual(Entry.UsedSpawnInfo.MaterialOverrides) == false)
        {
            bNeedsRefresh = true;
        }

        if (bNeedsRefresh)
        {
            // 기존 비주얼 정리 (Dissolve 포함)
            DestroyVisualComponent(Data);

            // 새 컴포넌트 생성
            if (NewSpawnInfo.IsValid())
            {
                bool bIsPrimaryMesh = Data == PrimaryActionData;
                Entry.SpawnedComponent = CreateMeshComponent(NewSpawnInfo,bIsPrimaryMesh);
                ApplyCollisionSettings(Entry.SpawnedComponent, Data->EquipmentVisualSettings);
            }

            Entry.UsedSpawnInfo = NewSpawnInfo;
        }
    }
}

void UEquipmentInstance::RespawnVisuals()
{
    if (!AttachTarget.IsValid())
    {
        return;
    }

    TArray<TObjectPtr<UPREquipActionData>> ActionsToRespawn = AttachedActions;

    DestroyAllVisuals();

    for (const TObjectPtr<UPREquipActionData>& ActionData : ActionsToRespawn)
    {
        if (ActionData)
        {
            AttachPart(ActionData);
        }
    }
}

void UEquipmentInstance::DestroyAllVisuals()
{
    // Dissolve 전체 정리
    ActiveDissolves.Empty();
    ClearDissolveTimer();

    for (auto& Pair : SpawnedVisuals)
    {
        if (Pair.Value.SpawnedComponent)
        {
            Pair.Value.SpawnedComponent->DestroyComponent();
        }
    }

    SpawnedVisuals.Empty();
    EquipmentTags.Reset();
}

void UEquipmentInstance::GetSpawnedVisualInfo(TMap<UPREquipActionData*, FSpawnedVisualEntry>& OutVisualInfo) const
{
	OutVisualInfo = SpawnedVisuals;
}

void UEquipmentInstance::AddDynamicTag(FGameplayTag TagToAdd)
{
    EquipmentTags.AddTag(TagToAdd);
    OnEquipmentTagsChanged();
}

FEquipmentMeshSpawnInfo UEquipmentInstance::SelectSpawnInfo(const FEquipmentVisualSettings& VisualSettings) const
{
    if (!VisualSettings.bUseRuleBasedMeshSpawnInfo)
    {
        return VisualSettings.DefaultMeshSpawnInfo;
    }

    // Priority 내림차순 정렬을 위한 복사
    TArray<FRuleBasedMeshVisualSetting> SortedRules = VisualSettings.RuleBasedMeshSpawnSettings;
    SortedRules.Sort([](const FRuleBasedMeshVisualSetting& A, const FRuleBasedMeshVisualSetting& B)
    {
        return A.Priority > B.Priority;
    });

    // 조건 충족하는 첫 번째 룰 반환
    for (const FRuleBasedMeshVisualSetting& Rule : SortedRules)
    {
        if (EquipmentTags.HasAll(Rule.RequiredTags))
        {
            return Rule.MeshSpawnInfo;
        }
    }

    return VisualSettings.DefaultMeshSpawnInfo;
}

USceneComponent* UEquipmentInstance::CreateMeshComponent(const FEquipmentMeshSpawnInfo& SpawnInfo, bool bIsPrimaryMesh)
{
    AActor* Owner = AttachTarget.IsValid() ? AttachTarget->GetOwner() : nullptr;
    if (!Owner)
    {
        return nullptr;
    }

    USceneComponent* NewComponent = nullptr;

    switch (SpawnInfo.MeshType)
    {
    case EEquipmentMeshType::StaticMesh:
        if (SpawnInfo.StaticMesh)
        {
            UStaticMeshComponent* StaticMeshComp = NewObject<UStaticMeshComponent>(Owner);
            StaticMeshComp->SetStaticMesh(SpawnInfo.StaticMesh);
            ApplyMaterialOverrides(StaticMeshComp, SpawnInfo.MaterialOverrides);
            NewComponent = StaticMeshComp;
        }
        break;

    case EEquipmentMeshType::SkeletalMesh:
        if (SpawnInfo.SkeletalMesh)
        {
            USkeletalMeshComponent* SkelMeshComp = NewObject<USkeletalMeshComponent>(Owner);
            SkelMeshComp->SetSkeletalMesh(SpawnInfo.SkeletalMesh);
            ApplyMaterialOverrides(SkelMeshComp, SpawnInfo.MaterialOverrides);
            NewComponent = SkelMeshComp;
        }
        break;

    default:
        break;
    }

    if (NewComponent)
    {
        ApplyAttachment(NewComponent, SpawnInfo.AttachmentInfo, !bIsPrimaryMesh);
        NewComponent->RegisterComponent();
    }

    return NewComponent;
}

void UEquipmentInstance::ApplyAttachment(USceneComponent* Component, const FEquipmentAttachmentInfo& AttachInfo, bool bIsChild)
{
    if (!Component || !AttachTarget.IsValid())
    {
        return;
    }
    
    USceneComponent* Parent = nullptr;
    if (!bIsChild)
    {
        Parent = AttachTarget.Get();
    }
    else if (IsValid(PrimaryActionData) && SpawnedVisuals.Find(PrimaryActionData))
    {
        Parent = SpawnedVisuals[PrimaryActionData].SpawnedComponent;
    }
    else
    {
        UE_LOG(LogTemp,Warning,TEXT("Try to attach child but there is no primary visual"));
    }
    
    Component->AttachToComponent(
               Parent,
               FAttachmentTransformRules::SnapToTargetIncludingScale,
               AttachInfo.SocketName
           );    
    Component->SetRelativeLocation(AttachInfo.LocationOffset);
    Component->SetRelativeRotation(AttachInfo.RotationOffset);
    Component->SetRelativeScale3D(AttachInfo.Scale);
}

FGameplayTag UEquipmentInstance::GetSlotTag() const
{
    return PrimaryActionData ? PrimaryActionData->EquipmentSlot : FGameplayTag();
}

USceneComponent* UEquipmentInstance::GetPrimaryComponent() const
{
    if (PrimaryActionData)
    {
        const FSpawnedVisualEntry* Entry = SpawnedVisuals.Find(PrimaryActionData);
        if (Entry)
        {
            return Entry->SpawnedComponent;
        }
    }

    return nullptr;
}

bool UEquipmentInstance::HasVisual(UPREquipActionData* InActionData) const
{
    return SpawnedVisuals.Contains(InActionData);
}

void UEquipmentInstance::SetVisualsVisible(bool bVisible)
{
    bVisualsVisible = bVisible;

    for (auto& Pair : SpawnedVisuals)
    {
        if (Pair.Value.SpawnedComponent)
        {
            Pair.Value.SpawnedComponent->SetVisibility(bVisualsVisible, true);
        }
    }
}

void UEquipmentInstance::OnEquipped()
{
    // 하위 클래스에서 override하여 장착 완료 시 처리 수행
}

void UEquipmentInstance::OnEquipmentTagsChanged()
{
    // 하위 클래스에서 override하여 태그 변경에 따른 처리 수행
}

void UEquipmentInstance::DestroyVisualComponent(UPREquipActionData* ActionData)
{
	// Dissolve 상태 정리
	ActiveDissolves.Remove(ActionData);
	if (ActiveDissolves.IsEmpty())
	{
		ClearDissolveTimer();
	}

	// 컴포넌트 파괴
	if (FSpawnedVisualEntry* Entry = SpawnedVisuals.Find(ActionData))
	{
		if (Entry->SpawnedComponent)
		{
			Entry->SpawnedComponent->DestroyComponent();
			Entry->SpawnedComponent = nullptr;
		}
	}
}

void UEquipmentInstance::ClearDissolveTimer()
{
	if (!DissolveTimerHandle.IsValid())
	{
		return;
	}

	UWorld* World = AttachTarget.IsValid() ? AttachTarget->GetWorld() : nullptr;
	if (IsValid(World))
	{
		World->GetTimerManager().ClearTimer(DissolveTimerHandle);
	}
	DissolveTimerHandle.Invalidate();
}

void UEquipmentInstance::StartDissolve(float Time, bool bReverse)
{
	for (auto& [ActionData, Entry] : SpawnedVisuals)
	{
		if (IsValid(ActionData))
		{
			StartPartDissolve(ActionData, Time, bReverse);
		}
	}
}

void UEquipmentInstance::StartPartDissolve(UPREquipActionData* ActionData, float Time, bool bReverse)
{
	if (!IsValid(ActionData))
	{
		return;
	}

	FSpawnedVisualEntry* VisualEntry = SpawnedVisuals.Find(ActionData);
	if (!VisualEntry)
	{
		return;
	}

	UMeshComponent* MeshComp = Cast<UMeshComponent>(VisualEntry->SpawnedComponent);
	if (!IsValid(MeshComp))
	{
		return;
	}

	UWorld* World = AttachTarget.IsValid() ? AttachTarget->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		return;
	}

	TArray<UMaterialInstanceDynamic*> Mids = UPRBlueprintFunctionLibrary::GetAllDynamicMaterials(MeshComp);
	if (Mids.IsEmpty())
	{
		return;
	}

	FDissolveEntry Entry;
	Entry.Materials = Mids;
	Entry.StartTime = World->GetTimeSeconds();
	Entry.Duration = FMath::Max(Time, SMALL_NUMBER);
	Entry.bReverse = bReverse;

	// 초기값 설정
	float InitialValue = bReverse ? 1.0f : 0.0f;
	if (bUseDissolvePercentage)
	{
		InitialValue *= 100.0f;
	}
	for (UMaterialInstanceDynamic* Mid : Entry.Materials)
	{
		if (IsValid(Mid))
		{
			Mid->SetScalarParameterValue(DissolveParameterName, InitialValue);
		}
	}

	ActiveDissolves.Add(ActionData, MoveTemp(Entry));

	// 타이머 시작 (이미 돌고 있지 않다면)
	if (!DissolveTimerHandle.IsValid())
	{
		World->GetTimerManager().SetTimer(
			DissolveTimerHandle,
			FTimerDelegate::CreateUObject(this, &ThisClass::UpdateDissolve),
			1.0f / 60.0f,
			true
		);
	}
}

void UEquipmentInstance::UpdateDissolve()
{
	UWorld* World = AttachTarget.IsValid() ? AttachTarget->GetWorld() : nullptr;
	if (!IsValid(World))
	{
		ActiveDissolves.Empty();
		return;
	}

	float CurrentTime = World->GetTimeSeconds();
	TArray<UPREquipActionData*> CompletedEntries;

	for (auto& [ActionData, Entry] : ActiveDissolves)
	{
		float Elapsed = CurrentTime - Entry.StartTime;
		float Alpha = FMath::Clamp(Elapsed / Entry.Duration, 0.0f, 1.0f);

		if (Entry.bReverse)
		{
			Alpha = 1.0f - Alpha;
		}

		float Value = bUseDissolvePercentage ? Alpha * 100.0f : Alpha;
		for (UMaterialInstanceDynamic* Mid : Entry.Materials)
		{
			if (IsValid(Mid))
			{
				Mid->SetScalarParameterValue(DissolveParameterName, Value);
			}
		}

		if (Elapsed >= Entry.Duration)
		{
			CompletedEntries.Add(ActionData);
		}
	}

	for (UPREquipActionData* Completed : CompletedEntries)
	{
		ActiveDissolves.Remove(Completed);
	}

	if (ActiveDissolves.IsEmpty())
	{
		ClearDissolveTimer();
	}
}

void UEquipmentInstance::ApplyMaterialOverrides(UMeshComponent* MeshComponent, const TMap<int32, UMaterialInterface*>& MaterialOverrides)
{
    if (!IsValid(MeshComponent))
    {
        return;
    }

    for (const auto& Pair : MaterialOverrides)
    {
        if (IsValid(Pair.Value))
        {
            MeshComponent->SetMaterial(Pair.Key, Pair.Value);
        }
    }
}

void UEquipmentInstance::ApplyCollisionSettings(USceneComponent* Component, const FEquipmentVisualSettings& VisualSettings)
{
    if (VisualSettings.CollisionProfileName == UCollisionProfile::NoCollision_ProfileName)
    {
        return;
    }

    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component);
    if (!IsValid(PrimComp))
    {
        return;
    }

    PrimComp->SetCollisionProfileName(VisualSettings.CollisionProfileName);
    PrimComp->SetCollisionEnabled(VisualSettings.CollisionEnabled);   
}