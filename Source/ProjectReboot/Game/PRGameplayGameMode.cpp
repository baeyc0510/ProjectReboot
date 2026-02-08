// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameplayGameMode.h"
#include "PRGameplayGameState.h"
#include "StateTree.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Room/PRRoomWorldSubsystem.h"
#include "ProjectReboot/Room/PRRoomController.h"
#include "ProjectReboot/Room/PRStageManagerSubsystem.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"
#include "RogueliteSubsystem.h"
#include "GameFramework/Character.h"

void APRGameplayGameMode::OnGameplayEvent(const FGameplayTag& EventTag, int32 Delta)
{
	APRGameplayGameState* GS = GetGameState<APRGameplayGameState>();
	if (!GS)
	{
		return;
	}

	GS->AddEventCount(EventTag, Delta);

	// StateTree에서 클리어 조건을 판정하므로 여기서는 카운트만 증가
}

void APRGameplayGameMode::MoveToNextRoom(int32 TargetRoomIndex)
{
	// 첫 방 진입이거나 외부에서 직접 호출 시 페이드 처리
	StartFadeOutAndMoveToRoom(TargetRoomIndex);
}

void APRGameplayGameMode::StartFadeOutAndMoveToRoom(int32 TargetRoomIndex)
{
	PendingRoomIndex = TargetRoomIndex;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		// 컨트롤러 없으면 페이드 없이 바로 이동
		OnFadeOutComplete();
		return;
	}

	// 첫 진입이 아닌 경우에만 페이드 아웃
	if (!bIsFirstRoomEntry && IsValid(PC->PlayerCameraManager))
	{
		// 입력 차단
		PC->DisableInput(PC);

		// HUD 숨김
		if (UPRViewModelSubsystem* ViewModelSubsystem = ULocalPlayer::GetSubsystem<UPRViewModelSubsystem>(PC->GetLocalPlayer()))
		{
			ViewModelSubsystem->SetVisibilityByTag(TAG_UI_ViewModel_HUD, false);
		}

		// 페이드 아웃 시작
		PC->PlayerCameraManager->StartCameraFade(0.f, 1.f, FadeDuration, FLinearColor::Black, false, true);

		// 페이드 완료 후 방 이동
		GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &APRGameplayGameMode::OnFadeOutComplete, FadeDuration, false);
	}
	else
	{
		// 첫 진입 시 즉시 검은 화면 설정
		if (IsValid(PC->PlayerCameraManager))
		{
			PC->PlayerCameraManager->SetManualCameraFade(1.f, FLinearColor::Black, false);
		}
		PC->DisableInput(PC);

		// HUD 숨김 (첫 진입도 동일 처리)
		if (UPRViewModelSubsystem* ViewModelSubsystem = ULocalPlayer::GetSubsystem<UPRViewModelSubsystem>(PC->GetLocalPlayer()))
		{
			ViewModelSubsystem->SetVisibilityByTag(TAG_UI_ViewModel_HUD, false);
		}

		OnFadeOutComplete();
	}
}

void APRGameplayGameMode::OnFadeOutComplete()
{
	if (PendingRoomIndex == INDEX_NONE)
	{
		return;
	}

	// 방 준비 완료 시 OnNextRoomReady 호출 등록
	if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
	{
		RoomSubsystem->CallOrRegister_RoomReady(PendingRoomIndex,
			FOnRoomReadySignature::FDelegate::CreateUObject(this, &APRGameplayGameMode::OnNextRoomReady));
	}
	
	// 방 진입 시작 (레벨 로드)
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		StageManager->EnterRoomByIndex(PendingRoomIndex);
	}
}

void APRGameplayGameMode::StartPrewarmPhase()
{
	// 프리웜 델리게이트가 바인딩되어 있으면 실행
	if (OnRoomPrewarm.IsBound())
	{
		// 바인딩된 시스템들에게 프리웜 시작 알림 (동기적 실행)
		OnRoomPrewarm.Broadcast();

		// 모든 핸들러 처리 후 바인딩 해제
		OnRoomPrewarm.Clear();
	}

	// 최소 프리웜 시간 보장
	if (MinPrewarmDuration > 0.f)
	{
		FTimerHandle PrewarmTimerHandle;
		GetWorldTimerManager().SetTimer(PrewarmTimerHandle, this, &APRGameplayGameMode::OnPrewarmComplete, MinPrewarmDuration, false);
	}
	else
	{
		// 최소 시간 없으면 즉시 완료
		OnPrewarmComplete();
	}
}

