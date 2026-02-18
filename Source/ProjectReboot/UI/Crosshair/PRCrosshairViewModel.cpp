// PRCrosshairViewModel.cpp
#include "PRCrosshairViewModel.h"

#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Character/PRCharacterBase.h"
#include "ProjectReboot/Crosshair/PRCrosshairConfig.h"

UPRCrosshairViewModel::UPRCrosshairViewModel()
{
	ViewModelTag = TAG_UI_ViewModel_HUD_Crosshair;
}

void UPRCrosshairViewModel::InitializeForPlayer(ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForPlayer(InLocalPlayer);
	SetVisible(true);
}

void UPRCrosshairViewModel::Deinitialize()
{
	UnbindFromASC();
	Config = nullptr;
	Super::Deinitialize();
}

void UPRCrosshairViewModel::SetCharacter(APRCharacterBase* InCharacter)
{
	PlayerCharacter = InCharacter;
}

void UPRCrosshairViewModel::BindToASC(UAbilitySystemComponent* InASC)
{
	UnbindFromASC();

    if (!InASC)
    {
        return;
    }

    BoundASC = InASC;

	StateTagHandle = InASC->RegisterGenericGameplayTagEvent().AddUObject(this, &UPRCrosshairViewModel::OnTagChanged);

	HandleHideTagChanged(TAG_State_UI_HideCrosshair, InASC->HasMatchingGameplayTag(TAG_State_UI_HideCrosshair) ? 1 : 0);
    HandleCrosshairTagChanged(TAG_State_Weapon_Crosshair, InASC->HasMatchingGameplayTag(TAG_State_Weapon_Crosshair) ? 1 : 0);
    HandleCannotFireTagChanged(TAG_State_Weapon_CannotFire, InASC->HasMatchingGameplayTag(TAG_State_Weapon_CannotFire) ? 1 : 0);
}

void UPRCrosshairViewModel::UnbindFromASC()
{
	if (BoundASC.IsValid())
	{
		if (StateTagHandle.IsValid())
		{
			BoundASC->RegisterGenericGameplayTagEvent().Remove(StateTagHandle);
			StateTagHandle.Reset();
		}
	}

	BoundASC.Reset();
}

void UPRCrosshairViewModel::SetConfig(UPRCrosshairConfig* InConfig)
{
	Config = InConfig;
}

void UPRCrosshairViewModel::SetCrosshairTag(const FGameplayTag& NewTag)
{
	if (CurrentCrosshairTag == NewTag)
	{
		return;
	}

	CurrentCrosshairTag = NewTag;
	OnCrosshairTagChanged.Broadcast(NewTag);
}

void UPRCrosshairViewModel::SetCanFire(bool bNewCanFire)
{
	if (bCanFire != bNewCanFire)
	{
		bCanFire = bNewCanFire;
		OnCanFireChanged.Broadcast(bCanFire);
	}
}

void UPRCrosshairViewModel::SetTargetingEnemy(bool bNewTargeting)
{
	if (bIsTargetingEnemy != bNewTargeting)
	{
		bIsTargetingEnemy = bNewTargeting;
		OnTargetingEnemyChanged.Broadcast(bIsTargetingEnemy);
	}
}

void UPRCrosshairViewModel::ShowHitMarker(EPRHitMarkerType HitMarkerType)
{
	OnHitMarkerTriggered.Broadcast(HitMarkerType);
}

const FPRCrosshairSetting& UPRCrosshairViewModel::GetCurrentSetting() const
{
	if (Config)
	{
		return Config->GetCrosshairSetting(CurrentCrosshairTag);
	}

	static FPRCrosshairSetting DefaultSetting;
	return DefaultSetting;
}

void UPRCrosshairViewModel::OnTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (Tag.MatchesTag(TAG_State_UI_HideCrosshair))
	{
		HandleHideTagChanged(Tag, NewCount);
	}
	if (Tag.MatchesTag(TAG_State_Dead))
	{
		HandleHideTagChanged(Tag, -NewCount);
	}
	if (Tag.MatchesTag(TAG_State_Weapon_Crosshair))
	{
		HandleCrosshairTagChanged(Tag, NewCount);
	}
	if (Tag.MatchesTag(TAG_State_Weapon_CannotFire))
	{
		HandleCannotFireTagChanged(Tag, NewCount);
	}
}

void UPRCrosshairViewModel::HandleHideTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		SetVisible(false);
	}
	else
	{
		SetVisible(true);
	}
}

void UPRCrosshairViewModel::HandleCrosshairTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (!BoundASC.IsValid())
	{
		return;
	}

	FGameplayTag CrosshairTag = Tag;

	if (NewCount > 0 && !CrosshairTag.MatchesTagExact(TAG_State_Weapon_Crosshair))
	{
		SetCrosshairTag(CrosshairTag);
		return;
	}

	FGameplayTagContainer OwnedTags;
	BoundASC->GetOwnedGameplayTags(OwnedTags);

	CrosshairTag = TAG_State_Weapon_Crosshair;
	int32 MaxDepth = 0;

	for (const FGameplayTag& OwnedTag : OwnedTags)
	{
		if (!OwnedTag.MatchesTag(TAG_State_Weapon_Crosshair))
		{
			continue;
		}

		const int32 Depth = OwnedTag.GetGameplayTagParents().Num();
		if (Depth > MaxDepth)
		{
			MaxDepth = Depth;
			CrosshairTag = OwnedTag;
		}
	}

	SetCrosshairTag(CrosshairTag);
}

void UPRCrosshairViewModel::HandleCannotFireTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetCanFire(NewCount == 0);
}
