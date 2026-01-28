// PRMissileProjectile.cpp
#include "PRMissileProjectile.h"
#include "PRProportionalNavigationComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
#include "ProjectReboot/Combat/PRCombatInterface.h"

APRMissileProjectile::APRMissileProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 충돌 컴포넌트
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(10.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	RootComponent = CollisionComponent;

	// 메시 컴포넌트 (선택적)
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 발사체 이동 컴포넌트 (비례항법 유도)
	ProjectileMovement = CreateDefaultSubobject<UPRProportionalNavigationComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// 비례항법 기본값 (비활성)
	ProjectileMovement->EnableProportionalNavigation(false);
}

void APRMissileProjectile::BeginPlay()
{
	Super::BeginPlay();

	// 발사 위치 저장 (사거리 계산용)
	LaunchLocation = GetActorLocation();

	// 발사자 및 소유자 충돌 무시
	if (IsValid(CollisionComponent))
	{
		if (AActor* OwnerActor = GetOwner())
		{
			CollisionComponent->IgnoreActorWhenMoving(OwnerActor, true);
		}
		if (AActor* InstigatorActor = GetInstigator())
		{
			CollisionComponent->IgnoreActorWhenMoving(InstigatorActor, true);
		}
	}

	// 충돌 이벤트 바인딩
	CollisionComponent->OnComponentHit.AddDynamic(this, &APRMissileProjectile::OnHit);
}

void APRMissileProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 최대 사거리 초과 체크
	CheckMaxRangeDetonation();

	// 유도 미사일일 경우 근접 폭발 체크
	if (HomingTarget.IsValid())
	{
		CheckProximityDetonation();
	}
}

void APRMissileProjectile::SetHomingTarget(AActor* Target)
{
	if (!IsValid(Target))
	{
		HomingTarget.Reset();
		ProjectileMovement->EnableProportionalNavigation(false);
		ProjectileMovement->SetNavigationTarget(nullptr);
		return;
	}

	HomingTarget = Target;

	// 비례항법 유도 활성화
	USceneComponent* TargetComponent = nullptr;

	// 기본 CombatCapsule 사용
	if (IPRCombatInterface* CombatInterface = Cast<IPRCombatInterface>(Target))
	{
		if (UCapsuleComponent* CombatCapsule = CombatInterface->GetCombatCapsuleComponent())
		{
			TargetComponent = CombatCapsule;
		}
	}

	// Fallback: RootComponent 사용
	if (!IsValid(TargetComponent))
	{
		TargetComponent = Target->GetRootComponent();
	}

	ProjectileMovement->SetNavigationTarget(TargetComponent);
	ProjectileMovement->EnableProportionalNavigation(true);
}

void APRMissileProjectile::SetExplosionRadius(float Radius)
{
	ExplosionRadius = Radius;
	// 근접 폭발 거리는 폭발 반경의 50%로 설정
	ProximityDetonationRadius = Radius * 0.5f;
}

void APRMissileProjectile::SetDamageEffectClass(TSubclassOf<UGameplayEffect> EffectClass)
{
	DamageEffectClass = EffectClass;
}

void APRMissileProjectile::SetInstigatorASC(UAbilitySystemComponent* ASC)
{
	InstigatorASC = ASC;
}

void APRMissileProjectile::LaunchInDirection(const FVector& Direction, float Speed)
{
	if (!IsValid(ProjectileMovement))
	{
		return;
	}

	FVector NormalizedDir = Direction.GetSafeNormal();
	if (NormalizedDir.IsNearlyZero())
	{
		NormalizedDir = GetActorForwardVector();
	}

	ProjectileMovement->InitialSpeed = Speed;
	ProjectileMovement->MaxSpeed = Speed;
	ProjectileMovement->Velocity = NormalizedDir * Speed;
}

void APRMissileProjectile::SetNavigationConstant(float N)
{
	if (IsValid(ProjectileMovement))
	{
		ProjectileMovement->NavigationConstant = FMath::Clamp(N, 1.0f, 10.0f);
	}
}

