// Fill out your copyright notice in the Description page of Project Settings.


#include "PRCommonAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/Character/PRCharacterBase.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"
#include "Perception/AISense_Damage.h"
#include "ProjectReboot/PRGameplayTags.h"

void UPRCommonAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// Health 클램핑 (0 ~ MaxHealth)
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	// Shield 클램핑 (0 ~ MaxShield)
	else if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
	// Stagger 클램핑 (0 ~ HitImmunity)
	else if (Attribute == GetStaggerAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetHitImmunity());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetMaxShieldAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetHitImmunityAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
}

void UPRCommonAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if (Attribute == GetMoveSpeedAttribute())
	{
		UpdateCharacterWalkSpeed();
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		// MaxHealth보다 커지지 않도록 클램핑
		if (GetHealth() > NewValue)
		{
			SetHealth(NewValue);
		}
	}
	else if (Attribute == GetMaxShieldAttribute())
	{
		// MaxShield보다 커지지 않도록 클램핑
		if (GetShield() > NewValue)
		{
			SetShield(NewValue);
		}
	}
	else if (Attribute == GetHitImmunityAttribute())
	{
		// HitImmunity보다 커지지 않도록 클램핑
		if (GetStagger() > NewValue)
		{
			SetStagger(NewValue);
		}
	}
}

void UPRCommonAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	// IncomingDamage 처리 (메타 어트리뷰트)
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Data);
	}
	// Health 변경 처리
	else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		HandleHealthChanged(Data);
	}
	// Shield 클램핑
	else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	{
		const float ClampedShield = FMath::Clamp(GetShield(), 0.0f, GetMaxShield());
		SetShield(ClampedShield);
	}
	// Stagger 처리
	else if (Data.EvaluatedData.Attribute == GetStaggerAttribute())
	{
		const float ClampedStagger = FMath::Clamp(GetStagger(), 0.0f, GetHitImmunity());
		SetStagger(ClampedStagger);

		if (ClampedStagger >= GetHitImmunity() && GetHitImmunity() > 0.0f)
		{
			if (UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent())
			{
				FGameplayEventData EventData;
				EventData.EventTag = TAG_Event_Hit;
				EventData.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
				EventData.Target = GetOwningActor();
				EventData.ContextHandle = Data.EffectSpec.GetEffectContext();
				ASC->HandleGameplayEvent(TAG_Event_Hit, &EventData);
			}

			// 경직 수치 초기화
			SetStagger(0.0f);
		}
	}
}

void UPRCommonAttributeSet::HandleIncomingDamage(const FGameplayEffectModCallbackData& Data)
{
	const float IncomingDamageValue = GetIncomingDamage();
	SetIncomingDamage(0.0f); // 메타 어트리뷰트 초기화

	if (IncomingDamageValue <= 0.0f)
	{
		return;
	}

	// GE 태그에서 Dodgeable 여부 확인
	FGameplayTagContainer AssetTags;
	Data.EffectSpec.GetAllAssetTags(AssetTags);
	const bool bDodgeable = AssetTags.HasTag(TAG_Damage_Dodgeable);

	// 상태 체크
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	const bool bIsDodging = ASC && ASC->HasMatchingGameplayTag(TAG_State_Dodging);
	const bool bIsInvincible = ASC && ASC->HasMatchingGameplayTag(TAG_State_Invincible);

	// 회피 중 + Dodgeable 데미지 -> 이벤트 전송
	if (bIsDodging && bDodgeable)
	{
		SendDamageEvent(Data, IncomingDamageValue, true);
		return;
	}

	// 무적 상태 -> 데미지 무시 (이벤트 없음)
	if (bIsInvincible)
	{
		return;
	}

	// 일반 상태: Shield, Health 순서로 데미지 적용
	float RemainingDamage = IncomingDamageValue;

	const float CurrentShield = GetShield();
	if (CurrentShield > 0.0f)
	{
		const float ShieldDamage = FMath::Min(CurrentShield, RemainingDamage);
		SetShield(FMath::Max(0.0f, CurrentShield - ShieldDamage));
		RemainingDamage -= ShieldDamage;
	}

	if (RemainingDamage > 0.0f)
	{
		const float NewHealth = FMath::Max(0.0f, GetHealth() - RemainingDamage);
		SetHealth(NewHealth);

		// Stagger 증가 및 Hit 이벤트 체크
		const float NewStagger = FMath::Min(GetStagger() + RemainingDamage, GetHitImmunity());
		SetStagger(NewStagger);

		if (NewStagger >= GetHitImmunity() && GetHitImmunity() > 0.0f)
		{
			if (ASC)
			{
				FGameplayEventData HitEventData;
				HitEventData.EventTag = TAG_Event_Hit;
				HitEventData.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
				HitEventData.Target = GetOwningActor();
				HitEventData.ContextHandle = Data.EffectSpec.GetEffectContext();
				ASC->HandleGameplayEvent(TAG_Event_Hit, &HitEventData);
			}
			SetStagger(0.0f);
		}

		// 사망 체크
		if (NewHealth <= 0.0f)
		{
			HandleDeath(Data);
		}
	}

	// 데미지 이벤트 발송
	SendDamageEvent(Data, IncomingDamageValue, bDodgeable);

	// AI 감지용 이벤트
	ReportDamageEventIfNeeded(Data, IncomingDamageValue);
}

