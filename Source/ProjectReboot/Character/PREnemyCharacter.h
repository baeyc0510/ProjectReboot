// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRCharacterBase.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySet.h"
#include "PREnemyCharacter.generated.h"


class UPRAbilitySet;
struct FPRAICombatConfig;
class UPRAIConfig;

UCLASS()
class PROJECTREBOOT_API APREnemyCharacter : public APRCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APREnemyCharacter();

	UFUNCTION(BlueprintPure)
	UPRAIConfig* GetAIConfig() const { return AIConfig; }

	UFUNCTION(BlueprintCallable)
	void SetStrafeMode(bool bEnable);
	
	UFUNCTION(BlueprintPure)
	bool IsStrafeMode() const {return bIsStrafeMode;}
	
protected:
	/*~ AActor Interfaces ~*/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, BlueprintGetter=GetAIConfig,  Category = "PR Enemy")
	TObjectPtr<UPRAIConfig> AIConfig;
	
	/** AbilityConfig */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Enemy")
	TObjectPtr<UPRAbilitySet> EnemyAbilitySet;
	
private:
	bool bIsStrafeMode;
	FPRAbilitySetHandles AbilitySetHandles;
};