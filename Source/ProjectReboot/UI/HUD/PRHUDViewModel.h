#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelBase.h"
#include "GameplayTagContainer.h"
#include "PRHUDViewModel.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHUDAmmoChanged, int32, Current, int32, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHUDHealthChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHUDWeaponTypeChanged, const FGameplayTag&, WeaponTypeTag);

/**
 * HUD 뷰모델
 * 무기 타입(실탄, 빔, 미사일)에 따른 탄약/에너지 표시 관리
 * 플레이어 체력 표시 관리
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRHUDViewModel : public UPRViewModelBase
{
	GENERATED_BODY()

public:
	virtual void InitializeForPlayer(ULocalPlayer* InLocalPlayer) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindToASC(UAbilitySystemComponent* InASC);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UnbindFromASC();

	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetMaxAmmo() const { return MaxAmmo; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "HUD")
	FGameplayTag GetWeaponTypeTag() const { return WeaponTypeTag; }

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnHUDAmmoChanged OnAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnHUDHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "HUD|Events")
	FOnHUDWeaponTypeChanged OnWeaponTypeChanged;

private:
	void SetAmmo(int32 NewCurrent, int32 NewMax);
	void SetHealth(float NewCurrent, float NewMax);
	void SetWeaponType(const FGameplayTag& NewType);

	void UpdateAttributesBindings();
	void ClearAttributeBindings();

	// ASC 이벤트 핸들러
	void HandleWeaponTagChanged(const FGameplayTag Tag, int32 NewCount);
	
	void HandleAmmoChanged(const FOnAttributeChangeData& Data);
	void HandleMaxAmmoChanged(const FOnAttributeChangeData& Data);
	
	void HandleEnergyChanged(const FOnAttributeChangeData& Data);
	void HandleMaxEnergyChanged(const FOnAttributeChangeData& Data);
	
	void HandleMissileChanged(const FOnAttributeChangeData& Data);
	void HandleMaxMissileChanged(const FOnAttributeChangeData& Data);

	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);

private:
	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	FGameplayTag WeaponTypeTag;
	int32 CurrentAmmo = 0;
	int32 MaxAmmo = 0;
	float CurrentHealth = 0.0f;
	float MaxHealth = 0.0f;

	// 어트리뷰트 델리게이트 핸들
	FDelegateHandle AmmoChangeHandle;
	FDelegateHandle MaxAmmoChangeHandle;
	FDelegateHandle HealthChangeHandle;
	FDelegateHandle MaxHealthChangeHandle;
	
	// 태그 델리게이트 핸들
	FDelegateHandle BulletTagHandle;
	FDelegateHandle BeamTagHandle;
	FDelegateHandle MissileTagHandle;
};
