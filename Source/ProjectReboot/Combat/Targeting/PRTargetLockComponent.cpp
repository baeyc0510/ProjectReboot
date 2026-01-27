// PRTargetLockComponent.cpp
#include "PRTargetLockComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectReboot/Equipment/PREquipmentInterface.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/Equipment/Weapon/MissileWeaponInstance.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/UI/LockOn/PRLockOnViewModel.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogPRTargetLock, Log, All);

UPRTargetLockComponent::UPRTargetLockComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// 기본 락온 대상 태그 설정
	LockableTargetTag = TAG_Target_Lockable;
}

void UPRTargetLockComponent::BeginPlay()
{
	Super::BeginPlay();
	CacheEquipmentManager();
}

void UPRTargetLockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsLockOnEnabled)
	{
		return;
	}

	// 범위 내 락온 가능 타겟 탐색
	TArray<AActor*> PotentialTargets = FindLockableTargets();

	// 기존 타겟 상태 업데이트 (유예 시간 적용)
	for (int32 Index = LockingTargets.Num() - 1; Index >= 0; --Index)
	{
		FLockingTarget& LockingTarget = LockingTargets[Index];

		// 무효화된 타겟 즉시 제거
		if (!LockingTarget.Target.IsValid())
		{
			LockingTargets.RemoveAt(Index);
			continue;
		}

		AActor* Target = LockingTarget.Target.Get();
		bool bCurrentlyInCondition = PotentialTargets.Contains(Target);

		if (bCurrentlyInCondition)
		{
			// 조건 충족 - 유예 시간 초기화
			LockingTarget.bIsInCondition = true;
			LockingTarget.OutOfConditionTime = 0.0f;
		}
		else
		{
			// 조건 미충족 - 유예 시간 누적
			LockingTarget.bIsInCondition = false;
			LockingTarget.OutOfConditionTime += DeltaTime;

			// 유예 시간 초과 시 제거
			if (LockingTarget.OutOfConditionTime > LockGracePeriod)
			{
				bool bWasLocked = LockingTarget.bIsLocked;

				UE_LOG(LogPRTargetLock, Verbose, TEXT("락온 범위 이탈: %s"), *Target->GetName());
				if (bWasLocked)
				{
					UE_LOG(LogPRTargetLock, Verbose, TEXT("락온 해제: %s"), *Target->GetName());
				}

				// ViewModel 가시성 해제
				if (UPRLockOnViewModel* ViewModel = LockingTarget.ViewModel.Get())
				{
					ViewModel->SetVisible(false);
				}

				// 태그 이벤트 언바인딩
				UnbindTargetTagEvent(LockingTarget);

				LockingTargets.RemoveAt(Index);
			}
		}
	}

	// 새로운 타겟 추가
	for (AActor* Target : PotentialTargets)
	{
		bool bAlreadyTracking = false;
		for (const FLockingTarget& LockingTarget : LockingTargets)
		{
			if (LockingTarget.Target.Get() == Target)
			{
				bAlreadyTracking = true;
				break;
			}
		}

		if (!bAlreadyTracking)
		{
			// 최대 락온 수 체크 (장착된 미사일 수 기반)
			int32 MaxCount = GetMaxLockCount();
			if (MaxCount > 0 && LockingTargets.Num() >= MaxCount)
			{
				continue;
			}

			FLockingTarget NewTarget;
			NewTarget.Target = Target;
			NewTarget.LockingTime = 0.0f;
			NewTarget.bIsLocked = false;
			NewTarget.OutOfConditionTime = 0.0f;
			NewTarget.bIsInCondition = true;
			// ViewModel 할당 및 가시성 활성화
			NewTarget.ViewModel = GetOrCreateLockOnViewModel(Target);
			if (UPRLockOnViewModel* ViewModel = NewTarget.ViewModel.Get())
			{
				ViewModel->SetVisible(true);
			}

			// 태그 제거 이벤트 바인딩
			BindTargetTagEvent(NewTarget);

			LockingTargets.Add(NewTarget);

			UE_LOG(LogPRTargetLock, Verbose, TEXT("락온 범위 진입: %s"), *Target->GetName());
		}
	}

	// 락온 진행 업데이트
	UpdateLockingTargets(DeltaTime);

	// WeaponInstance에 동기화
	SyncLockedTargetsToWeapon();
}

void UPRTargetLockComponent::SetLockOnEnabled(bool bEnabled)
{
	bIsLockOnEnabled = bEnabled;
	SetComponentTickEnabled(bEnabled);

	UE_LOG(LogPRTargetLock, Log, TEXT("락온 %s"), bEnabled ? TEXT("활성화") : TEXT("비활성화"));

	if (!bEnabled)
	{
		ClearAllLocks();
	}
}

