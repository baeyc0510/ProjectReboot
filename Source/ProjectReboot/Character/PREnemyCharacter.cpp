// Fill out your copyright notice in the Description page of Project Settings.


#include "PREnemyCharacter.h"

#include "MotionWarpingComponent.h"
#include "PREnemyData.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"
#include "ProjectReboot/UI/Enemy/PREnemyStatusViewModel.h"
#include "ProjectReboot/UI/Enemy/PREnemyStatusWidget.h"
#include "ProjectReboot/UI/LockOn/PRLockOnViewModel.h"
#include "ProjectReboot/UI/LockOn/PRLockOnWidget.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

// Sets default values
APREnemyCharacter::APREnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bUseControllerRotationYaw = false;
	
	StatusWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatusWidgetComponent"));
	StatusWidgetComponent->SetupAttachment(RootComponent);
	
	LockOnWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidgetComponent"));
	LockOnWidgetComponent->SetupAttachment(RootComponent);

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));
	
	DefaultAbilitySystemTags.AddTag(TAG_Target_Lockable);
}

void APREnemyCharacter::FinishDie()
{
	Super::FinishDie();
	
	DestructWidget(StatusWidgetComponent);
	DestructWidget(LockOnWidgetComponent);
	
	if (AbilitySystem)
	{
		// 락온 대상에서 제거
		AbilitySystem->RemoveLooseGameplayTag(TAG_Target_Lockable);
	}
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
		AddDefaultAbilitySystemTags();
		if (EnemyData && EnemyData->AbilitySet)
		{
			AbilitySystem->GiveAbilitySet(EnemyData->AbilitySet, AbilitySetHandles);	
		}
	}
	
	if (CombatCapsuleComponent)
	{
		if (StatusWidgetComponent)
		{
			StatusWidgetComponent->AttachToComponent(CombatCapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}
		if (LockOnWidgetComponent)
		{
			LockOnWidgetComponent->AttachToComponent(CombatCapsuleComponent, FAttachmentTransformRules::KeepRelativeTransform);
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

	// EnemyStatusWidget 바인딩
	if (UPREnemyStatusWidget* EnemyStatusWidget = Cast<UPREnemyStatusWidget>(StatusWidgetComponent->GetWidget()))
	{
		if (UPREnemyStatusViewModel* VM = ViewModelSubsystem->GetOrCreateActorViewModel<UPREnemyStatusViewModel>(this))
		{
			EnemyStatusWidget->BindViewModel(VM);
		}
	}

	// LockOnWidget 바인딩
	if (UPRLockOnWidget* LockOnWidget = Cast<UPRLockOnWidget>(LockOnWidgetComponent->GetWidget()))
	{
		if (UPRLockOnViewModel* VM = ViewModelSubsystem->GetOrCreateActorViewModel<UPRLockOnViewModel>(this))
		{
			LockOnWidget->BindViewModel(VM);
		}
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

void APREnemyCharacter::DestructWidget(UWidgetComponent* WidgetComponent)
{
	if (WidgetComponent)
	{
		if (UUserWidget* Widget = WidgetComponent->GetWidget())
		{
			Widget->RemoveFromParent();
			Widget->Destruct();
		}
	}
}
