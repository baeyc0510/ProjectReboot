// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRCharacterBase.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySet.h"
#include "ProjectReboot/Game/PRPrewarmInterface.h"
#include "PREnemyCharacter.generated.h"


class UPREnemyData;
class UMotionWarpingComponent;
class UWidgetComponent;
class UPRAbilitySet;
struct FPRAICombatConfig;
class UPRAIConfig;
class UPRLockOnViewModel;

UCLASS()
class PROJECTREBOOT_API APREnemyCharacter : public APRCharacterBase, public IPRPrewarmInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APREnemyCharacter();
	
	/*~ ICombatInterface ~*/
	virtual void FinishDie() override;
	
	/*~ APREnemyCharacter Interface ~*/
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

	// 스폰 위치 반환 (Patrol용)
	UFUNCTION(BlueprintPure)
	FVector GetSpawnLocation() const {return SpawnLocation;}
	
protected:
	/*~ AActor Interfaces ~*/
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	
	/*~ IPRPrewarmInterface ~*/
	// 프리웜 대상 자식 오브젝트 수집
	virtual void GetPrewarmChildren(TArray<UObject*>& OutChildren) const override;

	virtual void BindViewModels();
	virtual void UnBindViewModels();
	
	/*~ APREnemyCharacter Interface ~*/
	void DestructWidget(UWidgetComponent* WidgetComponent);
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> LockOnWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;
	
private:
	bool bIsStrafeMode;
	FPRAbilitySetHandles AbilitySetHandles;

	// 스폰 위치 (Patrol 용)
	FVector SpawnLocation;
};