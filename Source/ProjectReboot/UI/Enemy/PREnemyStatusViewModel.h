// PREnemyStatusViewModel.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelBase.h"
#include "PREnemyStatusViewModel.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDisplayNameChanged, const FText&, DisplayName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyAttributeChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestructStatus);

/**
 * 적 체력/실드 상태 뷰모델
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPREnemyStatusViewModel : public UPRViewModelBase
{
	GENERATED_BODY()

public:
	/*~ UPRViewModelBase Interface ~*/
	// Actor-Bound ViewModel 초기화
	virtual void InitializeForActor(AActor* InTargetActor, ULocalPlayer* InLocalPlayer) override;

	// ViewModel 정리
	virtual void Deinitialize() override;

public:
	/*~ UPREnemyStatusViewModel Interface ~*/
	// ASC 바인딩
	UFUNCTION(BlueprintCallable, Category = "EnemyHUD")
	void BindToASC(UAbilitySystemComponent* InASC);

	// ASC 바인딩 해제
	UFUNCTION(BlueprintCallable, Category = "EnemyHUD")
	void UnbindFromASC();

	UFUNCTION(BlueprintCallable, Category = "EnemyHUD")
	void DestructStatus();
	
	// 이름 반환
	UFUNCTION(BlueprintPure, Category = "EnemyHUD")
	FText GetEnemyDisplayName() const {return EnemyDisplayName; }
	
	// 현재 체력 반환
	UFUNCTION(BlueprintPure, Category = "EnemyHUD")
	float GetCurrentHealth() const { return CurrentHealth; }

	// 최대 체력 반환
	UFUNCTION(BlueprintPure, Category = "EnemyHUD")
	float GetMaxHealth() const { return MaxHealth; }

	// 현재 실드 반환
	UFUNCTION(BlueprintPure, Category = "EnemyHUD")
	float GetCurrentShield() const { return CurrentShield; }

	// 최대 실드 반환
	UFUNCTION(BlueprintPure, Category = "EnemyHUD")
	float GetMaxShield() const { return MaxShield; }

public:
	UPROPERTY(BlueprintAssignable, Category = "EnemyHUD|Events")
	FOnEnemyDisplayNameChanged OnEnemyDisplayNameChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "EnemyHUD|Events")
	FOnEnemyAttributeChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "EnemyHUD|Events")
	FOnEnemyAttributeChanged OnShieldChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "EnemyHUD|Events")
	FOnDestructStatus OnDestructStatus;

private:
	// 값 갱신
	void SetEnemyDisplayName(const FText& InDisplayName);
	void SetHealth(float NewCurrent, float NewMax);
	void SetShield(float NewCurrent, float NewMax);
	
	void UpdateAttributeBindings();
	void ClearAttributeBindings();

	// ASC 이벤트 핸들러
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleShieldChanged(const FOnAttributeChangeData& Data);
	void HandleMaxShieldChanged(const FOnAttributeChangeData& Data);

private:
	// ASC 캐시
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;

	FText EnemyDisplayName;
	float CurrentHealth = 0.0f;
	float MaxHealth = 0.0f;
	float CurrentShield = 0.0f;
	float MaxShield = 0.0f;

	// 어트리뷰트 델리게이트 핸들
	FDelegateHandle HealthChangeHandle;
	FDelegateHandle MaxHealthChangeHandle;
	FDelegateHandle ShieldChangeHandle;
	FDelegateHandle MaxShieldChangeHandle;
};
