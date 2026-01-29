// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PRInteractionComponent.generated.h"

class AActor;
class APRPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, AActor*, NewInteractable);

/**
 * 상호작용 가능한 대상을 탐지하고 상호작용을 요청하는 컴포넌트
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 기본 생성자
	UPRInteractionComponent();

	// 상호작용 설정 초기화
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void InitializeSettings(float InDistance, float InRadius);

public:
	/*~ Interaction ~*/

	// 현재 대상과 상호작용
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool TryInteract();

	// 현재 상호작용 가능 대상 반환
	UFUNCTION(BlueprintPure, Category = "Interaction")
	AActor* GetCurrentInteractable() const;

	// 상호작용 대상 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Interaction")
	FOnInteractableChanged OnInteractableChanged;

protected:
	/*~ UActorComponent Interface ~*/
	// 매 프레임 상호작용 대상 갱신
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// 전방 Sphere Trace로 대상 탐지
	void UpdateInteractable();

protected:
	// 상호작용 거리
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionDistance = 300.0f;

	// 상호작용 탐지 반경
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionRadius = 50.0f;

private:
	// 현재 상호작용 대상
	TWeakObjectPtr<AActor> CurrentInteractable;
};
