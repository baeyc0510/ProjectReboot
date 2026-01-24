

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PREnemyData.generated.h"

class UPRAbilitySet;
class UPRAIConfig;
/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPREnemyData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	FText Name;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UPRAbilitySet> AbilitySet;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UPRAIConfig> AIConfig;
};
