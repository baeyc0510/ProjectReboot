// PREnemyIndicatorViewModel.cpp

#include "ProjectReboot/UI/EnemyIndicator/PREnemyIndicatorViewModel.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Combat/EnemyDetection/PREnemyDetectionComponent.h"

UPREnemyIndicatorViewModel::UPREnemyIndicatorViewModel()
{
	ViewModelTag = TAG_UI_ViewModel_HUD_EnemyIndicator;
}

void UPREnemyIndicatorViewModel::InitializeForPlayer(ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForPlayer(InLocalPlayer);
	SetVisible(true);

	BindToDetectionComponent();

	// 위치 갱신 타이머 시작
	if (IsValid(InLocalPlayer))
	{
		if (UWorld* World = InLocalPlayer->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				UpdateTimerHandle,
				this,
				&UPREnemyIndicatorViewModel::UpdateIndicatorPositions,
				UpdateInterval,
				true
			);
		}
	}

	UpdateIndicatorPositions();
}

void UPREnemyIndicatorViewModel::Deinitialize()
{
	UnbindFromDetectionComponent();

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UWorld* World = LP->GetWorld())
		{
			World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		}
	}

	Indicators.Empty();
	Super::Deinitialize();
}

void UPREnemyIndicatorViewModel::BindToDetectionComponent()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!IsValid(PC) || !IsValid(PC->GetPawn()))
	{
		return;
	}

	UPREnemyDetectionComponent* DetComp = PC->GetPawn()->FindComponentByClass<UPREnemyDetectionComponent>();
	if (!IsValid(DetComp))
	{
		return;
	}

	if (DetectionComponent.Get() == DetComp)
	{
		return;
	}

	UnbindFromDetectionComponent();

	DetectionComponent = DetComp;
	DetComp->OnTrackedEnemiesChanged.AddDynamic(this, &UPREnemyIndicatorViewModel::HandleTrackedEnemiesChanged);
}

void UPREnemyIndicatorViewModel::UnbindFromDetectionComponent()
{
	if (DetectionComponent.IsValid())
	{
		DetectionComponent->OnTrackedEnemiesChanged.RemoveDynamic(this, &UPREnemyIndicatorViewModel::HandleTrackedEnemiesChanged);
	}

	DetectionComponent.Reset();
}

void UPREnemyIndicatorViewModel::HandleTrackedEnemiesChanged()
{
	// 감지 목록 변경 시 즉시 위치 갱신
	UpdateIndicatorPositions();
}

void UPREnemyIndicatorViewModel::UpdateIndicatorPositions()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!IsValid(PC) || !IsValid(PC->GetPawn()) || !IsValid(PC->PlayerCameraManager))
	{
		return;
	}

	if (!DetectionComponent.IsValid())
	{
		BindToDetectionComponent();
	}

	if (!DetectionComponent.IsValid())
	{
		if (Indicators.Num() > 0)
		{
			Indicators.Empty();
			OnIndicatorsUpdated.Broadcast(Indicators);
		}
		return;
	}

	const FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
	TArray<FPREnemyIndicatorData> NewIndicators;

	for (const TWeakObjectPtr<AActor>& EnemyWeak : DetectionComponent->GetTrackedEnemies())
	{
		AActor* Enemy = EnemyWeak.Get();
		if (!IsValid(Enemy))
		{
			continue;
		}

		// 화면 안에 보이는 적은 인디케이터 표시하지 않음
		if (IsOnScreen(PC, Enemy->GetActorLocation()))
		{
			continue;
		}

		FPREnemyIndicatorData Data;
		Data.EnemyActor = Enemy;
		Data.Distance = FVector::Dist(PlayerLocation, Enemy->GetActorLocation());
		Data.ScreenAngle = CalculateScreenAngle(PC, Enemy->GetActorLocation());
		NewIndicators.Add(Data);
	}

	// 거리 순 정렬 (가까운 적 우선)
	NewIndicators.Sort([](const FPREnemyIndicatorData& A, const FPREnemyIndicatorData& B)
	{
		return A.Distance < B.Distance;
	});

	// 최대 개수 제한
	if (NewIndicators.Num() > MaxIndicators)
	{
		NewIndicators.SetNum(MaxIndicators);
	}

	Indicators = MoveTemp(NewIndicators);
	OnIndicatorsUpdated.Broadcast(Indicators);
}

float UPREnemyIndicatorViewModel::CalculateScreenAngle(APlayerController* PC, const FVector& EnemyLocation) const
{
	const FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

	// 플레이어 -> 적 방향 벡터 (수평면 투영)
	FVector ToEnemy = EnemyLocation - PlayerLocation;
	ToEnemy.Z = 0.0f;
	if (!ToEnemy.Normalize())
	{
		return 0.0f;
	}

	// 카메라 Forward/Right (수평면 투영)
	FVector CameraForward = PC->PlayerCameraManager->GetActorForwardVector();
	CameraForward.Z = 0.0f;
	CameraForward.Normalize();

	FVector CameraRight = PC->PlayerCameraManager->GetActorRightVector();
	CameraRight.Z = 0.0f;
	CameraRight.Normalize();

	// Atan2로 각도 계산
	const float DotForward = FVector::DotProduct(CameraForward, ToEnemy);
	const float DotRight = FVector::DotProduct(CameraRight, ToEnemy);
	float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(DotRight, DotForward));

	// 0~360도 정규화 (0=위, 90=오른쪽, 180=아래, 270=왼쪽)
	if (AngleDeg < 0.0f)
	{
		AngleDeg += 360.0f;
	}

	return AngleDeg;
}

bool UPREnemyIndicatorViewModel::IsOnScreen(APlayerController* PC, const FVector& EnemyLocation) const
{
	FVector2D ScreenPosition;
	const bool bProjected = PC->ProjectWorldLocationToScreen(EnemyLocation, ScreenPosition);
	if (!bProjected)
	{
		return false;
	}

	int32 ViewportX = 0;
	int32 ViewportY = 0;
	PC->GetViewportSize(ViewportX, ViewportY);
	if (ViewportX <= 0 || ViewportY <= 0)
	{
		return false;
	}

	// 스크린 좌표가 뷰포트 범위 내인지 확인
	return ScreenPosition.X >= 0.0f && ScreenPosition.X <= static_cast<float>(ViewportX)
		&& ScreenPosition.Y >= 0.0f && ScreenPosition.Y <= static_cast<float>(ViewportY);
}
