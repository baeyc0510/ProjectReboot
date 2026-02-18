// PRCrosshairViewModel.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelBase.h"
#include "PRCrosshairViewModel.generated.h"

class APRCharacterBase;
struct FOnAttributeChangeData;
class UAbilitySystemComponent;
struct FPRCrosshairSetting;
class UPRCrosshairConfig;

UENUM(BlueprintType)
enum class EPRHitMarkerType : uint8
{
	// 일반 히트
	Normal,
	// 킬 확인
	Kill
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairCanFireChanged, bool, bCanFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairTargetingEnemyChanged, bool, bTargeting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairTagChanged, const FGameplayTag&, NewTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitMarkerTriggered, EPRHitMarkerType, HitMarkerType);

/**
 * 크로스헤어 상태를 관리하는 ViewModel
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRCrosshairViewModel : public UPRViewModelBase
{
    GENERATED_BODY()

public:
    // 생성자
    UPRCrosshairViewModel();

    /*~ UPRViewModelBase Interface ~*/
    virtual void InitializeForPlayer(ULocalPlayer* InLocalPlayer) override;
    virtual void Deinitialize() override;

    /*~ UPRCrosshairViewModel Interface ~*/
    UFUNCTION(BlueprintCallable, Category = "Crosshair")
    void SetCharacter(APRCharacterBase* InCharacter);

    // ASC 바인딩
    UFUNCTION(BlueprintCallable, Category = "Crosshair")
    void BindToASC(UAbilitySystemComponent* InASC);

    UFUNCTION(BlueprintCallable, Category = "Crosshair")
    void UnbindFromASC();

    // Config 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair")
    void SetConfig(UPRCrosshairConfig* InConfig);

    // 크로스헤어 태그 변경 (무기 변경 시)
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetCrosshairTag(const FGameplayTag& NewTag);

    // 발사 가능 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetCanFire(bool bNewCanFire);

    // 적 타겟팅 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetTargetingEnemy(bool bNewTargeting);

    // 히트 마커 표시 요청
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void ShowHitMarker(EPRHitMarkerType HitMarkerType);

    // 발사 가능 여부 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    bool CanFire() const { return bCanFire; }

    // 적 타겟팅 여부 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    bool IsTargetingEnemy() const { return bIsTargetingEnemy; }

    // 현재 크로스헤어 태그 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    FGameplayTag GetCurrentCrosshairTag() const { return CurrentCrosshairTag; }

    // 현재 크로스헤어 설정 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    const FPRCrosshairSetting& GetCurrentSetting() const;

    // Config 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    UPRCrosshairConfig* GetConfig() const { return Config; }

public:
    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairCanFireChanged OnCanFireChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairTargetingEnemyChanged OnTargetingEnemyChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairTagChanged OnCrosshairTagChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnHitMarkerTriggered OnHitMarkerTriggered;

private:
    // ASC 이벤트 핸들러
    void OnTagChanged(const FGameplayTag Tag, int32 NewCount);
    void HandleHideTagChanged(const FGameplayTag Tag, int32 NewCount);
    void HandleCrosshairTagChanged(const FGameplayTag Tag, int32 NewCount);
    void HandleCannotFireTagChanged(const FGameplayTag Tag, int32 NewCount);

private:
    TWeakObjectPtr<APRCharacterBase> PlayerCharacter;

    /*~ AbilitySystem ~*/
    TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
    FDelegateHandle StateTagHandle;

    /*~ Configs ~*/
    UPROPERTY()
    TObjectPtr<UPRCrosshairConfig> Config;
    FGameplayTag CurrentCrosshairTag;

    /*~ States ~*/
    bool bCanFire = true;
    bool bIsTargetingEnemy = false;
};
