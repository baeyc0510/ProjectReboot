// PRPlayerCameraManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Camera/PlayerCameraManager.h"
#include "PRPlayerCameraManager.generated.h"

class UAbilitySystemComponent;
class UPRCameraComponent;

UCLASS()
class PROJECTREBOOT_API APRPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void BindToASC(UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void UnbindFromASC();

private:
	UPRCameraComponent* FindPRCameraComponent() const;
	void ReevaluateActiveSetting();
	void HandleStateTagChanged(const FGameplayTag Tag, int32 NewCount);

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	struct FTagEventHandle { FDelegateHandle Handle; FGameplayTag Tag; };
	TArray<FTagEventHandle> TagEventHandles;
	FGameplayTagContainer ActiveConditionTags;
};