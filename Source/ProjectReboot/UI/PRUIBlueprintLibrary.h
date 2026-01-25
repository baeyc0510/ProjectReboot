// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PRUIBlueprintLibrary.generated.h"

class UPRUIManagerSubsystem;
class UPRViewModelBase;
class UPRViewModelSubsystem;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRUIBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/*~ UI Manager ~*/
	
	// UI를 스택에 푸시
	UFUNCTION(BlueprintCallable, Category= "UI", meta = (DeterminesOutputType = "WidgetClass"))
	static UUserWidget* PushUI(APlayerController* OwningPlayer, TSubclassOf<UUserWidget> WidgetClass);
	
	// UI를 스택에서 팝
	UFUNCTION(BlueprintCallable, Category= "UI")
	static void PopUI(APlayerController* OwningPlayer, UUserWidget* WidgetInstance = nullptr);
	
	// UI 매니저 서브시스템 조회
	UFUNCTION(BlueprintCallable, Category= "UI")
	static UPRUIManagerSubsystem* GetUIManager(APlayerController* OwningPlayer);

	/*~ ViewModel ~*/

	// ViewModel 서브시스템 조회
	UFUNCTION(BlueprintCallable, Category= "ViewModel")
	static UPRViewModelSubsystem* GetViewModelSubsystem(APlayerController* OwningPlayer);

	// Global ViewModel 등록
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Global", meta = (DeterminesOutputType = "ViewModelClass"))
	static UPRViewModelBase* RegisterGlobalViewModel(APlayerController* OwningPlayer, TSubclassOf<UPRViewModelBase> ViewModelClass);

	// Global ViewModel 조회
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Global", meta = (DeterminesOutputType = "ViewModelClass"))
	static UPRViewModelBase* GetGlobalViewModel(APlayerController* OwningPlayer, TSubclassOf<UPRViewModelBase> ViewModelClass);

	// Global ViewModel 조회 또는 생성
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Global", meta = (DeterminesOutputType = "ViewModelClass"))
	static UPRViewModelBase* GetOrCreateGlobalViewModel(APlayerController* OwningPlayer, TSubclassOf<UPRViewModelBase> ViewModelClass);

	// Global ViewModel 등록 해제
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Global")
	static bool UnregisterGlobalViewModel(APlayerController* OwningPlayer, TSubclassOf<UPRViewModelBase> ViewModelClass);

	// Actor-Bound ViewModel 조회 또는 생성
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Actor", meta = (DeterminesOutputType = "ViewModelClass"))
	static UPRViewModelBase* GetOrCreateActorViewModel(APlayerController* OwningPlayer, AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass);

	// Actor-Bound ViewModel 조회
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Actor", meta = (DeterminesOutputType = "ViewModelClass"))
	static UPRViewModelBase* FindActorViewModel(APlayerController* OwningPlayer, AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass);

	// Actor-Bound ViewModel 제거
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Actor")
	static bool RemoveActorViewModel(APlayerController* OwningPlayer, AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass);

	// Actor-Bound ViewModel 전체 제거
	UFUNCTION(BlueprintCallable, Category = "ViewModel|Actor")
	static void RemoveAllViewModelsForActor(APlayerController* OwningPlayer, AActor* TargetActor);
};
