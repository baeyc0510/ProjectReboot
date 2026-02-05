#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProjectReboot/Interaction/PRInteractionTypes.h"
#include "PRInteractableInterface.generated.h"

class APRPlayerCharacter;

UINTERFACE(MinimalAPI, BlueprintType)
class UPRInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTREBOOT_API IPRInteractableInterface
{
	GENERATED_BODY()

public:
	/*~ IPRInteractableInterface ~*/
	// 상호작용 가능 여부
	virtual bool CanInteract(APawn* Interactor) const = 0;

	// 상호작용 실행
	virtual void Interact(APawn* Interactor) = 0;

	/*~ Optional ~*/
	// 상호작용 가능 상태 설정 (선택적 구현)
	virtual void SetInteractable(bool bEnabled) {}
	
	// UI 힌트 텍스트 (선택적 구현)
	virtual FText GetInteractionText() const { return FText::GetEmpty(); }

	// UI 표시용 정보 (선택적 구현)
	virtual void GetInteractionInfo(APawn* Interactor, FPRInteractionInfo& OutInfo) const
	{
		OutInfo = FPRInteractionInfo();
		OutInfo.DisplayText = GetInteractionText();
		OutInfo.bIsEnabled = CanInteract(Interactor);
	}

	// 상호작용 포커스 획득 (플레이어가 이 액터를 바라봄)
	virtual void OnGainInteractFocus(APawn* Interactor) {}

	// 상호작용 포커스 상실 (플레이어가 다른 곳을 바라봄)
	virtual void OnLoseInteractFocus(APawn* Interactor) {}
};