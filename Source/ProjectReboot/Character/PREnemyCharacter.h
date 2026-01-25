// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRCharacterBase.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySet.h"
#include "PREnemyCharacter.generated.h"


class UPREnemyData;
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
	UPREnemyData* GetEnemyData() const {return EnemyData;}
	
	UFUNCTION(BlueprintPure)
	FText GetEnemyDisplayName() const;
	
	UFUNCTION(BlueprintPure)
	UPRAIConfig* GetAIConfig() const;
	
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
	
	/** EnemyConfig */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR Enemy")
	TObjectPtr<UPREnemyData> EnemyData;
	
	/*~ Components ~*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> StatusWidgetComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
	
private:
	bool bIsStrafeMode;
	FPRAbilitySetHandles AbilitySetHandles;
};