TArray<AActor*> UPRTargetLockComponent::GetLockedTargets() const
{
	TArray<AActor*> Locked;
	for (const FLockingTarget& LockingTarget : LockingTargets)
	{
		if (LockingTarget.bIsLocked && LockingTarget.Target.IsValid())
		{
			Locked.Add(LockingTarget.Target.Get());
		}
	}
	return Locked;
}

TArray<AActor*> UPRTargetLockComponent::GetLockingTargets() const
{
	TArray<AActor*> Locking;
	for (const FLockingTarget& LockingTarget : LockingTargets)
	{
		if (!LockingTarget.bIsLocked && LockingTarget.Target.IsValid())
		{
			Locking.Add(LockingTarget.Target.Get());
		}
	}
	return Locking;
}

float UPRTargetLockComponent::GetLockProgress(AActor* Target) const
{
	for (const FLockingTarget& LockingTarget : LockingTargets)
	{
		if (LockingTarget.Target.Get() == Target)
		{
			if (LockingTarget.bIsLocked)
			{
				return 1.0f;
			}
			return FMath::Clamp(LockingTarget.LockingTime / LockOnTime, 0.0f, 1.0f);
		}
	}
	return 0.0f;
}

void UPRTargetLockComponent::ClearAllLocks()
{
	for (FLockingTarget& LockingTarget : LockingTargets)
	{
		if (LockingTarget.Target.IsValid())
		{
			AActor* Target = LockingTarget.Target.Get();
			UE_LOG(LogPRTargetLock, Verbose, TEXT("락온 초기화: %s"), *Target->GetName());

			// ViewModel 가시성 해제
			if (UPRLockOnViewModel* ViewModel = LockingTarget.ViewModel.Get())
			{
				ViewModel->SetVisible(false);
			}

			// 태그 이벤트 언바인딩
			UnbindTargetTagEvent(LockingTarget);
		}
	}

	LockingTargets.Empty();
	SyncLockedTargetsToWeapon();
}

TArray<AActor*> UPRTargetLockComponent::FindLockableTargets() const
{
	TArray<AActor*> Result;

	UWorld* World = GetWorld();
	AActor* Owner = GetOwner();
	if (!IsValid(World) || !IsValid(Owner))
	{
		return Result;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	GetCameraViewInfo(CameraLocation, CameraRotation);

	// 범위 내 액터 탐색
	TArray<AActor*> OverlappedActors;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(Owner);

	check(TargetFilterClass);
	
	UKismetSystemLibrary::SphereOverlapActors(
		World,
		CameraLocation,
		LockOnRange,
		TArray<TEnumAsByte<EObjectTypeQuery>>(),
		TargetFilterClass,
		IgnoreActors,
		OverlappedActors
	);

	// 락온 가능한 타겟 필터링
	for (AActor* Actor : OverlappedActors)
	{
		if (CanLockTarget(Actor))
		{
			Result.Add(Actor);
		}
	}

	return Result;
}

bool UPRTargetLockComponent::CanLockTarget(AActor* Target) const
{
	if (!IsValid(Target))
	{
		return false;
	}

	// GameplayTag 체크
	if (LockableTargetTag.IsValid())
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
		if (!IsValid(TargetASC))
		{
			return false;
		}

		if (!TargetASC->HasMatchingGameplayTag(LockableTargetTag))
		{
			return false;
		}
	}

	// 시야각 체크
	if (!IsTargetInViewCone(Target))
	{
		return false;
	}

	// Line of Sight 체크
	if (!HasLineOfSightTo(Target))
	{
		return false;
	}

	return true;
}

bool UPRTargetLockComponent::IsTargetInViewCone(AActor* Target) const
{
	FVector CameraLocation;
	FRotator CameraRotation;
	GetCameraViewInfo(CameraLocation, CameraRotation);

	FVector CameraForward = CameraRotation.Vector();
	float HalfConeAngle = LockOnConeAngle * 0.5f;

	// 멀티 포인트 체크 - 하나라도 시야각 내에 있으면 통과
	TArray<FVector> CheckLocations = GetTargetCheckLocations(Target);
	for (const FVector& CheckLocation : CheckLocations)
	{
		FVector ToTarget = (CheckLocation - CameraLocation).GetSafeNormal();
		float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
		float AngleRad = FMath::Acos(DotProduct);
		float AngleDeg = FMath::RadiansToDegrees(AngleRad);

		if (AngleDeg <= HalfConeAngle)
		{
			return true;
		}
	}

	return false;
}

