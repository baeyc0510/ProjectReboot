// Fill out your copyright notice in the Description page of Project Settings.

#include "PRPlasmaWave.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NiagaraSystem.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"

APRPlasmaWave::APRPlasmaWave()
{
	PrimaryActorTick.bCanEverTick = true;

	// 씬 루트
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 데미지 판정 영역 (박스)
	DamageBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageBox"));
	DamageBox->SetupAttachment(RootComponent);
	DamageBox->SetBoxExtent(DamageBoxExtent);
	DamageBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageBox->SetGenerateOverlapEvents(true);

	// 웨이브 이펙트
	WaveEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WaveEffect"));
	WaveEffect->SetupAttachment(RootComponent);
	WaveEffect->bAutoActivate = false;
}

void APRPlasmaWave::BeginPlay()
{
	Super::BeginPlay();

	// Overlap 이벤트 바인딩
	if (IsValid(DamageBox))
	{
		DamageBox->OnComponentBeginOverlap.AddDynamic(this, &APRPlasmaWave::OnWaveOverlap);
	}

	// 발사자 충돌 무시
	if (AActor* OwnerActor = GetOwner())
	{
		if (IsValid(DamageBox))
		{
			DamageBox->IgnoreActorWhenMoving(OwnerActor, true);
		}
	}
	if (AActor* InstigatorActor = GetInstigator())
	{
		if (IsValid(DamageBox))
		{
			DamageBox->IgnoreActorWhenMoving(InstigatorActor, true);
		}
	}

	// 시작 위치를 지면에 스냅
	TraceGround(0.f);
}

void APRPlasmaWave::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 이동
	const float MoveDelta = MoveSpeed * DeltaTime;
	const FVector NewLocation = GetActorLocation() + MoveDirection * MoveDelta;
	SetActorLocation(NewLocation);

	TraveledDistance += MoveDelta;

	// 지면 추적
	TraceGround(DeltaTime);

	// 최대 거리 도달 시 소멸
	if (TraveledDistance >= MaxDistance)
	{
		Destroy();
	}
}

void APRPlasmaWave::InitWave(const FVector& Direction, const FGameplayEffectSpecHandle& InDamageSpec)
{
	// 수평 방향만 사용
	MoveDirection = FVector(Direction.X, Direction.Y, 0.f).GetSafeNormal();
	if (MoveDirection.IsNearlyZero())
	{
		MoveDirection = GetActorForwardVector();
	}

	DamageSpec = InDamageSpec;

	// 이동 방향으로 회전
	SetActorRotation(MoveDirection.Rotation());

	// 데미지 박스 크기 적용
	if (IsValid(DamageBox))
	{
		DamageBox->SetBoxExtent(DamageBoxExtent);
	}

	// Niagara 에셋 설정 및 활성화
	if (IsValid(WaveNiagaraSystem) && IsValid(WaveEffect))
	{
		WaveEffect->SetAsset(WaveNiagaraSystem);
	}

	if (IsValid(WaveEffect))
	{
		WaveEffect->Activate();
	}
}

void APRPlasmaWave::OnWaveOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor))
	{
		return;
	}

	// 자신 및 소유자 무시
	if (OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	// 중복 히트 방지
	if (HitActors.Contains(OtherActor))
	{
		return;
	}
	HitActors.Add(OtherActor);

	// CombatInterface 피격 알림 (Ghost 감지 등)
	if (IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(OtherActor))
	{
		CombatInterface->OnHit(SweepResult);
	}

	// 데미지 적용
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor);
	if (!IsValid(TargetASC))
	{
		return;
	}

	if (DamageSpec.IsValid())
	{
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
	}
}

void APRPlasmaWave::TraceGround(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TraceStart = FVector(CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z + GroundTraceHeight);
	const FVector TraceEnd = FVector(CurrentLocation.X, CurrentLocation.Y, CurrentLocation.Z - GroundTraceHeight);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());

	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		float BottomOffsetZ = DamageBoxExtent.Z;
		if (IsValid(DamageBox))
		{
			BottomOffsetZ = DamageBox->GetScaledBoxExtent().Z;
		}

		FVector SnappedLocation = CurrentLocation;
		SnappedLocation.Z = HitResult.Location.Z + BottomOffsetZ;
		SetActorLocation(SnappedLocation);
	}
}
