// WeaponInstance.cpp
#include "WeaponInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "NiagaraComponent.h"
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
	FName SocketName = (ActiveMuzzles.Num() > 0) ? ActiveMuzzles[0].SocketName : DefaultMuzzleSocketName;

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
	// 기존 컴포넌트 정리
	for (FActiveMuzzleInfo& Muzzle : ActiveMuzzles)
	{
		if (IsValid(Muzzle.MuzzleFlashComp))
		{
			Muzzle.MuzzleFlashComp->DestroyComponent();
		}
	}
	ActiveMuzzles.Empty();

	// 설정 매칭
	TArray<FName> SocketNames;
	for (const FWeaponMuzzleSlotConfig& Config : MuzzleSlotConfigs)
	{
		if (EquipmentTags.HasAll(Config.RequiredTags))
		{
			SocketNames = Config.MuzzleSocketNames;
			break;
		}
	}
	if (SocketNames.IsEmpty())
	{
		SocketNames.Add(DefaultMuzzleSocketName);
	}

	// 새 머즐 컴포넌트 생성 & 부착
	USceneComponent* AttachComp = GetPrimaryComponent();
	for (const FName& SocketName : SocketNames)
	{
		FActiveMuzzleInfo NewMuzzle;
		NewMuzzle.SocketName = SocketName;

		if (IsValid(FXSettings.MuzzleFlashVFX) && IsValid(AttachComp))
		{
			NewMuzzle.MuzzleFlashComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
				FXSettings.MuzzleFlashVFX,
				AttachComp,
				SocketName,
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				EAttachLocation::SnapToTarget,
				false
			);
			if (IsValid(NewMuzzle.MuzzleFlashComp))
			{
				NewMuzzle.MuzzleFlashComp->SetRelativeScale3D(FXSettings.MuzzleFlashScale);
			}
			NewMuzzle.MuzzleFlashComp->Deactivate();
		}
		ActiveMuzzles.Add(NewMuzzle);
	}
}

void UWeaponInstance::OnEquipped()
{
	Super::OnEquipped();
	WarmupVFX();
}

void UWeaponInstance::WarmupVFX()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	// 화면 밖에서 아주 작은 스케일로 VFX를 생성해 셰이더 컴파일 히칭을 완화
	const FVector WarmupLocation(1000000.0f, 1000000.0f, -1000000.0f);
	const FVector WarmupScale(0.01f, 0.01f, 0.01f);

	TArray<TObjectPtr<UNiagaraSystem>> WarmupSystems;
	WarmupSystems.Reserve(4);
	WarmupSystems.Add(FXSettings.MuzzleFlashVFX);
	WarmupSystems.Add(FXSettings.TrailVFX);
	WarmupSystems.Add(FXSettings.DefaultImpactVFX);
	WarmupSystems.Add(FXSettings.ExplodeVFX);

	for (const TObjectPtr<UNiagaraSystem>& System : WarmupSystems)
	{
		if (!IsValid(System))
		{
			continue;
		}

		UNiagaraComponent* WarmupComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			System,
			WarmupLocation,
			FRotator::ZeroRotator,
			WarmupScale,
			true,
			true
		);
		if (IsValid(WarmupComp))
		{
			WarmupComp->Deactivate();
		}
	}
}

void UWeaponInstance::PlayMuzzleFlash()
{
	for (FActiveMuzzleInfo& Muzzle : ActiveMuzzles)
	{
		if (IsValid(Muzzle.MuzzleFlashComp))
		{
			Muzzle.MuzzleFlashComp->SetRelativeScale3D(FXSettings.MuzzleFlashScale);
			Muzzle.MuzzleFlashComp->Activate(true);
			break;
		}
	}

	// 사운드 재생
	if (IsValid(FXSettings.FireSound) && ActiveMuzzles.Num() > 0)
	{
		FTransform MuzzleTransform = GetMuzzleTransform();
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			FXSettings.FireSound,
			MuzzleTransform.GetLocation()
		);
	}
}

void UWeaponInstance::PlayImpact(const FHitResult& HitResult)
{
	if (!IsValid(FXSettings.DefaultImpactVFX))
	{
		return;
	}

	const FVector ImpactNormalVector = FVector(HitResult.ImpactNormal);
	const FVector ImpactNormal = FXSettings.bFlipImpactNormal
		? -ImpactNormalVector
		: ImpactNormalVector;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		FXSettings.DefaultImpactVFX,
		HitResult.ImpactPoint,
		ImpactNormal.Rotation()
	);

	if (IsValid(FXSettings.ImpactSound))
	{
		UGameplayStatics::PlaySoundAtLocation(	
			GetWorld(),
			FXSettings.ImpactSound,
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
