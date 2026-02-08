// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "ProjectReboot/Room/PRRoomTypes.h"
#include "GameFramework/Actor.h"
#include "PRRoomDoor.generated.h"

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
	// 상호작용 가능 여부
	virtual bool CanInteract(APawn* Interactor) const override;

	// 상호작용 실행
	virtual void Interact(APawn* Interactor) override;

	// UI 힌트 텍스트 반환
	virtual FText GetInteractionText() const override;

	// UI 표시용 정보 반환
	virtual void GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const override;

	// 상호작용 가능 상태 설정
	virtual void SetInteractable(bool bEnabled) override;

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

	// 상호작용 텍스트
	UPROPERTY(EditAnywhere, Category = "Room|Door")
	FText InteractionText;

	// 상호작용 가능 여부
	UPROPERTY(EditAnywhere, Category = "Room|Door")
	bool bIsInteractable = false;

	// 대상 방 인덱스
	UPROPERTY(VisibleAnywhere, Category = "Room|Door")
	int32 TargetRoomIndex = -1;

	// 표시되는 방 타입
	UPROPERTY(VisibleAnywhere, Category = "Room|Door")
	EPRRoomType DisplayRoomType = EPRRoomType::None;

	// 표시되는 보상 카테고리
	UPROPERTY(VisibleAnywhere, Category = "Room|Door")
	FGameplayTag DisplayRewardCategory;
};
