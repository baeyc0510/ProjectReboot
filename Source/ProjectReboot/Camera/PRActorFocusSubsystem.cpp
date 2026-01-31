// Fill out your copyright notice in the Description page of Project Settings.

#include "PRActorFocusSubsystem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/PRGameplayTags.h"

void UPRActorFocusSubsystem::Deinitialize()
{
	if (IsFocusing())
	{
		RestoreFocus();
	}

	Super::Deinitialize();
}

/*~ 포커스 관리 ~*/

bool UPRActorFocusSubsystem::FocusOnActor(AActor* TargetActor, const FActorFocusParams& Params)
{
	if (!IsValid(TargetActor))
	{
		return false;
	}

	// 이미 포커스 중이면 먼저 복원
	if (IsFocusing())
	{
		RestoreFocus();
	}

	// 상태 초기화
	CurrentFocusState = FActorFocusState();
	CurrentFocusState.TargetActor = TargetActor;
	CurrentFocusState.UsedParams = Params;

	// 카메라 설정 및 활성화
	SetupFocusCamera(TargetActor, Params);
	ActivateFocusCamera(Params);

	// 플레이어 상태 잠금
	if (Params.bLockPlayerRotation)
	{
		LockPlayerState(TargetActor, Params);
	}

	// 크로스헤어 숨김
	if (Params.bHideCrosshair)
	{
		SetCrosshairVisibility(TargetActor, false);
		CurrentFocusState.bCrosshairHidden = true;
	}

	return true;
}

void UPRActorFocusSubsystem::RestoreFocus()
{
	if (!IsFocusing())
	{
		return;
	}

	// UI 상태 복원
	if (CurrentFocusState.bCrosshairHidden && CurrentFocusState.TargetActor.IsValid())
	{
		SetCrosshairVisibility(CurrentFocusState.TargetActor.Get(), true);
	}

	// 플레이어 상태 복원
	UnlockPlayerState();

	// 카메라 복원
	RestoreCamera();
	CleanupFocusCamera();

	// 상태 초기화
	CurrentFocusState = FActorFocusState();
}

bool UPRActorFocusSubsystem::IsFocusing() const
{
	return CurrentFocusState.TargetActor.IsValid() &&
		   IsValid(CurrentFocusState.FocusCameraActor);
}

AActor* UPRActorFocusSubsystem::GetFocusedActor() const
{
	return CurrentFocusState.TargetActor.Get();
}

void UPRActorFocusSubsystem::SetupFocusCamera(AActor* TargetActor, const FActorFocusParams& Params)
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(TargetActor))
	{
		return;
	}

	// 카메라 액터 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CurrentFocusState.FocusCameraActor = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), SpawnParams);
	if (!CurrentFocusState.FocusCameraActor)
	{
		return;
	}

	// 타겟 정면 + 측면 오프셋 위치에 카메라 배치
	FVector TargetLocation = TargetActor->GetActorLocation();
	FRotator TargetRotation = TargetActor->GetActorRotation();

	// 카메라 방향: 타겟 정면 + Yaw 오프셋
	FRotator CameraDirection = TargetRotation;
	CameraDirection.Yaw += Params.CameraYawOffset;

	FVector CameraOffset = CameraDirection.Vector() * Params.CameraDistance;
	CameraOffset.Z = Params.CameraHeightOffset;

	FVector CameraLocation = TargetLocation + CameraOffset;

	// 화면 좌우 오프셋 적용
	const FVector ScreenOffset = TargetRotation.Quaternion().GetRightVector() * Params.ScreenHorizontalOffset;
	const FVector LookAtTarget = TargetLocation + FVector(0, 0, Params.CameraHeightOffset * 0.5f) + ScreenOffset;
	FRotator CameraRotation = (LookAtTarget - CameraLocation).Rotation();

	CurrentFocusState.FocusCameraActor->SetActorLocationAndRotation(CameraLocation, CameraRotation);
}

void UPRActorFocusSubsystem::ActivateFocusCamera(const FActorFocusParams& Params)
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	APlayerController* PC = LP->GetPlayerController(GetWorld());
	if (!PC || !CurrentFocusState.FocusCameraActor)
	{
		return;
	}

	CurrentFocusState.OriginalViewTarget = PC->GetViewTarget();
	PC->SetViewTargetWithBlend(CurrentFocusState.FocusCameraActor, Params.BlendTime, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
}

void UPRActorFocusSubsystem::RestoreCamera()
{
	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	APlayerController* PC = LP->GetPlayerController(GetWorld());
	if (!PC)
	{
		return;
	}

	if (CurrentFocusState.OriginalViewTarget.IsValid())
	{
		PC->SetViewTargetWithBlend(CurrentFocusState.OriginalViewTarget.Get(), CurrentFocusState.UsedParams.BlendTime, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}
}

void UPRActorFocusSubsystem::CleanupFocusCamera(bool bImmediate)
{
	if (!IsValid(CurrentFocusState.FocusCameraActor))
	{
		return;
	}

	if (!bImmediate && CurrentFocusState.UsedParams.BlendTime > 0.0f)
	{
		CurrentFocusState.FocusCameraActor->SetLifeSpan(CurrentFocusState.UsedParams.BlendTime);
		CurrentFocusState.FocusCameraActor = nullptr;
		return;
	}

	CurrentFocusState.FocusCameraActor->Destroy();
	CurrentFocusState.FocusCameraActor = nullptr;
}

void UPRActorFocusSubsystem::LockPlayerState(AActor* TargetActor, const FActorFocusParams& Params)
{
	if (!IsValid(TargetActor))
	{
		return;
	}

	CurrentFocusState.OriginalActorRotation = TargetActor->GetActorRotation();

	if (ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			CurrentFocusState.bOriginalOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
			MovementComp->bOrientRotationToMovement = false;
		}
	}

	// 카메라를 향해 캐릭터 회전
	if (CurrentFocusState.FocusCameraActor)
	{
		FVector ToCamera = CurrentFocusState.FocusCameraActor->GetActorLocation() - TargetActor->GetActorLocation();
		ToCamera.Z = 0;
		FRotator LookAtRotation = ToCamera.Rotation();
		TargetActor->SetActorRotation(LookAtRotation);
	}
}

void UPRActorFocusSubsystem::UnlockPlayerState()
{
	if (!CurrentFocusState.TargetActor.IsValid())
	{
		return;
	}

	AActor* TargetActor = CurrentFocusState.TargetActor.Get();

	if (ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->bOrientRotationToMovement = CurrentFocusState.bOriginalOrientRotationToMovement;
		}
	}
}

void UPRActorFocusSubsystem::SetCrosshairVisibility(AActor* TargetActor, bool bVisible)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!ASC)
	{
		return;
	}

	if (bVisible)
	{
		ASC->RemoveLooseGameplayTag(TAG_State_UI_HideCrosshair);
	}
	else
	{
		ASC->AddLooseGameplayTag(TAG_State_UI_HideCrosshair);
	}
}