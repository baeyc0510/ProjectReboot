// Fill out your copyright notice in the Description page of Project Settings.

#include "PROrbitalStrike.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"

APROrbitalStrike::APROrbitalStrike()
{
	PrimaryActorTick.bCanEverTick = false;

	// 씬 루트
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 데미지 판정 영역
	DamageArea = CreateDefaultSubobject<USphereComponent>(TEXT("DamageArea"));
	DamageArea->SetupAttachment(RootComponent);
	DamageArea->SetSphereRadius(300.f);
	DamageArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DamageArea->SetCollisionResponseToAllChannels(ECR_Overlap);

	// Niagara 컴포넌트 (에셋은 BP에서 설정)
	TelegraphEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TelegraphEffect"));
	TelegraphEffect->SetupAttachment(RootComponent);
	TelegraphEffect->bAutoActivate = false;

	ImpactEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ImpactEffect"));
	ImpactEffect->SetupAttachment(RootComponent);
	ImpactEffect->bAutoActivate = false;
}

void APROrbitalStrike::InitStrike(const FGameplayEffectSpecHandle& InDamageSpec, FLinearColor Color, float TelegraphDuration, float Radius)
{
	DamageSpec = InDamageSpec;
	CachedRadius = Radius;

	// Damage Area 크기 설정
	if (IsValid(DamageArea))
	{
		DamageArea->SetSphereRadius(Radius);
		DamageArea->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}

	// Phase 1 시작
	StartTelegraphPhase(Color, TelegraphDuration, Radius);

	// TelegraphDuration 후 Phase 2로 전환
	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&APROrbitalStrike::StartImpactPhase,
		TelegraphDuration,
		false
	);
}

void APROrbitalStrike::StartTelegraphPhase(FLinearColor Color, float TelegraphDuration, float Radius)
{
	if (IsValid(TelegraphEffect))
	{
		TelegraphEffect->SetVariableLinearColor(TEXT("Color"), Color);
		TelegraphEffect->SetVariableFloat(TEXT("LifeTime"), TelegraphDuration);
		TelegraphEffect->SetVariableFloat(TEXT("Scale"), Radius * 2.0f);
		TelegraphEffect->Activate();
	}
}

void APROrbitalStrike::StartImpactPhase()
{
	// Telegraph 종료
	if (IsValid(TelegraphEffect))
	{
		TelegraphEffect->Deactivate();
	}

	// 낙하 이펙트 시작
	if (IsValid(ImpactEffect))
	{
		ImpactEffect->Activate();
	}

	// ImpactDelay 후 데미지 판정
	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&APROrbitalStrike::ExecuteDamage,
		ImpactDelay,
		false
	);
}

void APROrbitalStrike::ExecuteDamage()
{
	// Collision 활성화하여 Overlap 체크
	if (IsValid(DamageArea))
	{
		DamageArea->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	}

	// Overlap된 액터들에게 데미지 적용
	TArray<AActor*> OverlappingActors;
	if (IsValid(DamageArea))
	{
		DamageArea->GetOverlappingActors(OverlappingActors);
	}

	for (AActor* Actor : OverlappingActors)
	{
		if (!IsValid(Actor))
		{
			continue;
		}

		// CombatInterface 피격 알림 (Ghost 감지 등)
		if (IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(Actor))
		{
			FHitResult HitResult;
			HitResult.ImpactPoint = Actor->GetActorLocation();
			HitResult.Location = GetActorLocation();
			CombatInterface->OnHit(HitResult);
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
		if (!IsValid(TargetASC))
		{
			continue;
		}

		if (DamageSpec.IsValid())
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
		}
	}

	// Collision 비활성화
	if (IsValid(DamageArea))
	{
		DamageArea->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}

	// 이펙트 완료 대기 후 Destroy
	SetLifeSpan(2.0f);
}
