// PRCrosshairWidget.cpp
#include "PRCrosshairWidget.h"

#include "PRCrosshairViewModel.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "ProjectReboot/Crosshair/PRCrosshairConfig.h"
#include "ProjectReboot/Crosshair/PRCrosshairStyle.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"

void UPRCrosshairWidget::NativeConstruct()
{
    Super::NativeConstruct();
    BindViewModel();
}

void UPRCrosshairWidget::NativeDestruct()
{
    UnbindViewModel();
    Super::NativeDestruct();
}

void UPRCrosshairWidget::BindViewModel()
{
    ULocalPlayer* LP = GetOwningLocalPlayer();
    if (!LP)
    {
        return;
    }

    UPRViewModelSubsystem* VMS = LP->GetSubsystem<UPRViewModelSubsystem>();
    if (!VMS)
    {
        return;
    }

    ViewModel = VMS->GetGlobalViewModel<UPRCrosshairViewModel>();
    if (!ViewModel)
    {
        return;
    }

    ViewModel->OnCanFireChanged.AddDynamic(this, &UPRCrosshairWidget::HandleCanFireChanged);
    ViewModel->OnVisibilityChanged.AddDynamic(this, &UPRCrosshairWidget::HandleVisibilityChanged);
    ViewModel->OnTargetingEnemyChanged.AddDynamic(this, &UPRCrosshairWidget::HandleTargetingEnemyChanged);
    ViewModel->OnCrosshairTagChanged.AddDynamic(this, &UPRCrosshairWidget::HandleCrosshairTagChanged);
    ViewModel->OnHitMarkerTriggered.AddDynamic(this, &UPRCrosshairWidget::HandleHitMarkerTriggered);

    ApplyInitialState();
}

void UPRCrosshairWidget::UnbindViewModel()
{
    if (!ViewModel)
    {
        return;
    }

    ViewModel->OnCanFireChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleCanFireChanged);
    ViewModel->OnVisibilityChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleVisibilityChanged);
    ViewModel->OnTargetingEnemyChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleTargetingEnemyChanged);
    ViewModel->OnCrosshairTagChanged.RemoveDynamic(this, &UPRCrosshairWidget::HandleCrosshairTagChanged);
    ViewModel->OnHitMarkerTriggered.RemoveDynamic(this, &UPRCrosshairWidget::HandleHitMarkerTriggered);

    ViewModel = nullptr;
}

void UPRCrosshairWidget::ApplyInitialState()
{
    if (!ViewModel)
    {
        return;
    }

    const FPRCrosshairSetting& Setting = ViewModel->GetCurrentSetting();
    bCanFire = ViewModel->CanFire();
    bIsTargetingEnemy = ViewModel->IsTargetingEnemy();

    ApplyStyle(Setting.Style);
    HandleVisibilityChanged(ViewModel->IsVisible());
}

void UPRCrosshairWidget::HandleCanFireChanged(bool bNewCanFire)
{
    bCanFire = bNewCanFire;
    UpdateCrosshairColor();
}

void UPRCrosshairWidget::HandleVisibilityChanged(bool bVisible)
{
    SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UPRCrosshairWidget::HandleTargetingEnemyChanged(bool bTargeting)
{
    bIsTargetingEnemy = bTargeting;
    UpdateCrosshairColor();
}

void UPRCrosshairWidget::HandleCrosshairTagChanged(const FGameplayTag& NewTag)
{
    if (!ViewModel)
    {
        return;
    }

    const FPRCrosshairSetting& Setting = ViewModel->GetCurrentSetting();
    ApplyStyle(Setting.Style);
}

void UPRCrosshairWidget::HandleHitMarkerTriggered(EPRHitMarkerType HitMarkerType)
{
    OnPlayHitMarkerAnimation(HitMarkerType);
}

void UPRCrosshairWidget::ApplyStyle(UPRCrosshairStyle* Style)
{
    CurrentStyle = Style;

    if (!Style)
    {
        if (CrosshairTop) CrosshairTop->SetVisibility(ESlateVisibility::Collapsed);
        if (CrosshairBottom) CrosshairBottom->SetVisibility(ESlateVisibility::Collapsed);
        if (CrosshairLeft) CrosshairLeft->SetVisibility(ESlateVisibility::Collapsed);
        if (CrosshairRight) CrosshairRight->SetVisibility(ESlateVisibility::Collapsed);
        if (CrosshairCenter) CrosshairCenter->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    // Top
    if (CrosshairTop)
    {
        if (Style->TopTexture)
        {
            CrosshairTop->SetBrushFromTexture(Style->TopTexture);
            CrosshairTop->SetDesiredSizeOverride(Style->ElementSize);
            CrosshairTop->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairTop->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Bottom
    if (CrosshairBottom)
    {
        if (Style->BottomTexture)
        {
            CrosshairBottom->SetBrushFromTexture(Style->BottomTexture);
            CrosshairBottom->SetDesiredSizeOverride(Style->ElementSize);
            CrosshairBottom->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairBottom->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Left
    if (CrosshairLeft)
    {
        if (Style->LeftTexture)
        {
            CrosshairLeft->SetBrushFromTexture(Style->LeftTexture);
            CrosshairLeft->SetDesiredSizeOverride(FVector2D(Style->ElementSize.Y, Style->ElementSize.X));
            CrosshairLeft->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairLeft->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Right
    if (CrosshairRight)
    {
        if (Style->RightTexture)
        {
            CrosshairRight->SetBrushFromTexture(Style->RightTexture);
            CrosshairRight->SetDesiredSizeOverride(FVector2D(Style->ElementSize.Y, Style->ElementSize.X));
            CrosshairRight->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairRight->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    // Center
    if (CrosshairCenter)
    {
        if (Style->bShowCenter && Style->CenterTexture)
        {
            CrosshairCenter->SetBrushFromTexture(Style->CenterTexture);
            CrosshairCenter->SetDesiredSizeOverride(Style->CenterSize);
            CrosshairCenter->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CrosshairCenter->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    UpdateCrosshairColor();
}

void UPRCrosshairWidget::UpdateCrosshairColor()
{
    if (!CurrentStyle)
    {
        return;
    }

    FLinearColor TargetColor = CurrentStyle->DefaultColor;

    if (!bCanFire)
    {
        TargetColor = CurrentStyle->CannotFireColor;
    }
    else if (bIsTargetingEnemy)
    {
        TargetColor = CurrentStyle->EnemyTargetColor;
    }

    SetCrosshairElementsColor(TargetColor);
}

void UPRCrosshairWidget::SetCrosshairElementsColor(const FLinearColor& Color)
{
    for (UImage* Element : GetCrosshairElements())
    {
        if (Element)
        {
            Element->SetColorAndOpacity(Color);
        }
    }
}

TArray<UImage*> UPRCrosshairWidget::GetCrosshairElements() const
{
    return { CrosshairTop, CrosshairBottom, CrosshairLeft, CrosshairRight, CrosshairCenter };
}
