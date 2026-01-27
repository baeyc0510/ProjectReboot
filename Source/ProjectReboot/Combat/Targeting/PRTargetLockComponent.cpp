// PRTargetLockComponent.cpp
#include "PRTargetLockComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectReboot/Equipment/PREquipmentInterface.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/Equipment/Weapon/MissileWeaponInstance.h"
#include "ProjectReboot/PRGameplayTags.h"

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

	// 범위 이탈한 타겟 처리
	for (int32 Index = LockingTargets.Num() - 1; Index >= 0; --Index)
	{
		FLockingTarget& LockingTarget = LockingTargets[Index];

		if (!LockingTarget.Target.IsValid() || !PotentialTargets.Contains(LockingTarget.Target.Get()))
		{
			AActor* LostTarget = LockingTarget.Target.Get();
			bool bWasLocked = LockingTarget.bIsLocked;

			LockingTargets.RemoveAt(Index);

			if (IsValid(LostTarget))
			{
				UE_LOG(LogPRTargetLock, Verbose, TEXT("락온 범위 이탈: %s"), *LostTarget->GetName());
				if (bWasLocked)
				{
					UE_LOG(LogPRTargetLock, Verbose, TEXT("락온 해제: %s"), *LostTarget->GetName());
					OnTargetUnlocked.Broadcast(LostTarget);
				}
				OnTargetExitedRange.Broadcast(LostTarget);
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
			// 최대 락온 수 체크
			if (MaxLockCount > 0 && LockingTargets.Num() >= MaxLockCount)
			{
				continue;
			}

			FLockingTarget NewTarget;
			NewTarget.Target = Target;
			NewTarget.LockingTime = 0.0f;
			NewTarget.bIsLocked = false;
			LockingTargets.Add(NewTarget);

			UE_LOG(LogPRTargetLock, Verbose, TEXT("락온 범위 진입: %s"), *Target->GetName());
			OnTargetEnteredRange.Broadcast(Target);
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
	for (const FLockingTarget& LockingTarget : LockingTargets)
	{
		if (LockingTarget.Target.IsValid())
		{
			UE_LOG(LogPRTargetLock, Verbose, TEXT("락온 초기화: %s"), *LockingTarget.Target->GetName());
			if (LockingTarget.bIsLocked)
			{
				OnTargetUnlocked.Broadcast(LockingTarget.Target.Get());
			}
			OnTargetExitedRange.Broadcast(LockingTarget.Target.Get());
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
	FVector ToTarget = (Target->GetActorLocation() - CameraLocation).GetSafeNormal();

	float DotProduct = FVector::DotProduct(CameraForward, ToTarget);
	float AngleRad = FMath::Acos(DotProduct);
	float AngleDeg = FMath::RadiansToDegrees(AngleRad);

	return AngleDeg <= (LockOnConeAngle * 0.5f);
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

	FHitResult Hit;
	bool bHit = World->LineTraceSingleByChannel(
		Hit,
		CameraLocation,
		Target->GetActorLocation(),
		ECC_Visibility,
		QueryParams
	);

	// 장애물이 없으면 시야 확보
	return !bHit;
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

		// 락온 시간 누적
		LockingTarget.LockingTime += DeltaTime;

		// 진행률 업데이트 브로드캐스트
		float Progress = FMath::Clamp(LockingTarget.LockingTime / LockOnTime, 0.0f, 1.0f);
		OnLockOnProgressUpdated.Broadcast(LockingTarget.Target.Get(), Progress);

		// 락온 완료 체크
		if (LockingTarget.LockingTime >= LockOnTime)
		{
			LockingTarget.bIsLocked = true;
			UE_LOG(LogPRTargetLock, Log, TEXT("락온 완료: %s"), *LockingTarget.Target->GetName());
			OnTargetLocked.Broadcast(LockingTarget.Target.Get());
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
