// PRGA_Reload.cpp
#include "PRGA_Reload.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
#include "ProjectReboot/Animation/PRAnimRegistryInterface.h"
#include "ProjectReboot/Equipment/PREquipmentBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/Equipment/Weapon/WeaponInstance.h"

UPRGA_Reload::UPRGA_Reload()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationPolicy = EPRAbilityActivationPolicy::OnInputTriggered;
	
	AbilityTags.AddTag(TAG_Ability_Reload);
	ActivationBlockedTags.AddTag(TAG_State_Dead);
	ActivationOwnedTags.AddTag(TAG_State_Weapon_CannotFire);
	
	WeaponSlotTag = TAG_Equipment_Slot_Weapon_Primary;
}

bool UPRGA_Reload::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const UWeaponInstance* Weapon = GetWeaponInstance();
	if (!IsValid(Weapon))
	{
		return false;
	}

	if (Weapon->IsReloading())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		return false;
	}

	return Weapon->CanReload();
}

void UPRGA_Reload::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UWeaponInstance* Weapon = GetWeaponInstance();
	if (!IsValid(Weapon))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	Weapon->StartReload();

	if (UAnimMontage* ReloadMontage = GetReloadMontage())
	{
		PlayReloadMontage(ReloadMontage);
		return;
	}

	const float ReloadTime = GetReloadTime();
	if (ReloadTime > 0.0f)
	{
		StartReloadDelay(ReloadTime);
		return;
	}

	FinishReload();
	K2_EndAbility();
}

void UPRGA_Reload::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}

	if (ReloadDelayTask)
	{
		ReloadDelayTask->EndTask();
		ReloadDelayTask = nullptr;
	}

	if (bWasCancelled)
	{
		CancelReload();
	}
}

void UPRGA_Reload::PlayReloadMontage(UAnimMontage* MontageToPlay)
{
	if (!IsValid(MontageToPlay))
	{
		return;
	}

	float PlayRate = MontagePlayRate;
	if (bMatchReloadTime)
	{
		const float MinReloadTime = FMath::Max(0.1f, MinReloadTimeWhenMatch);
		const float ReloadTime = FMath::Max(GetReloadTime(), MinReloadTime);
		const float MontageLength = MontageToPlay->GetPlayLength();
		if (ReloadTime > KINDA_SMALL_NUMBER && MontageLength > KINDA_SMALL_NUMBER)
		{
			PlayRate = MontageLength / ReloadTime;
		}
	}

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageToPlay,
		PlayRate,
		NAME_None,
		false
	);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageBlendOut);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);

	MontageTask->ReadyForActivation();
}

UAnimMontage* UPRGA_Reload::GetReloadMontage() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return nullptr;
	}

	if (IsValid(ReloadMontageOverride))
	{
		return ReloadMontageOverride;
	}

	IPRAnimRegistryInterface* ARI = Cast<IPRAnimRegistryInterface>(AvatarActor);
	if (!ARI)
	{
		return nullptr;
	}

	const FGameplayTag ReloadTag = GetReloadMontageTagByWeaponType();
	if (UAnimMontage* ReloadMontage = ARI->FindMontageByGameplayTag(ReloadTag))
	{
		return ReloadMontage;
	}

	return ARI->FindMontageByGameplayTag(TAG_Montage_Reload);
}

FGameplayTag UPRGA_Reload::GetReloadMontageTagByWeaponType() const
{
	if (const UWeaponInstance* WeaponInstance = GetWeaponInstance())
	{
		if (WeaponInstance->HasTag(TAG_Equipment_Weapon_Type_Bullet))
		{
			return TAG_Montage_Reload_Bullet;
		}
		if (WeaponInstance->HasTag(TAG_Equipment_Weapon_Type_Beam))
		{
			return TAG_Montage_Reload_Beam;
		}
		if (WeaponInstance->HasTag(TAG_Equipment_Weapon_Type_Missile))
		{
			return TAG_Montage_Reload_Missile;
		}
	}

	return TAG_Montage_Reload;
}

float UPRGA_Reload::GetReloadTime() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!IsValid(ASC))
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, ASC->GetNumericAttribute(UPRWeaponAttributeSet::GetReloadTimeAttribute()));
}

void UPRGA_Reload::StartReloadDelay(float Delay)
{
	ReloadDelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Delay);
	ReloadDelayTask->OnFinish.AddDynamic(this, &ThisClass::OnReloadDelayFinished);
	ReloadDelayTask->ReadyForActivation();
}

void UPRGA_Reload::FinishReload()
{
	if (UWeaponInstance* Weapon = GetWeaponInstance())
	{
		Weapon->FinishReload();
	}
}

void UPRGA_Reload::CancelReload()
{
	if (UWeaponInstance* Weapon = GetWeaponInstance())
	{
		Weapon->CancelReload();
	}
}

void UPRGA_Reload::OnMontageCompleted()
{
	FinishReload();
	K2_EndAbility();
}

void UPRGA_Reload::OnMontageBlendOut()
{
	FinishReload();
	K2_EndAbility();
}

void UPRGA_Reload::OnMontageCancelled()
{
	CancelReload();
	K2_EndAbility();
}

void UPRGA_Reload::OnReloadDelayFinished()
{
	FinishReload();
	K2_EndAbility();
}

UWeaponInstance* UPRGA_Reload::GetWeaponInstance() const
{
	AActor* AvatarActor = GetAvatarActorFromActorInfo();
	if (!IsValid(AvatarActor))
	{
		return nullptr;
	}

	UPREquipmentManagerComponent* EquipmentManager = UPREquipmentBlueprintLibrary::GetEquipmentManager(AvatarActor);
	if (!IsValid(EquipmentManager))
	{
		return nullptr;
	}

	return Cast<UWeaponInstance>(EquipmentManager->GetEquipmentInstance(WeaponSlotTag));
}

