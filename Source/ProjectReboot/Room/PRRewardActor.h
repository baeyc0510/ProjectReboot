// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectReboot/Interaction/PRInteractableInterface.h"
#include "PRRewardActor.generated.h"

class URoguelitePoolPreset;
class UPRActionDecisionPanel;
class URogueliteActionData;

/**
 * 보상 액터 - 상호작용하여 보상 선택
 */
UCLASS()
class PROJECTREBOOT_API APRRewardActor : public AActor, public IPRInteractableInterface
{
	GENERATED_BODY()

public:
	APRRewardActor();

	// 보상 풀 프리셋 설정
	UFUNCTION(BlueprintCallable, Category = "Reward")
	void SetRewardPoolPreset(URoguelitePoolPreset* InPoolPreset);

	// 보상 풀 프리셋 조회
	UFUNCTION(BlueprintPure, Category = "Reward")
	URoguelitePoolPreset* GetRewardPoolPreset() const { return RewardPoolPreset; }

protected:
	/*~ AActor Interface ~*/
	virtual void BeginPlay() override;

	/*~ IPRInteractableInterface ~*/
	virtual bool CanInteract(APawn* Interactor) const override;
	virtual void Interact(APawn* Interactor) override;
	virtual FText GetInteractionText() const override;
	virtual USoundBase* GetInteractionSound() override {return InteractionSound;}
	
	// 보상 선택 패널 표시
	void ShowRewardPanel(APawn* Interactor);

	// 보상 선택 확정 처리
	UFUNCTION()
	void HandleRewardConfirmed(URogueliteActionData* SelectedAction);

protected:
	// 보상 풀 프리셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	TObjectPtr<URoguelitePoolPreset> RewardPoolPreset;

	// 보상 선택 패널 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPRActionDecisionPanel> DecisionPanelClass;

	// 상호작용 텍스트
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	FText InteractionText = FText::FromString(TEXT("보상 선택"));

	// 보상 선택 개수
	UPROPERTY(EditDefaultsOnly, Category = "Reward")
	int32 RewardCount = 3;

	// 사운드 세팅
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	TObjectPtr<USoundBase> InteractionSound;
	
	// 상호작용 가능 여부
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	bool bIsInteractable = true;
	
private:
	// 현재 표시 중인 패널
	UPROPERTY()
	TObjectPtr<UPRActionDecisionPanel> CurrentPanel;
};
