#include "PRWeaponPartIconWidget.h"
#include "Components/Image.h"

void UPRWeaponPartIconWidget::SetIcon(UTexture2D* InTexture)
{
	if (!IsValid(IconImage))
	{
		return;
	}

	if (IsValid(InTexture))
	{
		IconImage->SetBrushFromTexture(InTexture);
		IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		IconImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}
