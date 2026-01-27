// PRMissileProjectile.cpp
#include "PRMissileProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"

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

	// 발사체 이동 컴포넌트
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 3000.0f;
	ProjectileMovement->MaxSpeed = 3000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;

	// 유도 기본값 (비활성)
	ProjectileMovement->bIsHomingProjectile = false;
	ProjectileMovement->HomingAccelerationMagnitude = 5000.0f;
}

void APRMissileProjectile::BeginPlay()
{
	Super::BeginPlay();

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
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->HomingTargetComponent = nullptr;
		return;
	}

	HomingTarget = Target;
	ProjectileMovement->bIsHomingProjectile = true;
	ProjectileMovement->HomingTargetComponent = Target->GetRootComponent();
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
