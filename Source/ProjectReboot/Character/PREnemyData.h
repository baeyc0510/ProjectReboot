

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProjectReboot/Game/PRPrewarmInterface.h"
#include "PREnemyData.generated.h"

class UPRAbilitySet;
class UPRAIConfig;
/**
 *
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPREnemyData : public UDataAsset, public IPRPrewarmInterface
{
	GENERATED_BODY()

public:
	/*~ IPRPrewarmInterface ~*/
	// 프리웜 대상 자식 오브젝트 수집
	virtual void GetPrewarmChildren(TArray<UObject*>& OutChildren) const override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	FText Name;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UPRAbilitySet> AbilitySet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UPRAIConfig> AIConfig;
};