void APRMissileProjectile::SetMaxNavigationAcceleration(float MaxAcceleration)
{
	if (IsValid(ProjectileMovement))
	{
		ProjectileMovement->MaxNavigationAcceleration = FMath::Max(0.0f, MaxAcceleration);
	}
}

void APRMissileProjectile::SetMaxRange(float Range)
{
	MaxRange = Range;
}

void APRMissileProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 자기 자신이나 소유자와 충돌 무시
	if (OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}

	Explode();
}

void APRMissileProjectile::CheckProximityDetonation()
{
	if (bHasExploded || !HomingTarget.IsValid())
	{
		return;
	}

	float Distance = FVector::Dist(GetActorLocation(), HomingTarget->GetActorLocation());
	if (Distance <= ProximityDetonationRadius)
	{
		Explode();
	}
}

void APRMissileProjectile::CheckMaxRangeDetonation()
{
	if (bHasExploded || MaxRange <= 0.0f)
	{
		return;
	}

	float TraveledDistance = FVector::Dist(GetActorLocation(), LaunchLocation);
	if (TraveledDistance >= MaxRange)
	{
		Explode();
	}
}

void APRMissileProjectile::Explode()
{
	if (bHasExploded)
	{
		return;
	}
	bHasExploded = true;

	// AOE 데미지 적용
	ApplyAOEDamage();

	// 폭발 이펙트 스폰
	SpawnExplosionEffect();

	// 발사체 제거
	Destroy();
}

void APRMissileProjectile::SpawnExplosionEffect()
{
	if (!IsValid(ExplosionVFX))
	{
		return;
	}

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		ExplosionVFX,
		GetActorLocation(),
		FRotator::ZeroRotator,
		FVector(1.0f),
		true,
		true,
		ENCPoolMethod::AutoRelease
	);
}

void APRMissileProjectile::ApplyAOEDamage()
{
	if (!IsValid(DamageEffectClass) || !InstigatorASC.IsValid())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// 폭발 위치에서 범위 내 액터 탐색
	TArray<AActor*> OverlappedActors;
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	IgnoreActors.Add(GetOwner());
	IgnoreActors.Add(GetInstigator());

	UKismetSystemLibrary::SphereOverlapActors(
		World,
		GetActorLocation(),
		ExplosionRadius,
		TArray<TEnumAsByte<EObjectTypeQuery>>(), // 모든 오브젝트 타입
		AActor::StaticClass(),
		IgnoreActors,
		OverlappedActors
	);

	// 각 액터에게 데미지 적용
	for (AActor* TargetActor : OverlappedActors)
	{
		if (!IsValid(TargetActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
		if (!IsValid(TargetASC))
		{
			continue;
		}

		// GE Spec 생성
		FGameplayEffectContextHandle ContextHandle = InstigatorASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);
		ContextHandle.AddInstigator(GetInstigator(), this);

		FGameplayEffectSpecHandle SpecHandle = InstigatorASC->MakeOutgoingSpec(DamageEffectClass, 1, ContextHandle);
		if (!SpecHandle.IsValid())
		{
			continue;
		}

		// 데미지 값 설정 (Instigator의 무기 어트리뷰트에서 가져옴)
		float BaseDamage = InstigatorASC->GetNumericAttribute(UPRWeaponAttributeSet::GetBaseDamageAttribute());
		float DamageMultiplier = InstigatorASC->GetNumericAttribute(UPRWeaponAttributeSet::GetDamageMultiplierAttribute());
		SpecHandle.Data->SetSetByCallerMagnitude(TAG_SetByCaller_Combat_Damage, BaseDamage * DamageMultiplier);

		// 폭발 데미지 타입 태그 추가
		SpecHandle.Data->DynamicAssetTags.AddTag(TAG_DamageType_Explosion);

		// 데미지 적용
		InstigatorASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}
