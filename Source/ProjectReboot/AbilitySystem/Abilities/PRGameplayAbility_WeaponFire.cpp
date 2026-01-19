// PRGameplayAbility_WeaponFire.cpp
#include "PRGameplayAbility_WeaponFire.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "GameFramework/PlayerController.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/Equipment/Weapon/WeaponInstance.h"

UPRGameplayAbility_WeaponFire::UPRGameplayAbility_WeaponFire()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EPRAbilityActivationPolicy::OnInputTriggered;
}

void UPRGameplayAbility_WeaponFire::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
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
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return nullptr;
	}

	return AvatarActor->FindComponentByClass<UPREquipmentManagerComponent>();
}

bool UPRGameplayAbility_WeaponFire::PerformHitscan(FHitResult& OutHitResult, float Range) const
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
		return false;
	}

	return World->LineTraceSingleByChannel(OutHitResult, Start, End, TraceChannel, QueryParams);
}

bool UPRGameplayAbility_WeaponFire::PerformHitscanWithSpread(FHitResult& OutHitResult, float SpreadAngle, float Range) const
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
		return false;
	}

	return World->LineTraceSingleByChannel(OutHitResult, Start, End, TraceChannel, QueryParams);
}

void UPRGameplayAbility_WeaponFire::ApplyWeaponDamage(const FHitResult& HitResult)
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