void UPRCommonAttributeSet::HandleHealthChanged(const FGameplayEffectModCallbackData& Data)
{
	// Health 클램핑 (힐링 GE 등 직접 Health 수정 시)
	const float ClampedHealth = FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth());
	SetHealth(ClampedHealth);

	// 사망 체크
	if (ClampedHealth <= 0.0f)
	{
		HandleDeath(Data);
	}
}

void UPRCommonAttributeSet::HandleDeath(const FGameplayEffectModCallbackData& Data)
{
	AActor* OwnerActor = GetOwningActor();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	// CombatInterface를 통한 사망 처리
	if (IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(OwnerActor))
	{
		// 이미 사망 처리 중인 경우 무시
		if (CombatInterface->IsDead())
		{
			return;
		}

		const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
		CombatInterface->Die(EffectContext);
	}
}

void UPRCommonAttributeSet::SendDamageEvent(const FGameplayEffectModCallbackData& Data, float DamageAmount, bool bDodgeable)
{
	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = TAG_Event_Damage;
	EventData.EventMagnitude = DamageAmount;
	EventData.Instigator = Data.EffectSpec.GetEffectContext().GetInstigator();
	EventData.Target = GetOwningActor();
	EventData.ContextHandle = Data.EffectSpec.GetEffectContext();

	// Dodgeable 태그를 InstigatorTags에 포함
	if (bDodgeable)
	{
		EventData.InstigatorTags.AddTag(TAG_Damage_Dodgeable);
	}

	ASC->HandleGameplayEvent(TAG_Event_Damage, &EventData);
}

void UPRCommonAttributeSet::ReportDamageEventIfNeeded(const FGameplayEffectModCallbackData& Data, float DamageAmount)
{
	AActor* OwnerActor = GetOwningActor();
	if (!IsValid(OwnerActor) || !OwnerActor->HasAuthority())
	{
		return;
	}

	if (DamageAmount <= 0.0f)
	{
		return;
	}

	const FGameplayEffectContextHandle& EffectContext = Data.EffectSpec.GetEffectContext();
	AActor* InstigatorActor = EffectContext.GetInstigator();
	if (!IsValid(InstigatorActor))
	{
		InstigatorActor = EffectContext.GetEffectCauser();
	}

	if (InstigatorActor == OwnerActor)
	{
		return;
	}

	FVector EventLocation = OwnerActor->GetActorLocation();
	if (IsValid(InstigatorActor))
	{
		EventLocation = InstigatorActor->GetActorLocation();
	}

	FVector HitLocation = OwnerActor->GetActorLocation();
	if (const FHitResult* HitResult = EffectContext.GetHitResult())
	{
		HitLocation = HitResult->Location;
	}

	UWorld* World = OwnerActor->GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	UAISense_Damage::ReportDamageEvent(World, OwnerActor, InstigatorActor, DamageAmount, EventLocation, HitLocation);
}

void UPRCommonAttributeSet::UpdateCharacterWalkSpeed()
{
	if (APRCharacterBase* Character = Cast<APRCharacterBase>(GetOwningActor()))
	{
		if (UCharacterMovementComponent* CMC = Character->GetCharacterMovement())
		{
			float MaxMoveSpeed = Character->GetBaseMoveSpeed() * GetMoveSpeed();
			CMC->MaxWalkSpeed = MaxMoveSpeed;
			CMC->MaxFlySpeed = MaxMoveSpeed;
			CMC->MaxSwimSpeed = MaxMoveSpeed;
		}
	}
}