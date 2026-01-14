// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRActionListItemWidget.generated.h"

class URogueliteActionData;
class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UPRActionListItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Roguelite")
	void InitWidget(URogueliteActionData* InActionData);

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TB_ActionName;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TB_Description;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<URogueliteActionData> ActionData;
};