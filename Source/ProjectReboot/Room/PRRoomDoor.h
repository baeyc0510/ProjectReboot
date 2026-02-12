// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "GameFramework/Actor.h"
#include "PRRoomDoor.generated.h"

class UPRBillboardWidgetComponent;

/**
 * 보상 카테고리별 표시 정보
 */
USTRUCT(BlueprintType)
struct FPRRewardCategoryDisplayInfo
{
	GENERATED_BODY()

	// 표시 텍스트
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayText;

	// 표시 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon = nullptr;
};

/**
 * 방 전환용 문 액터
 */
UCLASS(Blueprintable)
class PROJECTREBOOT_API APRRoomDoor : public AActor, public IPRInteractableInterface
{
	GENERATED_BODY()

public:
	APRRoomDoor();

	/*~ IPRInteractableInterface ~*/
	virtual bool CanInteract(APawn* Interactor) const override;
	virtual void Interact(APawn* Interactor) override;
	virtual FText GetInteractionText() const override;
	virtual void GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const override;
	virtual void SetInteractable(bool bEnabled) override;
	virtual USoundBase* GetInteractionSound() override {return InteractionSound;}

public:
	// 대상 방 인덱스 설정
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void SetTargetRoomIndex(int32 InTargetRoomIndex);

	// 표시 정보 설정 (방 타입, 보상 카테고리)
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void SetDisplayInfo(EPRRoomType InRoomType, FGameplayTag InRewardCategory);

	// 표시 정보 설정 시 호출 (UI 갱신용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Door")
	void OnDisplayInfoSet(EPRRoomType RoomType, FGameplayTag RewardCategory);

	// 할당 해제
	UFUNCTION(BlueprintCallable, Category = "Room|Door")
	void ClearAssignment();

	// 할당 해제 시 호출 (UI 갱신용)
	UFUNCTION(BlueprintImplementableEvent, Category = "Room|Door")
	void OnAssignmentCleared();

	// 대상 방 인덱스 반환
	UFUNCTION(BlueprintPure, Category = "Room|Door")
	int32 GetTargetRoomIndex() const { return TargetRoomIndex; }

	// 표시되는 방 타입 반환
	UFUNCTION(BlueprintPure, Category = "Room|Door")
	EPRRoomType GetDisplayRoomType() const { return DisplayRoomType; }

	// 표시되는 보상 카테고리 반환
	UFUNCTION(BlueprintPure, Category = "Room|Door")
	FGameplayTag GetDisplayRewardCategory() const { return DisplayRewardCategory; }

protected:
	/*~ AActor Interface ~*/
	virtual void BeginPlay() override;

	// 빌보드 위젯에 보상 카테고리 정보 반영
	void UpdateBillboardFromCategory(FGameplayTag InRewardCategory);

	// 빌보드 위젯 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UPRBillboardWidgetComponent> BillboardWidget;

	// 보상 카테고리별 표시 정보 매핑
	UPROPERTY(EditDefaultsOnly, Category = "Room|Door")
	TMap<FGameplayTag, FPRRewardCategoryDisplayInfo> RewardCategoryDisplayMap;

	// 상호작용 텍스트
	UPROPERTY(EditAnywhere, Category = "Interaction")
	FText InteractionText;
	
	// 사운드 세팅
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TObjectPtr<USoundBase> InteractionSound;
	
	// 상호작용 가능 여부
	UPROPERTY(EditAnywhere, Category = "Interaction")
	bool bIsInteractable = false;

	// 대상 방 인덱스
	UPROPERTY(VisibleAnywhere, Category = "Room")
	int32 TargetRoomIndex = -1;

	// 표시되는 방 타입
	UPROPERTY(VisibleAnywhere, Category = "Room")
	EPRRoomType DisplayRoomType = EPRRoomType::Default;

	// 표시되는 보상 카테고리
	UPROPERTY(VisibleAnywhere, Category = "Room")
	FGameplayTag DisplayRewardCategory;
};
