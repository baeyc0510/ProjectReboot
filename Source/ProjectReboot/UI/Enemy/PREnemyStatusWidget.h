// PREnemyStatusWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PREnemyStatusWidget.generated.h"

class UProgressBar;
class UPREnemyStatusViewModel;

/**
 * 적 체력/실드 표시 위젯
 */
UCLASS()
class PROJECTREBOOT_API UPREnemyStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/*~ UUserWidget Interfaces ~*/
	// 위젯 생성 시 호출
	virtual void NativeConstruct() override;

	// 위젯 파괴 시 호출
	virtual void NativeDestruct() override;

public:
	/*~ UPREnemyStatusWidget Interface ~*/
	// 뷰모델 바인딩
	UFUNCTION(BlueprintCallable, Category = "EnemyHUD")
	void BindViewModel(UPREnemyStatusViewModel* TargetViewModel);

protected:
	/*~ ViewModel Event Handlers ~*/
	UFUNCTION()
	void HandleHealthChanged(float Current, float Max);

	UFUNCTION()
	void HandleShieldChanged(float Current, float Max);

protected:
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
