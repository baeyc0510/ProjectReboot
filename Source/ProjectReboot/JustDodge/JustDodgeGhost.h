// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JustDodgeGhost.generated.h"

class UCapsuleComponent;
class UAbilitySystemComponent;


UCLASS()
class PROJECTREBOOT_API AJustDodgeGhost : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AJustDodgeGhost();
	
	virtual void BeginPlay() override;
	void SetOwnerASC(UAbilitySystemComponent* InASC);
	void InitCapsuleSize(UCapsuleComponent* InOriginalCapsule);

private:
	UFUNCTION()
	void HandleOnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CapsuleComponent;
	
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> OwnerASC;
};
