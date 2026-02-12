// PREnemyIndicatorViewModel.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelBase.h"
#include "PREnemyIndicatorViewModel.generated.h"

class UPREnemyDetectionComponent;

USTRUCT(BlueprintType)
struct FPREnemyIndicatorData
{
	GENERATED_BODY()

	// 적 액터 참조
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> EnemyActor = nullptr;

	// 카메라 Forward 기준 각도 (0=위, 90=오른쪽, 180=아래, 270=왼쪽)
	UPROPERTY(BlueprintReadOnly)
	float ScreenAngle = 0.0f;

	// 플레이어로부터의 거리 (cm)
	UPROPERTY(BlueprintReadOnly)
	float Distance = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyIndicatorsUpdated,
	const TArray<FPREnemyIndicatorData>&, Indicators);

/**
 * 적 인디케이터 뷰모델
 * 주변 적의 화면 위치/각도를 계산하여 Widget에 전달
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPREnemyIndicatorViewModel : public UPRViewModelBase
{
	GENERATED_BODY()

public:
	// 생성자
	UPREnemyIndicatorViewModel();

	/*~ UPRViewModelBase Interface ~*/
	virtual void InitializeForPlayer(ULocalPlayer* InLocalPlayer) override;
	virtual void Deinitialize() override;

	// 현재 인디케이터 데이터 반환
	UFUNCTION(BlueprintPure, Category = "EnemyIndicator")
	const TArray<FPREnemyIndicatorData>& GetIndicators() const { return Indicators; }

	// 인디케이터 업데이트 이벤트
	UPROPERTY(BlueprintAssignable, Category = "EnemyIndicator|Events")
	FOnEnemyIndicatorsUpdated OnIndicatorsUpdated;

protected:
	// 위치 계산 타이머 콜백
	void UpdateIndicatorPositions();

	// 감지 목록 변경 이벤트 핸들러
	UFUNCTION()
	void HandleTrackedEnemiesChanged();

	// 감지 컴포넌트 바인딩
	void BindToDetectionComponent();

	// 감지 컴포넌트 바인딩 해제
	void UnbindFromDetectionComponent();

private:
	// 카메라 Forward 기준 각도 계산
	float CalculateScreenAngle(APlayerController* PC, const FVector& EnemyLocation) const;

	// 화면 안에 있는 적인지 판별
	bool IsOnScreen(APlayerController* PC, const FVector& EnemyLocation) const;

protected:
	// 위치 갱신 간격 (초)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyIndicator")
	float UpdateInterval = 0.1f;

	// 최대 표시 개수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EnemyIndicator")
	int32 MaxIndicators = 5;

private:
	UPROPERTY()
	TArray<FPREnemyIndicatorData> Indicators;

	TWeakObjectPtr<UPREnemyDetectionComponent> DetectionComponent;
	FTimerHandle UpdateTimerHandle;
};
