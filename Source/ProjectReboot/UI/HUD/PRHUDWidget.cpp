#include "PRHUDWidget.h"
#include "PRHUDViewModel.h"
#include "ProjectReboot/UI/ViewModel/PRViewModelSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"

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

	ViewModel = nullptr;
}

void UPRHUDWidget::ApplyInitialState()
{
	if (!ViewModel)
	{
		return;
	}

	HandleAmmoChanged(ViewModel->GetCurrentAmmo(), ViewModel->GetMaxAmmo());
	HandleReserveAmmoChanged(ViewModel->GetCurrentReserveAmmo(), ViewModel->GetMaxReserveAmmo());
	HandleWeaponTypeChanged(ViewModel->GetWeaponTypeTag());
	HandleHealthChanged(ViewModel->GetCurrentHealth(), ViewModel->GetMaxHealth());
	HandleShieldChanged(ViewModel->GetCurrentShield(), ViewModel->GetMaxShield());
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
	if (HealthBar)
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		HealthBar->SetPercent(Percent);
	}
}

void UPRHUDWidget::HandleShieldChanged(float Current, float Max)
{
	if (ShieldBar)
	{
		const float Percent = (Max > KINDA_SMALL_NUMBER) ? (Current / Max) : 0.0f;
		ShieldBar->SetPercent(Percent);
	}
}