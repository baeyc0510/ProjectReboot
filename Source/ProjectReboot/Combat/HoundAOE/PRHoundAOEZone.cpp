// PRHoundAOEZone.cpp

#include "PRHoundAOEZone.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"

APRHoundAOEZone::APRHoundAOEZone()
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
	DamageArea->SetGenerateOverlapEvents(true);

	// 경고 이펙트 (에셋은 BP에서 설정)
	TelegraphEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TelegraphEffect"));
	TelegraphEffect->SetupAttachment(RootComponent);
	TelegraphEffect->bAutoActivate = false;

	// 낙하 이펙트 (에셋은 BP에서 설정)
	ImpactEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ImpactEffect"));
	ImpactEffect->SetupAttachment(RootComponent);
	ImpactEffect->bAutoActivate = false;

	// 데미지 영역 이펙트 (에셋은 BP에서 설정)
	DamageAreaEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DamageAreaEffect"));
	DamageAreaEffect->SetupAttachment(RootComponent);
	DamageAreaEffect->bAutoActivate = false;

}

void APRHoundAOEZone::InitZone(const FGameplayEffectSpecHandle& InDamageSpec, FLinearColor Color,
	float TelegraphDuration, float Radius, float DamageDuration, TSubclassOf<AActor> InTargetActorClass)
{
	DamageSpec = InDamageSpec;
	CachedRadius = Radius;
	CachedDamageDuration = DamageDuration;
	TargetActorClass = InTargetActorClass;

	// 데미지 영역 크기 설정
	if (IsValid(DamageArea))
	{
		DamageArea->SetSphereRadius(Radius);
		DamageArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 오버랩 이벤트 바인딩
	DamageArea->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnDamageAreaBeginOverlap);
	DamageArea->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnDamageAreaEndOverlap);

	// 이펙트 초기화
	if (IsValid(TelegraphEffect))
	{
		TelegraphEffect->Deactivate();
	}
	if (IsValid(ImpactEffect))
	{
		ImpactEffect->Deactivate();
	}
	if (IsValid(DamageAreaEffect))
	{
		DamageAreaEffect->Deactivate();
	}
	
	// Phase 1 시작
	StartTelegraphPhase(Color, TelegraphDuration, Radius);

	// TelegraphDuration 후 Phase 2로 전환
	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&APRHoundAOEZone::StartImpactPhase,
		TelegraphDuration,
		false
	);
}

void APRHoundAOEZone::StartTelegraphPhase(FLinearColor Color, float TelegraphDuration, float Radius)
{
	if (IsValid(TelegraphEffect))
	{
		TelegraphEffect->SetVariableLinearColor(TEXT("Color"), Color);
		TelegraphEffect->SetVariableFloat(TEXT("LifeTime"), TelegraphDuration);
		TelegraphEffect->SetVariableFloat(TEXT("Scale"), Radius * 2.0f);
		TelegraphEffect->Activate();
	}
}

void APRHoundAOEZone::StartImpactPhase()
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

	// ImpactDelay 후 데미지 영역 활성화
	GetWorld()->GetTimerManager().SetTimer(
		PhaseTimerHandle,
		this,
		&APRHoundAOEZone::StartDamageZonePhase,
		ImpactDelay,
		false
	);
}

void APRHoundAOEZone::StartDamageZonePhase()
{
	// 콜리전 활성화
	if (IsValid(DamageArea))
	{
		DamageArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	// 데미지 영역 이펙트 시작
	if (IsValid(DamageAreaEffect))
	{
		DamageAreaEffect->Activate();
	}

	// 이미 영역 안에 있는 액터에 GE 적용
	TArray<AActor*> OverlappingActors;
	if (IsValid(DamageArea))
	{
		DamageArea->GetOverlappingActors(OverlappingActors);
	}

	for (AActor* Actor : OverlappingActors)
	{
		if (IsValid(Actor) && Actor != GetOwner() && Actor != GetInstigator())
		{
			ApplyDamageEffectToActor(Actor);
		}
	}

	// DamageDuration 후 정리
	GetWorld()->GetTimerManager().SetTimer(
		DamageZoneTimerHandle,
		this,
		&APRHoundAOEZone::CleanupZone,
		CachedDamageDuration,
		false
	);
}

void APRHoundAOEZone::CleanupZone()
{
	// 콜리전 비활성화
	if (IsValid(DamageArea))
	{
		DamageArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 데미지 영역 이펙트 종료
	if (IsValid(DamageAreaEffect))
	{
		DamageAreaEffect->Deactivate();
	}

	// 모든 적용된 GE 제거
	RemoveAllAppliedEffects();

	// VFX 종료
	if (IsValid(ImpactEffect))
	{
		ImpactEffect->Deactivate();
	}

	// 잔여 VFX 재생 후 파괴
	SetLifeSpan(CleanupLifeSpan);
}

void APRHoundAOEZone::OnDamageAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	ApplyDamageEffectToActor(OtherActor);
}

void APRHoundAOEZone::OnDamageAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	RemoveDamageEffectFromActor(OtherActor);
}

void APRHoundAOEZone::ApplyDamageEffectToActor(AActor* TargetActor)
{
	if (!IsValidTargetActor(TargetActor) || !DamageSpec.IsValid())
	{
		return;
	}

	// 이미 적용된 경우 스킵
	if (AppliedEffectHandles.Contains(TargetActor))
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (!IsValid(TargetASC))
	{
		return;
	}

	// CombatInterface 피격 알림
	if (IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(TargetActor))
	{
		FHitResult HitResult;
		HitResult.ImpactPoint = TargetActor->GetActorLocation();
		HitResult.Location = GetActorLocation();
		CombatInterface->OnHit(HitResult);
	}

	// Duration GE 적용 및 핸들 캐시
	FActiveGameplayEffectHandle EffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
	if (EffectHandle.IsValid())
	{
		AppliedEffectHandles.Add(TargetActor, EffectHandle);
	}
}

bool APRHoundAOEZone::IsValidTargetActor(AActor* TargetActor) const
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	if (IsValid(TargetActorClass) && !TargetActor->IsA(TargetActorClass))
	{
		return false;
	}

	return true;
}

void APRHoundAOEZone::RemoveDamageEffectFromActor(AActor* TargetActor)
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	FActiveGameplayEffectHandle* HandlePtr = AppliedEffectHandles.Find(TargetActor);
	if (!HandlePtr)
	{
		return;
	}

	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
	if (IsValid(TargetASC))
	{
		TargetASC->RemoveActiveGameplayEffect(*HandlePtr);
	}

	AppliedEffectHandles.Remove(TargetActor);
}

void APRHoundAOEZone::RemoveAllAppliedEffects()
{
	for (auto& Pair : AppliedEffectHandles)
	{
		if (!Pair.Key.IsValid())
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Pair.Key.Get());
		if (IsValid(TargetASC))
		{
			TargetASC->RemoveActiveGameplayEffect(Pair.Value);
		}
	}

	AppliedEffectHandles.Empty();
}
