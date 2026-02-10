// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PREquipmentTypes.h"
#include "RogueliteTypes.h"
#include "UObject/Object.h"
#include "EquipmentInstance.generated.h"

class UPREquipActionData;
class UMeshComponent;

USTRUCT()
struct FSpawnedVisualEntry
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<USceneComponent> SpawnedComponent = nullptr;

	UPROPERTY()
	FEquipmentMeshSpawnInfo UsedSpawnInfo;
};

UCLASS(BlueprintType, Blueprintable)
class PROJECTREBOOT_API UEquipmentInstance : public UObject
{
	GENERATED_BODY()

public:
	// 초기화
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void Initialize(USceneComponent* InAttachTarget, UPREquipActionData* InPrimaryActionData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void Uninitialize();

	// 비주얼 관리
	// 외형 추가
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void AttachPart(UPREquipActionData* ActionData);
	
	// 외형 제거
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void DetachPart(UPREquipActionData* ActionData);
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	TArray<UPREquipActionData*> GetAllAttachedActions() const;
	
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	TArray<UPREquipActionData*> GetChildPartActions() const;
	
	// 외형 새로고침
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RefreshVisuals();

	// 외형 재생성
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void RespawnVisuals();
	
	// 모든 외형 파괴
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void DestroyAllVisuals();

	// 태그 추가
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void AddDynamicTag(FGameplayTag TagToAdd);
	
	// 태그 조회
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	const FGameplayTagContainer& GetGrantedTags() const { return EquipmentTags.GetTags(); }

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool HasTag(FGameplayTag Tag) const { return EquipmentTags.HasTag(Tag); }

	// Getters
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	UPREquipActionData* GetPrimaryActionData() const { return PrimaryActionData; }

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FGameplayTag GetSlotTag() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	USceneComponent* GetPrimaryComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool HasVisual(UPREquipActionData* ActionData) const;

	// 장비 외형 가시성 설정
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetVisualsVisible(bool bVisible);

	// 장비 외형 가시성 반환
	UFUNCTION(BlueprintPure, Category = "Equipment")
	bool IsVisualsVisible() const { return bVisualsVisible; }

	// 장비 장착 완료 시 호출
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void OnEquipped();

protected:
	// 장비 태그 변경 시 호출되는 가상 함수
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	virtual void OnEquipmentTagsChanged();

	FEquipmentMeshSpawnInfo SelectSpawnInfo(const FEquipmentVisualSettings& VisualSettings) const;
	USceneComponent* CreateMeshComponent(const FEquipmentMeshSpawnInfo& SpawnInfo, bool bIsPrimaryMesh);
	void ApplyAttachment(USceneComponent* Component, const FEquipmentAttachmentInfo& AttachInfo, bool bIsChild);
	void ApplyMaterialOverrides(UMeshComponent* MeshComponent, const TMap<int32, UMaterialInterface*>& MaterialOverrides);

protected:
	UPROPERTY()
	TWeakObjectPtr<USceneComponent> AttachTarget;

	UPROPERTY()
	TObjectPtr<UPREquipActionData> PrimaryActionData;

	UPROPERTY()
	FRogueliteTagCountContainer EquipmentTags;

	// 부착된 액션 목록
	UPROPERTY()
	TArray<TObjectPtr<UPREquipActionData>> AttachedActions;

	UPROPERTY()
	TMap<UPREquipActionData*, FSpawnedVisualEntry> SpawnedVisuals;

	// 외형 가시성 상태
	bool bVisualsVisible = true;
};