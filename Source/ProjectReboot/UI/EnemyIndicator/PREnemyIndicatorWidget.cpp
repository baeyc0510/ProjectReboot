// PREnemyIndicatorWidget.cpp

#include "ProjectReboot/UI/EnemyIndicator/PREnemyIndicatorWidget.h"

#include "ProjectReboot/UI/EnemyIndicator/PREnemyIndicatorViewModel.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Engine/Engine.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

void UPREnemyIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	BindViewModel();
}

void UPREnemyIndicatorWidget::NativeDestruct()
{
	UnbindViewModel();
	ActorToSlotIndex.Empty();
	SlotStates.Empty();
	Super::NativeDestruct();
}

void UPREnemyIndicatorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!IsValid(IndicatorCanvas))
	{
		return;
	}

	const FVector2D LayoutSize = ResolveLayoutSize();
	if (LayoutSize.IsNearlyZero())
	{
		return;
	}

	const FVector2D ScreenCenter = LayoutSize * 0.5f;
	const float RadiusPixels = LayoutSize.Y * IndicatorRadiusRatio;
	const float Alpha = FMath::Clamp(InDeltaTime * InterpSpeed, 0.0f, 1.0f);

	const int32 NumSlots = FMath::Min(WidgetPool.Num(), SlotStates.Num());
	for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
	{
		FIndicatorSlotState& State = SlotStates[SlotIndex];
		UUserWidget* Widget = WidgetPool[SlotIndex];
		if (!IsValid(Widget))
		{
			continue;
		}

		if (!State.bActive || !State.EnemyActor.IsValid())
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
			continue;
		}

		Widget->SetVisibility(ESlateVisibility::HitTestInvisible);

		// 각도 보간(최단 회전) + 스케일 보간
		const float DeltaAngle = FMath::FindDeltaAngleDegrees(State.CurrentAngle, State.TargetAngle);
		State.CurrentAngle = FMath::UnwindDegrees(State.CurrentAngle + DeltaAngle * Alpha);
		if (State.CurrentAngle < 0.0f)
		{
			State.CurrentAngle += 360.0f;
		}

		State.CurrentScale = FMath::Lerp(State.CurrentScale, State.TargetScale, Alpha);

		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot);
		if (!CanvasSlot)
		{
			continue;
		}

		const float AngleRad = FMath::DegreesToRadians(State.CurrentAngle);
		FVector2D WidgetPos;
		WidgetPos.X = ScreenCenter.X + RadiusPixels * FMath::Sin(AngleRad);
		WidgetPos.Y = ScreenCenter.Y - RadiusPixels * FMath::Cos(AngleRad);

		CanvasSlot->SetPosition(WidgetPos);
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));

		Widget->SetRenderTransformAngle(State.CurrentAngle);

		Widget->SetRenderScale(FVector2D(State.CurrentScale, State.CurrentScale));
	}
}

void UPREnemyIndicatorWidget::BindViewModel()
{
	ULocalPlayer* LP = GetOwningLocalPlayer();
	if (!LP)
	{
		return;
	}

	UPRViewModelSubsystem* VMS = LP->GetSubsystem<UPRViewModelSubsystem>();
	if (!VMS)
	{
		return;
	}

	ViewModel = VMS->GetOrCreateGlobalViewModel<UPREnemyIndicatorViewModel>();
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnIndicatorsUpdated.AddDynamic(this, &UPREnemyIndicatorWidget::HandleIndicatorsUpdated);
	ViewModel->OnVisibilityChanged.AddDynamic(this, &UPREnemyIndicatorWidget::HandleVisibilityChanged);

	ApplyInitialState();
}

void UPREnemyIndicatorWidget::UnbindViewModel()
{
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnIndicatorsUpdated.RemoveDynamic(this, &UPREnemyIndicatorWidget::HandleIndicatorsUpdated);
	ViewModel->OnVisibilityChanged.RemoveDynamic(this, &UPREnemyIndicatorWidget::HandleVisibilityChanged);

	ViewModel = nullptr;
}

void UPREnemyIndicatorWidget::ApplyInitialState()
{
	if (!ViewModel)
	{
		return;
	}

	HandleIndicatorsUpdated(ViewModel->GetIndicators());
	HandleVisibilityChanged(ViewModel->IsVisible());
}

