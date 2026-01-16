// PRCrosshairViewModel.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelBase.h"
#include "PRCrosshairViewModel.generated.h"

struct FOnAttributeChangeData;
class UAbilitySystemComponent;
struct FPRCrosshairSetting;
class UPRCrosshairConfig;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairSpreadChanged, float, NewSpread);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairADSStateChanged, bool, bIsADS);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairADSAlphaChanged, float, NewAlpha);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairCanFireChanged, bool, bCanFire);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrosshairAmmoChanged, int32, Current, int32, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairVisibilityChanged, bool, bVisible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairTargetingEnemyChanged, bool, bTargeting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrosshairTagChanged, const FGameplayTag&, NewTag);

/**
 * 크로스헤어 상태를 관리하는 ViewModel
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRCrosshairViewModel : public UPRViewModelBase
{
    GENERATED_BODY()

public:
    /*~ UPRViewModelBase Interface ~*/
    virtual void InitializeForPlayer(ULocalPlayer* InLocalPlayer) override;
    virtual void Deinitialize() override;
    
    /*~ UPRCrosshairViewModel Interface ~*/

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

    // ADS 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetADSState(bool bNewIsADS);

    // 사격 시 호출 (반동 스프레드 추가)
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void OnFired();

    // 이동 속도 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetMovementSpeed(float Speed);

    // 탄약 정보 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetAmmo(int32 Current, int32 Max);

    // 발사 가능 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetCanFire(bool bNewCanFire);

    // 가시성 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetVisible(bool bNewVisible);

    // 적 타겟팅 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Crosshair|Input")
    void SetTargetingEnemy(bool bNewTargeting);

    // 현재 스프레드 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    float GetCurrentSpread() const { return CurrentSpread; }

    // 정규화된 스프레드 반환 (0~1)
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    float GetNormalizedSpread() const { return NormalizedSpread; }

    // ADS 상태 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    bool IsADS() const { return bIsADS; }

    // ADS 알파 반환 (0=Hip, 1=ADS)
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    float GetADSAlpha() const { return ADSAlpha; }

    // 발사 가능 여부 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    bool CanFire() const { return bCanFire; }

    // 현재 탄약 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    int32 GetCurrentAmmo() const { return CurrentAmmo; }

    // 최대 탄약 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    int32 GetMaxAmmo() const { return MaxAmmo; }

    // 가시성 반환
    UFUNCTION(BlueprintPure, Category = "Crosshair|State")
    bool IsVisible() const { return bIsVisible; }

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
    FOnCrosshairSpreadChanged OnSpreadChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairADSStateChanged OnADSStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairADSAlphaChanged OnADSAlphaChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairCanFireChanged OnCanFireChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairAmmoChanged OnAmmoChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairVisibilityChanged OnVisibilityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairTargetingEnemyChanged OnTargetingEnemyChanged;

    UPROPERTY(BlueprintAssignable, Category = "Crosshair|Events")
    FOnCrosshairTagChanged OnCrosshairTagChanged;

private:
    // 매 프레임 업데이트
    bool Tick(float DeltaTime);

    // 스프레드 재계산
    void RecalculateSpread(float DeltaTime);

    // ADS 알파 업데이트
    void UpdateADSAlpha(float DeltaTime);

    // ASC 이벤트 핸들러
    void HandleCrosshairTagChanged(const FGameplayTag Tag, int32 NewCount);
    void HandleADSTagChanged(const FGameplayTag Tag, int32 NewCount);
    void HandleCannotFireTagChanged(const FGameplayTag Tag, int32 NewCount);
    void HandleAmmoChanged(const FOnAttributeChangeData& Data);
    void HandleMaxAmmoChanged(const FOnAttributeChangeData& Data);
    void HandleMoveSpeedChanged(const FOnAttributeChangeData& Data);
    
private:
    /*~ AbilitySystem ~*/
    // ASC Cache
    UPROPERTY()
    TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
    
    // Attribute 델리게이트 핸들
    FDelegateHandle CrosshairTagHandle;
    FDelegateHandle ADSTagHandle;
    FDelegateHandle CannotFireTagHandle;
    FDelegateHandle AmmoHandle;
    FDelegateHandle MaxAmmoHandle;
    FDelegateHandle MoveSpeedHandle;
    
    /*~ Configs ~*/
    UPROPERTY()
    TObjectPtr<UPRCrosshairConfig> Config;
    FGameplayTag CurrentCrosshairTag;

    /*~ States ~*/
    float CurrentSpread = 0.0f;
    float NormalizedSpread = 0.0f;
    float RecoilSpread = 0.0f;
    float MovementSpeed = 0.0f;

    bool bIsADS = false;
    float ADSAlpha = 0.0f;

    bool bCanFire = true;
    int32 CurrentAmmo = 0;
    int32 MaxAmmo = 0;

    bool bIsVisible = true;
    bool bIsTargetingEnemy = false;

    FTSTicker::FDelegateHandle TickHandle;

    static constexpr float ADSTransitionSpeed = 10.0f;
    static constexpr float MaxWalkSpeed = 600.0f;
    static constexpr float SpreadChangeTolerance = 0.5f;
};