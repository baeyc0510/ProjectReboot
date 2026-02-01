// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentInstance.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"

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

    if (FSpawnedVisualEntry* Entry = SpawnedVisuals.Find(InActionData))
    {
        // 컴포넌트 제거
        if (Entry->SpawnedComponent)
        {
            Entry->SpawnedComponent->DestroyComponent();
        }

        SpawnedVisuals.Remove(InActionData);
    }

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
            // 기존 컴포넌트 제거
            if (Entry.SpawnedComponent)
            {
                Entry.SpawnedComponent->DestroyComponent();
                Entry.SpawnedComponent = nullptr;
            }

            // 새 컴포넌트 생성
            if (NewSpawnInfo.IsValid())
            {
                bool bIsPrimaryMesh = Data == PrimaryActionData;
                Entry.SpawnedComponent = CreateMeshComponent(NewSpawnInfo,bIsPrimaryMesh);
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

void UEquipmentInstance::OnEquipmentTagsChanged()
{
    // 하위 클래스에서 override하여 태그 변경에 따른 처리 수행
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