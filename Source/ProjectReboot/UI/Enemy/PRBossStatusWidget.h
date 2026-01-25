// PRBossStatusWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRBossStatusWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UPREnemyStatusViewModel;

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
	UFUNCTION()
	void HandleEnemyDisplayNameChanged(const FText& DisplayName);

	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);

	UFUNCTION()
	void HandleShieldChanged(float Current, float Max);

	UFUNCTION()
	void HandleDestructStatus();
protected:
	// 보스 이름
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BossNameText;

	// 체력 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	// 실드 바
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ShieldBar;

private:
	void UnbindViewModel();
	void ApplyInitialState();

private:
	UPROPERTY()
	TObjectPtr<UPREnemyStatusViewModel> ViewModel;
};
