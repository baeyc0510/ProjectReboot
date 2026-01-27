// PRLockOnWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRLockOnWidget.generated.h"

class UImage;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPRLockOnViewModel;

/**
 * 락온 인디케이터 위젯
 * - Material의 "Progress" 파라미터로 진행률 표시
 */
UCLASS()
class PROJECTREBOOT_API UPRLockOnWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	// 뷰모델 바인딩
	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void BindViewModel(UPRLockOnViewModel* InViewModel);

protected:
	// ViewModel 이벤트 핸들러
	UFUNCTION()
	void HandleVisibilityChanged(bool bIsVisible);

	UFUNCTION()
	void HandleProgressChanged(float Progress);

	UFUNCTION()
	void HandleLockedStateChanged(bool bIsLocked);

protected:
	// 락온 인디케이터 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LockOnImage;

	// 락온 인디케이터 머티리얼 (에디터에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	TObjectPtr<UMaterialInterface> LockOnMaterial;

	// Progress 파라미터 이름
	UPROPERTY(EditDefaultsOnly, Category = "LockOn")
	FName ProgressParameterName = TEXT("Progress");

private:
	void UnbindViewModel();
	void ApplyInitialState();
	void SetProgress(float Progress);

private:
	UPROPERTY()
	TObjectPtr<UPRLockOnViewModel> ViewModel;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MaterialInstance;
};
