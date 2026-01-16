// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySet.h"
#include "PRPlayerCharacter.generated.h"

class UPRWeaponAttributeSet;
class UPRCrosshairViewModel;
class UPRCommonAttributeSet;
class UPRAbilitySet;
class URogueliteAbilityHandlerComponent;
class UPRInputConfig;
class UPRAbilitySystemComponent;
class UPREquipmentManagerComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class PROJECTREBOOT_API APRPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	APRPlayerCharacter();
	
	/*~ IAbilitySystemInterface ~*/
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/*~ APRPlayerCharacter Interfaces ~*/

	/** Returns CameraBoom subobject **/
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	float GetDesiredLookDirection() const {return DesiredLookDirection;}
	
	bool IsCrouching() const;
	bool IsSprinting() const;
	
protected:
	/*~ AActor Interfaces ~*/
	virtual void BeginPlay() override;
	
	/*~ APawn Interfaces ~*/
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	
	/*~ APRPlayerCharacter Interfaces ~*/
	
	virtual void OnTaggedInputPressed(FGameplayTag InputTag);
	virtual void OnTaggedInputReleased(FGameplayTag InputTag);
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
private:
	/*~ ViewModel ~*/
	UPRCrosshairViewModel* GetCrosshairViewModel() const;
	void BindCrosshairViewModel();
	void UnbindCrosshairViewModel();
	
	
protected:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* FollowCamera;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	float BaseMoveSpeed = 500.f;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsCrouching = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PR AbilitySystem")
	TObjectPtr<UPRAbilitySet> DefaultAbilitySet;
	
	/*~ Components ~*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AbilitySystem")
	UPRAbilitySystemComponent* AbilitySystem;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PR AbilitySystem")
	URogueliteAbilityHandlerComponent* RogueliteAbilityHandler;
	
	UPROPERTY(BlueprintReadOnly, Category = "PR AbilitySystem")
	UPRCommonAttributeSet* CommonAttributeSet;
	
	UPROPERTY(BlueprintReadOnly, Category = "PR AbilitySystem")
	UPRWeaponAttributeSet* WeaponAttributeSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equipment)
	UPREquipmentManagerComponent* EquipmentManager;

private:
	float DesiredLookDirection;
	FPRAbilitySetHandles DefaultAbilitySetHandles;
};
