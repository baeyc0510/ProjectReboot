// Fill out your copyright notice in the Description page of Project Settings.


#include "PREnemyCharacter.h"

#include "MotionWarpingComponent.h"
#include "PREnemyData.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AI/PRAIController.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySystemComponent.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "ProjectReboot/UI/Enemy/PREnemyStatusViewModel.h"
#include "ProjectReboot/UI/Enemy/PREnemyStatusWidget.h"
#include "ProjectReboot/UI/LockOn/PRLockOnViewModel.h"
#include "ProjectReboot/UI/LockOn/PRLockOnWidget.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

// Sets default values
APREnemyCharacter::APREnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.1f;
	bUseControllerRotationYaw = false;

	// AI 자동 Possess 설정
	AIControllerClass = APRAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// AI Sight가 동료 Enemy를 통과하도록 Visibility 채널 Ignore
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);

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

	// 킬 이벤트 전송 (웨이브 클리어 판정용)
	if (UWorld* World = GetWorld())
	{
		if (APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>())
		{
			GameState->AddEventCount(TAG_Event_Kill);
		}
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
	
	if (bIsStrafeMode)
	{
		GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else
	{
		GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void APREnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 스폰 위치 저장 (Patrol 용)
	SpawnLocation = GetActorLocation();

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

void APREnemyCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// 안전 코드 (스폰 버그로 인한 웨이브 클리어 불가 현상 방지)
	
	if (IsDead())
	{
		return;
	}
	
	// 추락 감지
	if (GetActorLocation().Z < SpawnLocation.Z - 2000.f)
	{
		FGameplayEffectContextHandle ContextHandle;
		Die(ContextHandle);
	}
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

void APREnemyCharacter::GetPrewarmChildren(TArray<UObject*>& OutChildren) const
{
	if (IsValid(EnemyData))
	{
		OutChildren.Add(EnemyData);
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