bool UPRTargetLockComponent::HasLineOfSightTo(AActor* Target) const
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	GetCameraViewInfo(CameraLocation, CameraRotation);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(Target);

	// 멀티 포인트 체크 - 하나라도 보이면 시야 확보
	TArray<FVector> CheckLocations = GetTargetCheckLocations(Target);
	for (const FVector& CheckLocation : CheckLocations)
	{
		FHitResult Hit;
		bool bHit = World->LineTraceSingleByChannel(
			Hit,
			CameraLocation,
			CheckLocation,
			ECC_Visibility,
			QueryParams
		);

		if (!bHit)
		{
			return true;
		}
	}

	return false;
}

TArray<FVector> UPRTargetLockComponent::GetTargetCheckLocations(AActor* Target) const
{
	TArray<FVector> Locations;

	if (!IsValid(Target))
	{
		return Locations;
	}

	FVector ActorLocation = Target->GetActorLocation();

	
	UCapsuleComponent* Capsule = nullptr;
	
	// CombatInterface인 경우 CombatCapsuleComponent
	if (const IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(Target))
	{
		Capsule = CombatInterface->GetCombatCapsuleComponent();
	}
	// 캐릭터인 경우 Capsule 기반
	else if (const ACharacter* Character = Cast<ACharacter>(Target))
	{
		Capsule = Character->GetCapsuleComponent();
	}
	
	if (Capsule)
	{
		float HalfHeight = Capsule->GetScaledCapsuleHalfHeight();

		// 머리 (상단)
		Locations.Add(ActorLocation + FVector(0, 0, HalfHeight * 0.8f));
		// 몸통 (중앙)
		Locations.Add(ActorLocation + FVector(0, 0, HalfHeight * 0.3f));
		// 하체 (하단)
		Locations.Add(ActorLocation - FVector(0, 0, HalfHeight * 0.3f));

		return Locations;
	}

	// 폴백: 액터 위치만 사용
	Locations.Add(ActorLocation);
	return Locations;
}

void UPRTargetLockComponent::UpdateLockingTargets(float DeltaTime)
{
	for (FLockingTarget& LockingTarget : LockingTargets)
	{
		if (!LockingTarget.Target.IsValid())
		{
			continue;
		}

		if (LockingTarget.bIsLocked)
		{
			continue;
		}

		// 조건 충족 시에만 락온 진행
		if (LockingTarget.bIsInCondition)
		{
			LockingTarget.LockingTime += DeltaTime;
		}

		// 진행률 업데이트
		float Progress = FMath::Clamp(LockingTarget.LockingTime / LockOnTime, 0.0f, 1.0f);

		// ViewModel 갱신
		UPRLockOnViewModel* ViewModel = LockingTarget.ViewModel.Get();
		if (ViewModel)
		{
			ViewModel->SetProgress(Progress);
		}

		// 락온 완료 체크
		if (FMath::IsNearlyEqual(Progress, 1.0f))
		{
			LockingTarget.bIsLocked = true;
			UE_LOG(LogPRTargetLock, Log, TEXT("락온 완료: %s"), *LockingTarget.Target->GetName());

			// ViewModel 락온 완료 상태 설정
			if (ViewModel)
			{
				ViewModel->SetLocked(true);
			}
		}
	}
}

void UPRTargetLockComponent::SyncLockedTargetsToWeapon()
{
	UMissileWeaponInstance* MissileWeapon = GetMissileWeaponInstance();
	if (!IsValid(MissileWeapon))
	{
		return;
	}

	TArray<AActor*> LockedTargets = GetLockedTargets();
	MissileWeapon->SetLockedTargets(LockedTargets);
}

void UPRTargetLockComponent::CacheEquipmentManager()
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		CachedEquipmentManager.Reset();
		return;
	}

	IPREquipmentInterface* EquipmentInterface = Cast<IPREquipmentInterface>(Owner);
	if (!EquipmentInterface)
	{
		CachedEquipmentManager.Reset();
		return;
	}

	CachedEquipmentManager = EquipmentInterface->GetEquipmentManager();
}

UMissileWeaponInstance* UPRTargetLockComponent::GetMissileWeaponInstance() const
{
	if (!CachedEquipmentManager.IsValid())
	{
		const_cast<UPRTargetLockComponent*>(this)->CacheEquipmentManager();
	}

	UPREquipmentManagerComponent* EquipmentManager = CachedEquipmentManager.Get();
	if (!IsValid(EquipmentManager))
	{
		return nullptr;
	}

	// 미사일 타입 무기 슬롯에서 조회
	UEquipmentInstance* Instance = EquipmentManager->GetEquipmentInstance(TAG_Equipment_Slot_Weapon_Primary);
	return Cast<UMissileWeaponInstance>(Instance);
}

