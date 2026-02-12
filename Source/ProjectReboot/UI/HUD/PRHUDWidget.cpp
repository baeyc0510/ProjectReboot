#include "PRHUDWidget.h"
#include "PRHUDViewModel.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ProjectReboot/UI/Interfaces/PRProgressBarInterface.h"

void UPRHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindViewModel();
}

void UPRHUDWidget::NativeDestruct()
{
	UnbindViewModel();
	Super::NativeDestruct();
}

void UPRHUDWidget::BindViewModel()
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

	ViewModel = VMS->GetGlobalViewModel<UPRHUDViewModel>();
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnAmmoChanged.AddDynamic(this, &UPRHUDWidget::HandleAmmoChanged);
	ViewModel->OnReserveAmmoChanged.AddDynamic(this, &UPRHUDWidget::HandleReserveAmmoChanged);
	ViewModel->OnWeaponTypeChanged.AddDynamic(this, &UPRHUDWidget::HandleWeaponTypeChanged);
	ViewModel->OnHealthChanged.AddDynamic(this, &UPRHUDWidget::HandleHealthChanged);
	ViewModel->OnShieldChanged.AddDynamic(this, &UPRHUDWidget::HandleShieldChanged);
	ViewModel->OnHealthSegmentChanged.AddDynamic(this, &UPRHUDWidget::HandleHealthSegmentChanged);
	ViewModel->OnShieldSegmentChanged.AddDynamic(this, &UPRHUDWidget::HandleShieldSegmentChanged);
	ViewModel->OnVisibilityChanged.AddDynamic(this, &UPRHUDWidget::HandleVisibilityChanged);

	ApplyInitialState();
}

void UPRHUDWidget::UnbindViewModel()
{
	if (!ViewModel)
	{
		return;
	}

	ViewModel->OnAmmoChanged.RemoveDynamic(this, &UPRHUDWidget::HandleAmmoChanged);
	ViewModel->OnReserveAmmoChanged.RemoveDynamic(this, &UPRHUDWidget::HandleReserveAmmoChanged);
	ViewModel->OnWeaponTypeChanged.RemoveDynamic(this, &UPRHUDWidget::HandleWeaponTypeChanged);
	ViewModel->OnHealthChanged.RemoveDynamic(this, &UPRHUDWidget::HandleHealthChanged);
	ViewModel->OnShieldChanged.RemoveDynamic(this, &UPRHUDWidget::HandleShieldChanged);
	ViewModel->OnHealthSegmentChanged.RemoveDynamic(this, &UPRHUDWidget::HandleHealthSegmentChanged);
	ViewModel->OnShieldSegmentChanged.RemoveDynamic(this, &UPRHUDWidget::HandleShieldSegmentChanged);
	ViewModel->OnVisibilityChanged.RemoveDynamic(this, &UPRHUDWidget::HandleVisibilityChanged);

	ViewModel = nullptr;
}

void UPRHUDWidget::ApplyInitialState()
{
	if (!ViewModel)
	{
		return;
	}

	// 초기 상태 동기화
	HandleAmmoChanged(ViewModel->GetCurrentAmmo(), ViewModel->GetMaxAmmo());
	HandleReserveAmmoChanged(ViewModel->GetCurrentReserveAmmo(), ViewModel->GetMaxReserveAmmo());
	HandleWeaponTypeChanged(ViewModel->GetWeaponTypeTag());
	HandleHealthChanged(ViewModel->GetCurrentHealth(), ViewModel->GetMaxHealth());
	HandleShieldChanged(ViewModel->GetCurrentShield(), ViewModel->GetMaxShield());
	HandleHealthSegmentChanged(ViewModel->GetHealthNumSegments(), ViewModel->GetHealthSpacing());
	HandleShieldSegmentChanged(ViewModel->GetShieldNumSegments(), ViewModel->GetShieldSpacing());
	HandleVisibilityChanged(ViewModel->IsVisible());
}

void UPRHUDWidget::HandleVisibilityChanged(bool bVisible)
{
	// HUD 전체 가시성 적용
	SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}

void UPRHUDWidget::HandleAmmoChanged(int32 Current, int32 Max)
{
	if (AmmoText)
	{
		AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), Current, Max)));
	}
}

void UPRHUDWidget::HandleReserveAmmoChanged(int32 Current, int32 Max)
{
	if (ReserveAmmoText)
	{
		ReserveAmmoText->SetText(FText::AsNumber(Current));
	}

	if (MaxReserveAmmoText)
	{
		if (Max > 0)
		{
			MaxReserveAmmoText->SetText(FText::AsNumber(Max));
			MaxReserveAmmoText->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			MaxReserveAmmoText->SetText(FText::GetEmpty());
			MaxReserveAmmoText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UPRHUDWidget::HandleWeaponTypeChanged(const FGameplayTag& NewTag)
{
	if (!WeaponIcon)
	{
		return;
	}

	if (TObjectPtr<UTexture2D>* Icon = WeaponIconMap.Find(NewTag))
	{
		WeaponIcon->SetBrushFromTexture(*Icon);
		WeaponIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		WeaponIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPRHUDWidget::HandleHealthChanged(float Current, float Max)
{
	if (IsValid(HealthBar) && HealthBar->Implements<UPRProgressBarInterface>())
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		IPRProgressBarInterface::Execute_SetPercent(HealthBar, Percent);
	}
}

void UPRHUDWidget::HandleShieldChanged(float Current, float Max)
{
	if (IsValid(ShieldBar) && ShieldBar->Implements<UPRProgressBarInterface>())
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		IPRProgressBarInterface::Execute_SetPercent(ShieldBar, Percent);
	}
}

void UPRHUDWidget::HandleHealthSegmentChanged(int32 NumSegments, float Spacing)
{
	if (IsValid(HealthBar) && HealthBar->Implements<UPRProgressBarInterface>())
	{
		IPRProgressBarInterface::Execute_SetSegments(HealthBar, NumSegments, Spacing);
	}
}

void UPRHUDWidget::HandleShieldSegmentChanged(int32 NumSegments, float Spacing)
{
	if (IsValid(ShieldBar) && ShieldBar->Implements<UPRProgressBarInterface>())
	{
		IPRProgressBarInterface::Execute_SetSegments(ShieldBar, NumSegments, Spacing);
	}
}