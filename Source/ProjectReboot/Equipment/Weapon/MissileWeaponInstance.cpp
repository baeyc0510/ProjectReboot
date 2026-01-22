// MissileWeaponInstance.cpp
#include "MissileWeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"

bool UMissileWeaponInstance::CanFire() const
{
	if (GetLoadedMissiles() <= 0)
	{
		return false;
	}

	return Super::CanFire();
}

void UMissileWeaponInstance::OnFired()
{
	Super::OnFired();
	ConsumeMissile();

	// 다음 발사 인덱스로 이동
	const TArray<FName>& MuzzleSockets = GetActiveMuzzleSockets();
	if (MuzzleSockets.Num() > 0)
	{
		CurrentFireIndex = (CurrentFireIndex + 1) % MuzzleSockets.Num();
	}

	// 발사 후 메시 업데이트
	UpdateMissileVisuals();
}

void UMissileWeaponInstance::OnEquipmentTagsChanged()
{
	Super::OnEquipmentTagsChanged();

	// 기존 탄환 메시 제거 후 새로 스폰
	DestroyAmmoMeshComponents();
	SpawnAmmoMeshComponents();

	// 발사 인덱스 리셋
	CurrentFireIndex = 0;

	// 미사일 메시 가시성 업데이트
	UpdateMissileVisuals();
}

int32 UMissileWeaponInstance::GetLoadedMissiles() const
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (IsValid(ASC))
	{
		bool bFound = false;
		float Missiles = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFound);
		if (bFound)
		{
			return FMath::FloorToInt(Missiles);
		}
	}

	return 0;
}

void UMissileWeaponInstance::ConsumeMissile()
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (!IsValid(ASC))
	{
		return;
	}

	bool bFound = false;
	float CurrentMissiles = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetAmmoAttribute(), bFound);
	if (bFound)
	{
		float NewMissiles = FMath::Max(0.0f, CurrentMissiles - 1.0f);
		ASC->SetNumericAttributeBase(UPRWeaponAttributeSet::GetAmmoAttribute(), NewMissiles);
	}
}

float UMissileWeaponInstance::GetExplosionRadius() const
{
	UAbilitySystemComponent* ASC = GetOwnerASC();
	if (IsValid(ASC))
	{
		bool bFound = false;
		float Radius = ASC->GetGameplayAttributeValue(UPRWeaponAttributeSet::GetEffectRadiusAttribute(), bFound);
		if (bFound)
		{
			return Radius;
		}
	}

	return 300.0f;
}

FTransform UMissileWeaponInstance::GetMuzzleTransform() const
{
	const TArray<FName>& MuzzleSockets = GetActiveMuzzleSockets();
	if (MuzzleSockets.Num() == 0)
	{
		return Super::GetMuzzleTransform();
	}

	USceneComponent* PrimaryComp = GetPrimaryComponent();
	if (!IsValid(PrimaryComp))
	{
		return FTransform::Identity;
	}

	// 현재 인덱스 범위 체크 (순차 발사)
	int32 SafeIndex = CurrentFireIndex % MuzzleSockets.Num();
	FName SocketName = MuzzleSockets[SafeIndex];

	if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(PrimaryComp))
	{
		if (SkelMesh->DoesSocketExist(SocketName))
		{
			return SkelMesh->GetSocketTransform(SocketName);
		}
	}
	else if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(PrimaryComp))
	{
		if (StaticMesh->DoesSocketExist(SocketName))
		{
			return StaticMesh->GetSocketTransform(SocketName);
		}
	}

	return PrimaryComp->GetComponentTransform();
}

void UMissileWeaponInstance::SpawnAmmoMeshComponents()
{
	USceneComponent* PrimaryComp = GetPrimaryComponent();
	if (!IsValid(PrimaryComp))
	{
		return;
	}

	// EquipmentTags에서 매칭되는 설정 찾기
	const FMissileAmmoMeshConfig* MatchedConfig = nullptr;
	for (const FMissileAmmoMeshConfig& Config : AmmoMeshConfigs)
	{
		if (EquipmentTags.HasAll(Config.RequiredTags))
		{
			MatchedConfig = &Config;
			break;
		}
	}

	if (!MatchedConfig || !IsValid(MatchedConfig->AmmoMesh))
	{
		return;
	}

	AActor* OwnerActor = PrimaryComp->GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	// 각 소켓에 탄환 메시 컴포넌트 생성
	for (const FName& SocketName : MatchedConfig->AmmoMeshSocketNames)
	{
		if (SocketName.IsNone())
		{
			continue;
		}

		USkeletalMeshComponent* MeshComp = NewObject<USkeletalMeshComponent>(OwnerActor);
		if (IsValid(MeshComp))
		{
			MeshComp->SetSkeletalMesh(MatchedConfig->AmmoMesh);
			MeshComp->AttachToComponent(PrimaryComp, FAttachmentTransformRules::SnapToTargetIncludingScale, SocketName);
			MeshComp->RegisterComponent();
			AmmoMeshComponents.Add(MeshComp);
		}
	}
}

void UMissileWeaponInstance::DestroyAmmoMeshComponents()
{
	for (USkeletalMeshComponent* MeshComp : AmmoMeshComponents)
	{
		if (IsValid(MeshComp))
		{
			MeshComp->DestroyComponent();
		}
	}
	AmmoMeshComponents.Empty();
}

void UMissileWeaponInstance::UpdateMissileVisuals()
{
	int32 LoadedCount = GetLoadedMissiles();

	for (int32 i = 0; i < AmmoMeshComponents.Num(); ++i)
	{
		USkeletalMeshComponent* MeshComp = AmmoMeshComponents[i];
		if (IsValid(MeshComp))
		{
			bool bShouldBeVisible = (i < LoadedCount);
			MeshComp->SetVisibility(bShouldBeVisible);
		}
	}
}
