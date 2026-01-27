// WeaponInstance.cpp
#include "WeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

bool UWeaponInstance::CanFire() const
{
	return true;
}

void UWeaponInstance::OnFired()
{
}

bool UWeaponInstance::CanReload() const
{
	return false;
}

bool UWeaponInstance::IsReloading() const
{
	return false;
}

void UWeaponInstance::StartReload()
{
}

void UWeaponInstance::FinishReload()
{
}

void UWeaponInstance::CancelReload()
{
}

FTransform UWeaponInstance::GetMuzzleTransform() const
{
	// 사용할 소켓 이름 결정
	FName SocketName = (ActiveMuzzleSockets.Num() > 0) ? ActiveMuzzleSockets[0] : DefaultMuzzleSocketName;

	USceneComponent* PrimaryComp = GetPrimaryComponent();
	if (IsValid(PrimaryComp))
	{
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

		// 소켓이 없으면 컴포넌트 트랜스폼 반환
		return PrimaryComp->GetComponentTransform();
	}

	return FTransform::Identity;
}

void UWeaponInstance::OnEquipmentTagsChanged()
{
	Super::OnEquipmentTagsChanged();
	UpdateMuzzleSlotConfig();
}

void UWeaponInstance::UpdateMuzzleSlotConfig()
{
	ActiveMuzzleSockets.Empty();

	// EquipmentTags에서 매칭되는 설정 찾기 (모든 RequiredTags를 포함해야 매칭)
	for (const FWeaponMuzzleSlotConfig& Config : MuzzleSlotConfigs)
	{
		if (EquipmentTags.HasAll(Config.RequiredTags))
		{
			ActiveMuzzleSockets = Config.MuzzleSocketNames;
			return;
		}
	}

	// 매칭되는 설정이 없으면 기본 소켓 사용
	ActiveMuzzleSockets.Add(DefaultMuzzleSocketName);
}

void UWeaponInstance::PlayMuzzleFlash()
{
	if (!IsValid(VFXSettings.MuzzleFlashVFX))
	{
		return;
	}

	FTransform MuzzleTransform = GetMuzzleTransform();
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		VFXSettings.MuzzleFlashVFX,
		MuzzleTransform.GetLocation(),
		MuzzleTransform.GetRotation().Rotator()
	);

	if (IsValid(VFXSettings.FireSound))
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			VFXSettings.FireSound,
			MuzzleTransform.GetLocation()
		);
	}
}

void UWeaponInstance::PlayImpact(const FHitResult& HitResult)
{
	if (!IsValid(VFXSettings.DefaultImpactVFX))
	{
		return;
	}

	const FVector ImpactNormalVector = FVector(HitResult.ImpactNormal);
	const FVector ImpactNormal = VFXSettings.bFlipImpactNormal
		? -ImpactNormalVector
		: ImpactNormalVector;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		VFXSettings.DefaultImpactVFX,
		HitResult.ImpactPoint,
		ImpactNormal.Rotation()
	);

	if (IsValid(VFXSettings.ImpactSound))
	{
		UGameplayStatics::PlaySoundAtLocation(	
			GetWorld(),
			VFXSettings.ImpactSound,
			HitResult.ImpactPoint
		);
	}
}

UAbilitySystemComponent* UWeaponInstance::GetOwnerASC() const
{
	if (!AttachTarget.IsValid())
	{
		return nullptr;
	}

	AActor* Owner = AttachTarget->GetOwner();
	if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner))
	{
		return ASI->GetAbilitySystemComponent();
	}

	return nullptr;
}
