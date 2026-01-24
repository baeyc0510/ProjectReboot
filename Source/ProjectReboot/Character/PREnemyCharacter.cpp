// Fill out your copyright notice in the Description page of Project Settings.


#include "PREnemyCharacter.h"

#include "MotionWarpingComponent.h"
#include "PREnemyData.h"
#include "Components/WidgetComponent.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"
#include "ProjectReboot/UI/Enemy/PREnemyStatusViewModel.h"
#include "ProjectReboot/UI/Enemy/PREnemyStatusWidget.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

// Sets default values
APREnemyCharacter::APREnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;
	
	StatusWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusWidgetComponent"));
	StatusWidgetComponent->SetupAttachment(RootComponent);
	
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
}

FText APREnemyCharacter::GetEnemyDisplayName() const
{
	if (EnemyData)
	{
		return EnemyData->Name;
	}
	return FText::FromString(GetName());
}

UPRAIConfig* APREnemyCharacter::GetAIConfig() const
{
	if (EnemyData)
	{
		return EnemyData->AIConfig;
	}
	return nullptr;
}

void APREnemyCharacter::SetStrafeMode(bool bEnable)
{
	bIsStrafeMode = bEnable;
}

void APREnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		for (TSubclassOf<UAnimInstance>& AnimLayerClass : EnemyAnimLayers)
		{
			MeshComponent->LinkAnimClassLayers(AnimLayerClass);	
		}
	}
	
	if (AbilitySystem)
	{
		AbilitySystem->InitAbilityActorInfo(this,this);
		if (EnemyData && EnemyData->AbilitySet)
		{
			AbilitySystem->GiveAbilitySet(EnemyData->AbilitySet, AbilitySetHandles);	
		}
	}
	
	BindViewModels();
}

void APREnemyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (AbilitySystem)
	{
		AbilitySetHandles.RemoveFromAbilitySystem();
	}
	
	UnBindViewModels();
}

void APREnemyCharacter::BindViewModels()
{
	if (!StatusWidgetComponent)
	{
		return;
	}
	
	UPREnemyStatusWidget* EnemyStatusWidget = Cast<UPREnemyStatusWidget>(StatusWidgetComponent->GetWidget());
	if (!EnemyStatusWidget)
	{
		return;
	}
	
	ULocalPlayer* LP = GetWorld()->GetFirstLocalPlayerFromController();
	if (!LP)
	{
		return;
	}
	
	UPRViewModelSubsystem* ViewModelSubsystem = LP->GetSubsystem<UPRViewModelSubsystem>();
	if (!ViewModelSubsystem)
	{
		return;
	}
	
	if (UPREnemyStatusViewModel* VM = ViewModelSubsystem->GetOrCreateActorViewModel<UPREnemyStatusViewModel>(this))
	{
		EnemyStatusWidget->BindViewModel(VM);
	}
}

void APREnemyCharacter::UnBindViewModels()
{
	if (ULocalPlayer* LP = GetWorld()->GetFirstLocalPlayerFromController())
	{
		if (UPRViewModelSubsystem* ViewModelSubsystem = LP->GetSubsystem<UPRViewModelSubsystem>())
		{
			ViewModelSubsystem->RemoveAllViewModelsForActor(this);
		}
	}
}
