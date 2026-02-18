#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PRWeaponPartIconWidget.generated.h"

class UImage;
class UTexture2D;

/**
 * 무기 부품 아이콘 위젯
 * 테두리 등 디자인은 BP에서 구성
 */
UCLASS()
class PROJECTREBOOT_API UPRWeaponPartIconWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 아이콘 텍스처 설정
	UFUNCTION(BlueprintCallable, Category = "PartIcon")
	void SetIcon(UTexture2D* InTexture);

protected:
	// 아이콘 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;
};
