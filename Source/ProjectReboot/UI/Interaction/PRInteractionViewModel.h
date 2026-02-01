// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/Interaction/PRInteractionTypes.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelBase.h"
#include "PRInteractionViewModel.generated.h"

// ViewModel 변경 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractionViewModelUpdated);
// 활성 여부 변경 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionEnabledChanged, bool, bEnabled);
// 텍스트/아이콘 변경 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionContentChanged, const FText&, DisplayText, UTexture2D*, Icon);

/**
 * 상호작용 UI ViewModel
 */
UCLASS(BlueprintType)
class PROJECTREBOOT_API UPRInteractionViewModel : public UPRViewModelBase
{
	GENERATED_BODY()

public:
	// 생성자
	UPRInteractionViewModel();

	/*~ 상호작용 상태 ~*/

	// 상호작용 정보 갱신
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionInfo(const FPRInteractionInfo& InInfo, bool bVisible);

	// 상호작용 정보 초기화
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void ClearInteractionInfo();

	/*~ Getter ~*/

	// 표시 텍스트 반환
	UFUNCTION(BlueprintPure, Category = "Interaction")
	const FText& GetDisplayText() const { return DisplayText; }

	// 아이콘 반환
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UTexture2D* GetIcon() const { return Icon; }

	// 상호작용 가능 여부 반환
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsEnabled() const { return bIsEnabled; }

public:
	/*~ Delegates ~*/

	// ViewModel 상태 갱신 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionViewModelUpdated OnViewModelUpdated;

	// 활성 여부 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionEnabledChanged OnEnabledChanged;

	// 텍스트/아이콘 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
	FOnInteractionContentChanged OnContentChanged;

	// 표시 텍스트
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	FText DisplayText;

	// 아이콘
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	TObjectPtr<UTexture2D> Icon = nullptr;

	// 상호작용 가능 여부
	UPROPERTY(BlueprintReadOnly, Category = "Interaction")
	bool bIsEnabled = false;

};