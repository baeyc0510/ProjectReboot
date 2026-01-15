// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"
#include "PRPlayerCharacter.generated.h"

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
	
	bool IsCrouching() const {return bIsCrouching;}
	bool IsSprinting() const {return bIsSprinting;}
	
protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;
	
	virtual void OnTaggedInputPressed(FGameplayTag InputTag);
	virtual void OnTaggedInputReleased(FGameplayTag InputTag);
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void ToggleCrouch();
	void Crouch();
	void UnCrouch();
	
	void Sprint();
	void StopSprint();
	
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

	/** Event Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* CrouchAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SprintAction;
	
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
	bool bIsSprinting = false;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement)
	bool bIsCrouching = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AbilitySystem)
	UPRAbilitySystemComponent* AbilitySystem;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AbilitySystem)
	URogueliteAbilityHandlerComponent* RogueliteAbilityHandler;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Equipment)
	UPREquipmentManagerComponent* EquipmentManager;
	
private:
	float DesiredLookDirection;
};
