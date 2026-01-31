// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PRCameraBlueprintLibrary.generated.h"

class UPRActorFocusSubsystem;
struct FActorFocusParams;

/**
 * 카메라 관련 블루프린트 라이브러리
 */
UCLASS()
class PROJECTREBOOT_API UPRCameraBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*~ 액터 포커스 ~*/

	// ActorFocus 서브시스템 조회
	UFUNCTION(BlueprintCallable, Category = "ActorFocus")
	static UPRActorFocusSubsystem* GetActorFocusSubsystem(APlayerController* OwningPlayer);

	// 액터에 포커스
	UFUNCTION(BlueprintCallable, Category = "ActorFocus")
	static bool FocusOnActor(APlayerController* OwningPlayer, AActor* TargetActor, const FActorFocusParams& Params);

	// 포커스 해제
	UFUNCTION(BlueprintCallable, Category = "ActorFocus")
	static void RestoreFocus(APlayerController* OwningPlayer);

	// 포커스 중인지 확인
	UFUNCTION(BlueprintPure, Category = "ActorFocus")
	static bool IsFocusing(APlayerController* OwningPlayer);
};