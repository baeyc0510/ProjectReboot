// PRBossStatusWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRBossStatusWidget.generated.h"

class UTextBlock;
class UPREnemyStatusViewModel;
class IPRProgressBarInterface;

/**
 * 보스 체력/실드/이름 표시 위젯
 */
UCLASS()
class PROJECTREBOOT_API UPRBossStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/*~ UUserWidget Interfaces ~*/
	// 위젯 생성 시 호출
	virtual void NativeConstruct() override;

	// 위젯 파괴 시 호출
	virtual void NativeDestruct() override;

public:
	/*~ UPRBossStatusWidget Interface ~*/
	// 뷰모델 바인딩
	UFUNCTION(BlueprintCallable, Category = "BossHUD")
	void BindViewModel(UPREnemyStatusViewModel* TargetViewModel);

protected:
	/*~ ViewModel Event Handlers ~*/
	// 가시성 변경 처리
	UFUNCTION()
	void HandleVisibilityChanged(bool bVisible);

	UFUNCTION()
	void HandleEnemyDisplayNameChanged(const FText& DisplayName);

	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);

	UFUNCTION()
	void HandleShieldChanged(float Current, float Max);

	UFUNCTION()
	void HandleDestructStatus();

	UFUNCTION()
	void HandleHealthSegmentChanged(int32 NumSegments, float Spacing);

	UFUNCTION()
	void HandleShieldSegmentChanged(int32 NumSegments, float Spacing);

protected:
	// 보스 이름
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BossNameText;

	// 체력 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> HealthBar;

	// 실드 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUserWidget> ShieldBar;

private:
	void UnbindViewModel();
	void ApplyInitialState();

private:
	UPROPERTY()
	TObjectPtr<UPREnemyStatusViewModel> ViewModel;
};
