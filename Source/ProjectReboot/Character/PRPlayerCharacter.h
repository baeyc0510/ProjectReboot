// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "PRCharacterBase.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySet.h"
#include "ProjectReboot/Equipment/PREquipmentInterface.h"
#include "ProjectReboot/Game/PRPrewarmInterface.h"
#include "PRPlayerCharacter.generated.h"

class UAIPerceptionStimuliSourceComponent;
class UCameraComponent;
class UPRViewModelSubsystem;
class USpringArmComponent;
class UPRWeaponAttributeSet;
class UPRCrosshairViewModel;
class UPRAbilitySet;
class URogueliteAbilityHandlerComponent;
class UPRInputConfig;
class UPREquipmentManagerComponent;
class UPRCameraComponent;
class UPREnemyDetectionComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class PROJECTREBOOT_API APRPlayerCharacter : public APRCharacterBase, public IPREquipmentInterface, public IGenericTeamAgentInterface, public IPRPrewarmInterface
{
	GENERATED_BODY()

public:
	APRPlayerCharacter();

	/*~ IGenericTeamAgentInterface ~*/
	virtual FGenericTeamId GetGenericTeamId() const override;

	/*~ APRPlayerCharacter Interfaces ~*/
	UFUNCTION(BlueprintCallable, Category = "Camera")
	UCameraComponent* DetachCamera();
	
	// 카메라 컴포넌트 반환
	FORCEINLINE UPRCameraComponent* GetCameraComponent() const { return CameraComponent; }

	// 카메라 래그 없이 텔레포트
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void TeleportWithoutCameraLag(const FVector& DestLocation, const FRotator& DestRotation);
	
	float GetDesiredLookDirection() const {return DesiredLookDirection;}
	
	bool IsCrouching() const;
	bool IsSprinting() const;
	bool IsAiming() const;
	bool IsMovementInputBlocked() const;
	
protected:
	/*~ AActor Interfaces ~*/
	virtual void BeginPlay() override;
	
	/*~ APawn Interfaces ~*/
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	
	/*~ IPRCombatInterface ~*/
	virtual void FinishDie() override;
	
	/*~ IPREquipmentInterface ~*/
	virtual UPREquipmentManagerComponent* GetEquipmentManager() const override {return EquipmentManager;}

	/*~ IPRPrewarmInterface ~*/
	// 프리웜 대상 에셋 목록 수집
	virtual void GetPrewarmNiagaraAssets(TArray<TSoftObjectPtr<UNiagaraSystem>>& OutAssets) const override;

	// 프리웜 대상 자식 오브젝트 수집
	virtual void GetPrewarmChildren(TArray<UObject*>& OutChildren) const override;
	
	/*~ APRPlayerCharacter Interfaces ~*/
	
	virtual void OnTaggedInputPressed(FGameplayTag InputTag);
	virtual void OnTaggedInputReleased(FGameplayTag InputTag);
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/*~ States ~*/
	UFUNCTION()
	void HandleStateTagChanged(const FGameplayTag Tag, int32 NewCount);
	
private:
	/*~ ViewModel ~*/
	UPRViewModelSubsystem* GetViewModelSubsystem() const;
	void BindViewModels();
	void UnbindViewModels();

	UFUNCTION()
	void HandleFullyDeath();
	
protected:
	// 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UPRCameraComponent* CameraComponent;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* DefaultMappingContext;
	
	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;
	
	/** InputConfig */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UPRInputConfig* InputConfig;
	
	/** AbilityConfig */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR AbilitySystem")
	TObjectPtr<UPRAbilitySet> DefaultAbilitySet;
	
	/*~ Components ~*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AbilitySystem")
	URogueliteAbilityHandlerComponent* RogueliteAbilityHandler;
	
	UPROPERTY(BlueprintReadOnly, Category = "PR AbilitySystem")
	UPRWeaponAttributeSet* WeaponAttributeSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	UPREquipmentManagerComponent* EquipmentManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionStimuliSourceComponent* StimuliSourceComponent;

	// // 적 감지 컴포넌트 (EnemyIndicator 시스템)
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Detection")
	// TObjectPtr<UPREnemyDetectionComponent> EnemyDetectionComponent;
private:
	float DesiredLookDirection;
	FPRAbilitySetHandles DefaultAbilitySetHandles;
	FDelegateHandle StateChangedDelegateHandle;
	FTimerHandle DeathTimerHandle;
	bool bIsFullyDead = false;
};
