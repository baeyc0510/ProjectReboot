// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PRActorFocusSubsystem.generated.h"

class ACameraActor;

/**
 * 액터 포커스 파라미터
 */
USTRUCT(BlueprintType)
struct FActorFocusParams
{
	GENERATED_BODY()

	/*~ 카메라 파라미터 ~*/

	// 타겟 액터로부터 카메라까지의 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraDistance = 250.0f;

	// 카메라 높이 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraHeightOffset = 60.0f;

	// 정면에서의 측면 각도 오프셋 (Yaw)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraYawOffset = 25.0f;

	// 카메라 전환 블렌딩 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BlendTime = 0.4f;

	// 화면상 좌우 오프셋 (양수 = 타겟 기준 우측으로 카메라 이동)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ScreenHorizontalOffset = -100.0f;

	/*~ 플레이어 상태 파라미터 ~*/

	// 플레이어 회전을 카메라 방향으로 잠금할지 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	bool bLockPlayerRotation = true;

	// 크로스헤어 숨김 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	bool bHideCrosshair = true;
};

/**
 * 포커스 상태 저장 (복원용)
 */
USTRUCT()
struct FActorFocusState
{
	GENERATED_BODY()

	// 포커스 중인 타겟 액터
	TWeakObjectPtr<AActor> TargetActor;

	// 원래 뷰 타겟
	TWeakObjectPtr<AActor> OriginalViewTarget;

	// 생성된 포커스 카메라
	UPROPERTY()
	TObjectPtr<ACameraActor> FocusCameraActor;

	// 원래 액터 회전
	FRotator OriginalActorRotation;

	// 원래 OrientRotationToMovement 설정
	bool bOriginalOrientRotationToMovement = false;

	// 크로스헤어 숨김 적용 여부
	bool bCrosshairHidden = false;

	// 사용된 파라미터 (복원 시 BlendTime 등 사용)
	FActorFocusParams UsedParams;
};

/**
 * 액터 포커스 서브시스템
 * - 특정 액터에 카메라 포커스
 * - 플레이어 상태 관리 (회전 잠금)
 * - UI 상태 관리 (크로스헤어 숨김)
 */
UCLASS()
class PROJECTREBOOT_API UPRActorFocusSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	
	// 특정 액터에 포커스
	UFUNCTION(BlueprintCallable, Category = "ActorFocus")
	bool FocusOnActor(AActor* TargetActor, const FActorFocusParams& Params);

	// 포커스 해제 및 원래 상태로 복원
	UFUNCTION(BlueprintCallable, Category = "ActorFocus")
	void RestoreFocus();

	// 현재 포커스 중인지 확인
	UFUNCTION(BlueprintPure, Category = "ActorFocus")
	bool IsFocusing() const;

	// 현재 포커스 중인 액터 반환
	UFUNCTION(BlueprintPure, Category = "ActorFocus")
	AActor* GetFocusedActor() const;

protected:
	// 포커스 카메라 생성 및 위치 설정
	void SetupFocusCamera(AActor* TargetActor, const FActorFocusParams& Params);

	// 포커스 카메라로 뷰 타겟 전환
	void ActivateFocusCamera(const FActorFocusParams& Params);

	// 원래 카메라로 복원
	void RestoreCamera();

	// 포커스 카메라 정리
	void CleanupFocusCamera(bool bImmediate = false);

	// 플레이어 상태 잠금 (회전 고정 + 카메라 향해 회전)
	void LockPlayerState(AActor* TargetActor, const FActorFocusParams& Params);

	// 플레이어 상태 잠금 해제
	void UnlockPlayerState();

	// 크로스헤어 등 UI 가시성 제어
	void SetCrosshairVisibility(AActor* TargetActor, bool bVisible);

private:
	// 현재 포커스 상태
	FActorFocusState CurrentFocusState;
};