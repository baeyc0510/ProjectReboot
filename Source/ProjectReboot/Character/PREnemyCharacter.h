// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRCharacterBase.h"
#include "PREnemyCharacter.generated.h"


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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, BlueprintGetter=GetAIConfig,  Category = "AI")
	TObjectPtr<UPRAIConfig> AIConfig;
	
private:
	bool bIsStrafeMode;
};