// Fill out your copyright notice in the Description page of Project Settings.

#include "PREquipmentPreviewActor.h"

#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Equipment/PREquipmentActionSet.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"
#include "ProjectReboot/Equipment/EquipmentInstance.h"

APREquipmentPreviewActor::APREquipmentPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	PreviewAttachComponent = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewAttachComponent"));
	PreviewAttachComponent->SetupAttachment(RootSceneComponent);
}

void APREquipmentPreviewActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	InitializePreviewVisuals();
}

#if WITH_EDITOR
void APREquipmentPreviewActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(APREquipmentPreviewActor, EquipmentActionSet))
	{
		// 기존 인스턴스 정리 후 재생성
		if (IsValid(PreviewInstance))
		{
			PreviewInstance->Uninitialize();
			PreviewInstance = nullptr;
		}
		InitializePreviewVisuals();
	}
}
#endif

void APREquipmentPreviewActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(PreviewInstance))
	{
		PreviewInstance->Uninitialize();
		PreviewInstance = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void APREquipmentPreviewActor::InitializePreviewVisuals()
{
	if (!IsValid(EquipmentActionSet) || !IsValid(EquipmentActionSet->PrimaryAction))
	{
		return;
	}

	// 이미 초기화된 경우 스킵
	if (IsValid(PreviewInstance))
	{
		return;
	}

	UPREquipActionData* PrimaryAction = EquipmentActionSet->PrimaryAction;

	// EquipmentInstanceType이 설정되어 있으면 해당 클래스 사용, 없으면 기본 클래스
	TSubclassOf<UEquipmentInstance> InstanceClass = PrimaryAction->EquipmentInstanceType;
	if (!InstanceClass)
	{
		InstanceClass = UEquipmentInstance::StaticClass();
	}

	// PreviewInstance 생성 및 초기화
	PreviewInstance = NewObject<UEquipmentInstance>(this, InstanceClass);
	PreviewInstance->Initialize(PreviewAttachComponent, PrimaryAction);
	PreviewInstance->AddDynamicTag(TAG_Equipment_Weapon_Type_Preview); // PreviewTag 추가 (Preview용 외형 있을 시 적용)

	// 파트 부착
	for (UPREquipActionData* PartAction : EquipmentActionSet->PartActions)
	{
		if (IsValid(PartAction))
		{
			PreviewInstance->AttachPart(PartAction);
		}
	}

	// 비주얼 새로고침
	PreviewInstance->RefreshVisuals();
}

void APREquipmentPreviewActor::SetEquipmentActionSet(UPREquipmentActionSet* InEquipmentActionSet)
{
	EquipmentActionSet = InEquipmentActionSet;

	// 기존 인스턴스 정리 후 재생성
	if (IsValid(PreviewInstance))
	{
		PreviewInstance->Uninitialize();
		PreviewInstance = nullptr;
	}
	InitializePreviewVisuals();
}