void APRGameplayGameMode::OnPrewarmComplete()
{
	// 프리웜 완료 후 텔레포트만 수행 (AI는 이미 스폰됨)
	if (IsValid(PendingRoomController))
	{
		// 플레이어를 해당 방으로 텔레포트
		if (UPRRoomWorldSubsystem* RoomSubsystem = GetWorld()->GetSubsystem<UPRRoomWorldSubsystem>())
		{
			RoomSubsystem->TeleportAllPlayersToRoom(PendingRoomController->GetRoomIndex());
		}
		
		PendingRoomController->ActivateRoom();

		UE_LOG(LogTemp, Log, TEXT("APRGameplayGameMode: Player teleported to room %d after prewarm"), PendingRoomController->GetRoomIndex());
		PendingRoomController = nullptr;
	}

	ExecuteFadeIn();
}

void APRGameplayGameMode::ExecuteFadeIn()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		return;
	}

	if (IsValid(PC->PlayerCameraManager))
	{
		// 페이드 인
		PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, FadeDuration, FLinearColor::Black, false, false);
	}

	// 입력 복원
	PC->EnableInput(PC);

	// HUD 표시
	if (UPRViewModelSubsystem* ViewModelSubsystem = ULocalPlayer::GetSubsystem<UPRViewModelSubsystem>(PC->GetLocalPlayer()))
	{
		ViewModelSubsystem->RestoreVisibilityByTag(TAG_UI_ViewModel_HUD, true);
	}

	// 첫 진입 플래그 해제
	bIsFirstRoomEntry = false;

	UE_LOG(LogTemp, Log, TEXT("APRGameplayGameMode: Prewarm complete, fade in started"));
}

void APRGameplayGameMode::BeginPlay()
{
	// 스테이지 설정 로드 및 등록
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		// AssetType이 설정되지 않았으면 기본값 사용
		FPrimaryAssetType AssetType = StageConfigAssetType.IsValid() ? StageConfigAssetType : FPrimaryAssetType(TEXT("PRStageConfig"));
		StageManager->LoadAndRegisterStageConfigs(AssetType);

		// 스테이지 진행 로드 (Seamless Travel 후 복구)
		StageManager->LoadStageProgress();
	}
	
	Super::BeginPlay();

	// 런 시작 (RoomGraph 생성)
	if (URogueliteSubsystem* RogueliteSubsystem = URogueliteSubsystem::Get(this))
	{
		if (!RogueliteSubsystem->IsRunActive())
		{
			RogueliteSubsystem->StartRun();	
		}
	}
	
	MoveToNextRoom (0);
}

void APRGameplayGameMode::OnRoomCleared()
{
	// 방 클리어 알림
	if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
	{
		GS->SendRoomEvent(TAG_Event_Room_Clear);
	}
	if (UPRStageManagerSubsystem* StageManager = UPRStageManagerSubsystem::Get(this))
	{
		StageManager->OnRoomCleared();
	}

	UE_LOG(LogTemp, Log, TEXT("APRGameplayGameMode: Room cleared!"));
}

void APRGameplayGameMode::OnNextRoomReady(APRRoomController* RoomController, const FPRRoomNodeInfo& InNodeInfo)
{
	CurrentNodeInfo = InNodeInfo;

	// NodeInfo의 Config가 비어있으면 DefaultRoomConfig 사용
	if (!IsValid(CurrentNodeInfo.FlowConfig.StateTree) && IsValid(DefaultRoomConfig.StateTree))
	{
		CurrentNodeInfo.FlowConfig.StateTree = DefaultRoomConfig.StateTree;
	}

	if (APRGameplayGameState* GS = GetGameState<APRGameplayGameState>())
	{
		GS->NotifyRoomEnter(RoomController);
	}

	if (IsValid(RoomController))
	{
		// RoomController 저장 (프리웜 완료 후 텔레포트에 사용)
		PendingRoomController = RoomController;

		// 프리웜 동안 AI가 플레이어를 감지하지 못하도록 먼 곳으로 이동
		if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
		{
			Player->SetActorLocation(FVector(0.f, 0.f, -50000.f));
		}

		// 방 초기화
		RoomController->InitRoom(CurrentNodeInfo);

		UE_LOG(LogTemp, Log, TEXT("APRGameplayGameMode: Room %d activated, starting prewarm"), RoomController->GetRoomIndex());

		// 프리웜 페이즈 시작 (완료 후 텔레포트/페이드 인)
		StartPrewarmPhase();
	}
}
