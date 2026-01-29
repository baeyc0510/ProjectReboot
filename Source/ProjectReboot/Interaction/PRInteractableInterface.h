#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
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

	// UI 힌트 텍스트
	virtual FText GetInteractionText() const { return FText::GetEmpty(); }
};