int32 UPRTargetLockComponent::GetMaxLockCount() const
{
	UMissileWeaponInstance* MissileWeapon = GetMissileWeaponInstance();
	if (!IsValid(MissileWeapon))
	{
		return 0;
	}

	return MissileWeapon->GetLoadedMissiles();
}

void UPRTargetLockComponent::GetCameraViewInfo(FVector& OutLocation, FRotator& OutRotation) const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		OutLocation = FVector::ZeroVector;
		OutRotation = FRotator::ZeroRotator;
		return;
	}

	APawn* Pawn = Cast<APawn>(Owner);
	if (IsValid(Pawn))
	{
		APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
		if (IsValid(PC))
		{
			PC->GetPlayerViewPoint(OutLocation, OutRotation);
			return;
		}
	}

	// 폴백: 액터 위치/방향
	OutLocation = Owner->GetActorLocation();
	OutRotation = Owner->GetActorRotation();
}

UPRLockOnViewModel* UPRTargetLockComponent::GetOrCreateLockOnViewModel(AActor* Target)
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!IsValid(LP) || !IsValid(Target))
	{
		return nullptr;
	}

	UPRViewModelSubsystem* VMS = LP->GetSubsystem<UPRViewModelSubsystem>();
	if (!IsValid(VMS))
	{
		return nullptr;
	}

	return VMS->GetOrCreateActorViewModel<UPRLockOnViewModel>(Target);
}

ULocalPlayer* UPRTargetLockComponent::GetLocalPlayer() const
{
	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return nullptr;
	}

	APawn* Pawn = Cast<APawn>(Owner);
	if (!IsValid(Pawn))
	{
		return nullptr;
	}

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!IsValid(PC))
	{
		return nullptr;
	}

	return PC->GetLocalPlayer();
}

void UPRTargetLockComponent::BindTargetTagEvent(FLockingTarget& LockingTarget)
{
	if (!LockingTarget.Target.IsValid() || !LockableTargetTag.IsValid())
	{
		return;
	}

	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(LockingTarget.Target.Get());
	if (!IsValid(ASC))
	{
		return;
	}

	LockingTarget.TargetASC = ASC;

	// 태그 제거 이벤트 바인딩
	LockingTarget.TagEventHandle = ASC->RegisterGameplayTagEvent(
		LockableTargetTag,
		EGameplayTagEventType::NewOrRemoved
	).AddLambda([this, WeakTarget = LockingTarget.Target](const FGameplayTag Tag, int32 NewCount)
	{
		// 태그가 제거되었을 때 (NewCount == 0)
		if (NewCount == 0 && WeakTarget.IsValid())
		{
			HandleLockableTagRemoved(WeakTarget.Get());
		}
	});
}

void UPRTargetLockComponent::UnbindTargetTagEvent(FLockingTarget& LockingTarget)
{
	if (!LockingTarget.TargetASC.IsValid() || !LockingTarget.TagEventHandle.IsValid())
	{
		return;
	}

	LockingTarget.TargetASC->RegisterGameplayTagEvent(
		LockableTargetTag,
		EGameplayTagEventType::NewOrRemoved
	).Remove(LockingTarget.TagEventHandle);

	LockingTarget.TagEventHandle.Reset();
	LockingTarget.TargetASC.Reset();
}

void UPRTargetLockComponent::HandleLockableTagRemoved(AActor* Target)
{
	if (!IsValid(Target))
	{
		return;
	}

	// 해당 타겟을 LockingTargets에서 찾아 제거
	for (int32 Index = LockingTargets.Num() - 1; Index >= 0; --Index)
	{
		FLockingTarget& LockingTarget = LockingTargets[Index];

		if (LockingTarget.Target.Get() == Target)
		{
			UE_LOG(LogPRTargetLock, Log, TEXT("락온 대상 태그 제거됨: %s"), *Target->GetName());

			// ViewModel 가시성 해제
			if (UPRLockOnViewModel* ViewModel = LockingTarget.ViewModel.Get())
			{
				ViewModel->SetVisible(false);
			}

			// 이벤트 언바인딩
			UnbindTargetTagEvent(LockingTarget);

			LockingTargets.RemoveAt(Index);
			break;
		}
	}

	SyncLockedTargetsToWeapon();
}
