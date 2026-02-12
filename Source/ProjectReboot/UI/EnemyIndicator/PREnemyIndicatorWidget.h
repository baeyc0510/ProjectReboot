// PREnemyIndicatorWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PREnemyIndicatorWidget.generated.h"

struct FPREnemyIndicatorData;
class UPREnemyIndicatorViewModel;
class UCanvasPanel;

/**
 * 적 인디케이터 위젯
 * 화면 중앙 기준 원형 링 위에 적 방향 아이콘 표시
 */
UCLASS()
class PROJECTREBOOT_API UPREnemyIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	/*~ ViewModel Event Handlers ~*/
	// 가시성 변경 처리
	UFUNCTION()
	void HandleVisibilityChanged(bool bVisible);

	// 인디케이터 업데이트 처리
	UFUNCTION()
	void HandleIndicatorsUpdated(const TArray<FPREnemyIndicatorData>& NewIndicators);

protected:
	// 개별 인디케이터 아이콘 위젯 클래스 (BP에서 지정)
	UPROPERTY(EditDefaultsOnly, Category = "EnemyIndicator")
	TSubclassOf<UUserWidget> IndicatorIconClass;

	// 인디케이터 배치용 Canvas
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> IndicatorCanvas;

	// 원형 링 반지름 (화면 높이 대비 비율, 0.0~0.5)
	UPROPERTY(EditDefaultsOnly, Category = "EnemyIndicator", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float IndicatorRadiusRatio = 0.35f;

	// 감지 반경 (거리 스케일 계산용, DetectionComponent 반경과 맞춤)
	UPROPERTY(EditDefaultsOnly, Category = "EnemyIndicator")
	float MaxDistance = 3000.0f;

	// 위젯 보간 속도 (클수록 목표값에 빨리 수렴)
	UPROPERTY(EditDefaultsOnly, Category = "EnemyIndicator", meta = (ClampMin = "0.1", ClampMax = "60.0"))
	float InterpSpeed = 12.0f;

private:
	void BindViewModel();
	void UnbindViewModel();
	void ApplyInitialState();
	void UpdateIndicatorWidgets(const TArray<FPREnemyIndicatorData>& NewIndicators);
	FVector2D ResolveLayoutSize() const;
	int32 AllocateSlotForActor(AActor* EnemyActor);

private:
	UPROPERTY()
	TObjectPtr<UPREnemyIndicatorViewModel> ViewModel;

	// 풀링된 인디케이터 위젯들
	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> WidgetPool;

	struct FIndicatorSlotState
	{
		TWeakObjectPtr<AActor> EnemyActor;
		float CurrentAngle = 0.0f;
		float TargetAngle = 0.0f;
		float CurrentScale = 1.0f;
		float TargetScale = 1.0f;
		// 최초 활성화 시 Current 값을 Target으로 즉시 동기화했는지 여부
		bool bInitialized = false;
		bool bActive = false;
	};

	TArray<FIndicatorSlotState> SlotStates;
	TMap<TWeakObjectPtr<AActor>, int32> ActorToSlotIndex;
};
