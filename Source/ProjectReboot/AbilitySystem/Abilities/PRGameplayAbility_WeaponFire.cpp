// PRGameplayAbility_WeaponFire.cpp
#include "PRGameplayAbility_WeaponFire.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameFramework/PlayerController.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
#include "ProjectReboot/Equipment/PREquipmentInterface.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/Equipment/Weapon/WeaponInstance.h"

UPRGameplayAbility_WeaponFire::UPRGameplayAbility_WeaponFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EPRAbilityActivationPolicy::OnInputTriggered;
}

bool UPRGameplayAbility_WeaponFire::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	
}

void UPRGameplayAbility_WeaponFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	// 쿨다운 커밋 후 발사 처리
	if (CommitAbilityCooldown(Handle,ActorInfo,ActivationInfo,false))
	{
		OnActivateAbility(Handle,ActorInfo,ActivationInfo,TriggerEventData);
	}
}

void UPRGameplayAbility_WeaponFire::OnActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
}

UWeaponInstance* UPRGameplayAbility_WeaponFire::GetWeaponInstance() const
{
	UPREquipmentManagerComponent* EquipmentManager = GetEquipmentManager();
	if (!IsValid(EquipmentManager))
	{
		return nullptr;
	}

	UEquipmentInstance* Instance = EquipmentManager->GetEquipmentInstance(WeaponSlotTag);
	return Cast<UWeaponInstance>(Instance);
}

UPREquipmentManagerComponent* UPRGameplayAbility_WeaponFire::GetEquipmentManager() const
{
	if (IPREquipmentInterface* EI = Cast<IPREquipmentInterface>(GetAvatarActorFromActorInfo()))
	{
		return EI->GetEquipmentManager();
	}
	return nullptr;
}

TArray<FHitResult> UPRGameplayAbility_WeaponFire::PerformHitscan(float Range) const
{
	FVector Start;
	FVector Direction;
	GetCameraAimInfo(Start, Direction);

	FVector End = Start + Direction * Range;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return {};
	}

	// 관통 0이면 단일 트레이스 사용
	int32 MaxPenetration = 0;
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		MaxPenetration = FMath::Max(0, FMath::TruncToInt(ASC->GetNumericAttribute(UPRWeaponAttributeSet::GetMaxPenetrationAttribute())));
	}

	if (MaxPenetration <= 0)
	{
		FHitResult SingleHit;
		if (World->LineTraceSingleByChannel(SingleHit, Start, End, TraceChannel, QueryParams))
		{
			return { SingleHit };
		}
		return {};
	}

	// 관통 횟수만큼 반복 트레이스
	TArray<FHitResult> HitResults;
	HitResults.Reserve(MaxPenetration + 1);

	FVector TraceStart = Start;
	const int32 MaxHits = MaxPenetration + 1;
	for (int32 HitIndex = 0; HitIndex < MaxHits; ++HitIndex)
	{
		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, TraceStart, End, TraceChannel, QueryParams))
		{
			break;
		}

		HitResults.Add(Hit);

		// 동일 액터 재히트 방지
		if (AActor* HitActor = Hit.GetActor())
		{
			QueryParams.AddIgnoredActor(HitActor);
		}

		// 다음 트레이스 시작점 오프셋
		TraceStart = Hit.ImpactPoint + Direction * 1.0f;
	}

	return HitResults;
}

TArray<FHitResult> UPRGameplayAbility_WeaponFire::PerformHitscanWithSpread(float SpreadAngle, float Range) const
{
	FVector Start;
	FVector Direction;
	GetCameraAimInfo(Start, Direction);

	// 스프레드 적용
	if (SpreadAngle > 0.0f)
	{
		float HalfAngleRad = FMath::DegreesToRadians(SpreadAngle * 0.5f);
		Direction = FMath::VRandCone(Direction, HalfAngleRad);
	}

	FVector End = Start + Direction * Range;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetAvatarActorFromActorInfo());
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = true;

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return {};
	}

	// 관통 0이면 단일 트레이스 사용
	int32 MaxPenetration = 0;
	if (const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		MaxPenetration = FMath::Max(0, FMath::TruncToInt(ASC->GetNumericAttribute(UPRWeaponAttributeSet::GetMaxPenetrationAttribute())));
	}

	if (MaxPenetration <= 0)
	{
		FHitResult SingleHit;
		if (World->LineTraceSingleByChannel(SingleHit, Start, End, TraceChannel, QueryParams))
		{
			return { SingleHit };
		}
		return {};
	}

	// 관통 횟수만큼 반복 트레이스
	TArray<FHitResult> HitResults;
	HitResults.Reserve(MaxPenetration + 1);

	FVector TraceStart = Start;
	const int32 MaxHits = MaxPenetration + 1;
	for (int32 HitIndex = 0; HitIndex < MaxHits; ++HitIndex)
	{
		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, TraceStart, End, TraceChannel, QueryParams))
		{
			break;
		}

		HitResults.Add(Hit);

		// 동일 액터 재히트 방지
		if (AActor* HitActor = Hit.GetActor())
		{
			QueryParams.AddIgnoredActor(HitActor);
		}

		// 다음 트레이스 시작점 오프셋
		TraceStart = Hit.ImpactPoint + Direction * 1.0f;
	}

	return HitResults;
}

void UPRGameplayAbility_WeaponFire::ApplyWeaponDamage(const FHitResult& HitResult, int32 PenetrationCount)
{
	if (!IsValid(DamageEffectClass))
	{
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	if (!IsValid(HitActor))
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
	if (!IsValid(TargetASC))
	{
		return;
	}

	FGameplayEffectSpecHandle SpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	if (SpecHandle.IsValid())
	{
		// 기본 데미지 * 배율 전달
		if (const UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo())
		{
			const float BaseDamage = SourceASC->GetNumericAttribute(UPRWeaponAttributeSet::GetBaseDamageAttribute());
			const float DamageMultiplier = SourceASC->GetNumericAttribute(UPRWeaponAttributeSet::GetDamageMultiplierAttribute());
			SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Combat_Damage, BaseDamage * DamageMultiplier);
		}

		// 관통 카운트 전달
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Combat_PenetrationCount, static_cast<float>(FMath::Max(0, PenetrationCount)));
		
		// 무기 태그 전달
		if (UWeaponInstance* WeaponInstance = GetWeaponInstance())
		{
			SpecHandle.Data->AppendDynamicAssetTags(WeaponInstance->GetGrantedTags());	
		}
		
		FGameplayAbilityTargetDataHandle TargetDataHandle;
		FGameplayAbilityTargetData_SingleTargetHit* TargetData = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
		TargetDataHandle.Add(TargetData);

		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, SpecHandle, TargetDataHandle);
	}
}

void UPRGameplayAbility_WeaponFire::GetCameraAimInfo(FVector& OutStart, FVector& OutDirection) const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		OutStart = FVector::ZeroVector;
		OutDirection = FVector::ForwardVector;
		return;
	}

	// PlayerController의 카메라 뷰 사용
	APawn* Pawn = Cast<APawn>(AvatarActor);
	if (IsValid(Pawn))
	{
		APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
		if (IsValid(PC))
		{
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(OutStart, ViewRotation);
			OutDirection = ViewRotation.Vector();
			return;
		}
	}

	// 폴백: 액터 위치 및 전방
	OutStart = AvatarActor->GetActorLocation();
	OutDirection = AvatarActor->GetActorForwardVector();
}

FVector UPRGameplayAbility_WeaponFire::GetAimPoint(float Distance) const
{
	FVector Start;
	FVector Direction;
	GetCameraAimInfo(Start, Direction);
	return Start + Direction * Distance;
}
