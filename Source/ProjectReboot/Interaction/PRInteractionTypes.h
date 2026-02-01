// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRInteractionTypes.generated.h"

class UTexture2D;

/**
 * 상호작용 UI 표시 정보
 */
USTRUCT(BlueprintType)
struct FPRInteractionInfo
{
	GENERATED_BODY()

	// 표시 텍스트 (예: 줍기, 장착)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText DisplayText;

	// 아이콘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	TObjectPtr<UTexture2D> Icon = nullptr;

	// 상호작용 가능 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bIsEnabled = false;

};