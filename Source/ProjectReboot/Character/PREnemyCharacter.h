// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRCharacterBase.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySet.h"
#include "PREnemyCharacter.generated.h"


class UMotionWarpingComponent;
class UWidgetComponent;
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
	
	UFUNCTION(BlueprintPure)
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	
protected:
	/*~ AActor Interfaces ~*/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void BindViewModels();
	virtual void UnBindViewModels();
protected:
	/** Animation */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Animation")
	TArray<TSubclassOf<UAnimInstance>> EnemyAnimLayers;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, BlueprintGetter=GetAIConfig,  Category = "PR Enemy")
	TObjectPtr<UPRAIConfig> AIConfig;
	
	/** AbilityConfig */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Enemy")
	TObjectPtr<UPRAbilitySet> EnemyAbilitySet;
	
	/*~ Components ~*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> StatusWidgetComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
	
private:
	bool bIsStrafeMode;
	FPRAbilitySetHandles AbilitySetHandles;
};