void UPREnemyIndicatorWidget::HandleVisibilityChanged(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UPREnemyIndicatorWidget::HandleIndicatorsUpdated(const TArray<FPREnemyIndicatorData>& NewIndicators)
{
	UpdateIndicatorWidgets(NewIndicators);
}

FVector2D UPREnemyIndicatorWidget::ResolveLayoutSize() const
{
	if (IsValid(IndicatorCanvas))
	{
		FVector2D LayoutSize = IndicatorCanvas->GetCachedGeometry().GetLocalSize();
		if (!LayoutSize.IsNearlyZero())
		{
			return LayoutSize;
		}
	}

	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(const_cast<UPREnemyIndicatorWidget*>(this));
	return (ViewportScale > KINDA_SMALL_NUMBER) ? (ViewportSize / ViewportScale) : ViewportSize;
}

int32 UPREnemyIndicatorWidget::AllocateSlotForActor(AActor* EnemyActor)
{
	if (!IsValid(EnemyActor))
	{
		return INDEX_NONE;
	}

	const TWeakObjectPtr<AActor> EnemyKey = EnemyActor;
	if (const int32* ExistingIndex = ActorToSlotIndex.Find(EnemyKey))
	{
		return *ExistingIndex;
	}

	// 비어있는 슬롯 검색
	for (int32 SlotIndex = 0; SlotIndex < SlotStates.Num(); ++SlotIndex)
	{
		if (!SlotStates[SlotIndex].bActive || !SlotStates[SlotIndex].EnemyActor.IsValid())
		{
			SlotStates[SlotIndex] = FIndicatorSlotState{};
			SlotStates[SlotIndex].EnemyActor = EnemyActor;
			SlotStates[SlotIndex].bActive = true;
			ActorToSlotIndex.Add(EnemyKey, SlotIndex);
			return SlotIndex;
		}
	}

	// 새 슬롯 추가
	const int32 NewIndex = SlotStates.AddDefaulted();
	SlotStates[NewIndex].EnemyActor = EnemyActor;
	SlotStates[NewIndex].bActive = true;
	ActorToSlotIndex.Add(EnemyKey, NewIndex);
	return NewIndex;
}

void UPREnemyIndicatorWidget::UpdateIndicatorWidgets(const TArray<FPREnemyIndicatorData>& NewIndicators)
{
	if (!IsValid(IndicatorCanvas) || !IndicatorIconClass)
	{
		return;
	}

	auto ApplyIconSlotLayout = [this](UCanvasPanelSlot* CanvasSlot)
	{
		if (!CanvasSlot)
		{
			return;
		}

		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CanvasSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		CanvasSlot->SetAutoSize(true);
	};

	// 슬롯/풀 준비 (액터 매핑 기반이라 NewIndicators.Num()과 1:1이 아닐 수 있음)
	// - 먼저 들어온 액터가 늘어나면 SlotStates가 증가하고, 그에 맞춰 WidgetPool도 확장
	TSet<TWeakObjectPtr<AActor>> IncomingActors;
	IncomingActors.Reserve(NewIndicators.Num());
	for (const FPREnemyIndicatorData& Data : NewIndicators)
	{
		if (IsValid(Data.EnemyActor))
		{
			IncomingActors.Add(Data.EnemyActor);
		}
	}

	// 제거된 액터 슬롯 정리
	for (auto It = ActorToSlotIndex.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || !IncomingActors.Contains(It.Key()))
		{
			const int32 SlotIndex = It.Value();
			if (SlotStates.IsValidIndex(SlotIndex))
			{
				SlotStates[SlotIndex].bActive = false;
				SlotStates[SlotIndex].bInitialized = false;
				SlotStates[SlotIndex].EnemyActor.Reset();
			}
			It.RemoveCurrent();
		}
	}

	// 필요한 슬롯 할당 + 풀 확장
	int32 MaxRequiredSlotIndex = -1;
	for (const FPREnemyIndicatorData& Data : NewIndicators)
	{
		const int32 SlotIndex = AllocateSlotForActor(Data.EnemyActor);
		MaxRequiredSlotIndex = FMath::Max(MaxRequiredSlotIndex, SlotIndex);
	}

	const int32 RequiredPoolSize = MaxRequiredSlotIndex + 1;
	while (WidgetPool.Num() < RequiredPoolSize)
	{
		UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), IndicatorIconClass);
		if (!IsValid(NewWidget))
		{
			break;
		}

		NewWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

		IndicatorCanvas->AddChild(NewWidget);
		if (UCanvasPanelSlot* NewCanvasSlot = Cast<UCanvasPanelSlot>(NewWidget->Slot))
		{
			ApplyIconSlotLayout(NewCanvasSlot);
		}

		NewWidget->SetVisibility(ESlateVisibility::Collapsed);
		WidgetPool.Add(NewWidget);
	}

	// 풀링된 위젯에 공통 레이아웃 세팅 반영
	for (UUserWidget* Widget : WidgetPool)
	{
		if (!IsValid(Widget))
		{
			continue;
		}

		Widget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

		ApplyIconSlotLayout(Cast<UCanvasPanelSlot>(Widget->Slot));
	}

	// SlotStates가 WidgetPool보다 짧으면 맞춰줌
	if (SlotStates.Num() < WidgetPool.Num())
	{
		SlotStates.SetNum(WidgetPool.Num());
	}

	// 목표값(Target) 갱신 (표시는 NativeTick에서 보간)
	for (const FPREnemyIndicatorData& Data : NewIndicators)
	{
		if (!IsValid(Data.EnemyActor))
		{
			continue;
		}

		const int32 SlotIndex = AllocateSlotForActor(Data.EnemyActor);
		if (!SlotStates.IsValidIndex(SlotIndex))
		{
			continue;
		}

		FIndicatorSlotState& State = SlotStates[SlotIndex];
		State.bActive = true;
		State.EnemyActor = Data.EnemyActor;
		State.TargetAngle = FMath::Fmod(Data.ScreenAngle + 360.0f, 360.0f);

		const float DistanceAlpha = FMath::Clamp(Data.Distance / MaxDistance, 0.0f, 1.0f);
		State.TargetScale = FMath::Lerp(1.0f, 0.5f, DistanceAlpha);

		// 새로 감지(또는 슬롯 재사용)된 경우: 최초 1회는 즉시 목표값으로 스냅하여
		// 0도(위)에서 보간 이동하는 느낌/이전 슬롯 위치에서 이동하는 느낌을 제거
		if (!State.bInitialized)
		{
			State.CurrentAngle = State.TargetAngle;
			State.CurrentScale = State.TargetScale;
			State.bInitialized = true;
		}
	}
}
