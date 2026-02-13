// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PREquipmentPreviewActor.generated.h"

class UPREquipmentActionSet;
class UPREquipActionData;
class UEquipmentInstance;

/**
 * 장비 프리뷰를 표시하는 액터
 * - EquipmentActionSet 기반으로 장비 메시 시각화
 * - 레벨에 배치하여 장비 프리뷰 표시용으로 사용
 */
UCLASS()
class PROJECTREBOOT_API APREquipmentPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	APREquipmentPreviewActor();

protected:
	/*~ AActor Interface ~*/
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	// 장비 프리뷰 시각화 초기화
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void InitializePreviewVisuals();

	// 장비 세트 설정 및 프리뷰 갱신
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetEquipmentActionSet(UPREquipmentActionSet* InEquipmentActionSet);

public:
	// 프리뷰할 장비 세트
	UPROPERTY(EditAnywhere, Category = "Equipment")
	TObjectPtr<UPREquipmentActionSet> EquipmentActionSet;

protected:
	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	// 장비 프리뷰용 부착 대상 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> PreviewAttachComponent;

	// 장비 프리뷰 인스턴스 (시각적 표현 담당)
	UPROPERTY(Transient)
	TObjectPtr<UEquipmentInstance> PreviewInstance;
};
