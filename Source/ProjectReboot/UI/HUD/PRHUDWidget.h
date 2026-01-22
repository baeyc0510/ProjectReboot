#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PRHUDWidget.generated.h"

class UPRHUDViewModel;
class UTextBlock;
class UImage;
class UTexture2D;
class UProgressBar;

/**
 * HUD 위젯
 * 탄약 및 무기 아이콘 표시
 * 플레이어 체력 표시
 */
UCLASS()
class PROJECTREBOOT_API UPRHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

protected:
	/*~ ViewModel Event Handlers ~*/
	UFUNCTION()
	void HandleAmmoChanged(int32 Current, int32 Max);

	UFUNCTION()
	void HandleWeaponTypeChanged(const FGameplayTag& NewTag);

	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);

	UFUNCTION()
	void HandleShieldChanged(float Current, float Max);

protected:
	// 탄약 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoText;

	// 무기 아이콘 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WeaponIcon;

	// 체력 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	// 실드 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ShieldBar;

	// 무기 타입별 아이콘 매핑
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TMap<FGameplayTag, TObjectPtr<UTexture2D>> WeaponIconMap;

private:
	void BindViewModel();
	void UnbindViewModel();
	void ApplyInitialState();

private:
	UPROPERTY()
	TObjectPtr<UPRHUDViewModel> ViewModel;
};