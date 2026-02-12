#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "PRHUDWidget.generated.h"

class UPRHUDViewModel;
class UTextBlock;
class UImage;
class UTexture2D;
class IPRProgressBarInterface;

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
	// 가시성 변경 처리
	UFUNCTION()
	void HandleVisibilityChanged(bool bVisible);

	UFUNCTION()
	void HandleAmmoChanged(int32 Current, int32 Max);

	// 예비 탄약 변경 처리
	UFUNCTION()
	void HandleReserveAmmoChanged(int32 Current, int32 Max);

	UFUNCTION()
	void HandleWeaponTypeChanged(const FGameplayTag& NewTag);

	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);

	UFUNCTION()
	void HandleShieldChanged(float Current, float Max);

	UFUNCTION()
	void HandleHealthSegmentChanged(int32 NumSegments, float Spacing);

	UFUNCTION()
	void HandleShieldSegmentChanged(int32 NumSegments, float Spacing);

protected:
	// 탄약 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AmmoText;

	// 예비 탄약 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ReserveAmmoText;

	// 예비 탄약 최대값 텍스트 (옵션)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MaxReserveAmmoText;

	// 무기 아이콘 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> WeaponIcon;

	// 체력 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> HealthBar;

	// 실드 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> ShieldBar;

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