// PRCrosshairViewModel.cpp
#include "PRCrosshairViewModel.h"

#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/Character/PRCharacterBase.h"
#include "ProjectReboot/Crosshair/PRCrosshairConfig.h"

void UPRCrosshairViewModel::InitializeForPlayer(ULocalPlayer* InLocalPlayer)
{
	Super::InitializeForPlayer(InLocalPlayer);

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UPRCrosshairViewModel::Tick)
	);
}

void UPRCrosshairViewModel::Deinitialize()
{
	UnbindFromASC();

	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

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

    // 크로스헤어 태그
	StateTagHandle = InASC->RegisterGenericGameplayTagEvent().AddUObject(this, &UPRCrosshairViewModel::OnTagChanged);
	
    // 현재 값으로 초기화
	HandleHideTagChanged(TAG_State_UI_HideCrosshair, InASC->HasMatchingGameplayTag(TAG_State_UI_HideCrosshair) ? 1 : 0);
    HandleCrosshairTagChanged(TAG_State_Weapon_Crosshair, InASC->HasMatchingGameplayTag(TAG_State_Weapon_Crosshair) ? 1 : 0);
    HandleADSTagChanged(TAG_State_Aiming, InASC->HasMatchingGameplayTag(TAG_State_Aiming) ? 1 : 0);
    HandleCannotFireTagChanged(TAG_State_Weapon_CannotFire, InASC->HasMatchingGameplayTag(TAG_State_Weapon_CannotFire) ? 1 : 0);

    if (const UPRCommonAttributeSet* CommonAttr = InASC->GetSet<UPRCommonAttributeSet>())
    {
        CurrentMovementSpeed = CommonAttr->GetMoveSpeed();
    }
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

	if (Config)
	{
		const FPRCrosshairSetting& Setting = Config->GetCrosshairSetting(CurrentCrosshairTag);
		CurrentSpread = Setting.BaseSpread;
		OnSpreadChanged.Broadcast(CurrentSpread);
	}
}

void UPRCrosshairViewModel::SetCrosshairTag(const FGameplayTag& NewTag)
{
	if (CurrentCrosshairTag == NewTag)
	{
		return;
	}

	CurrentCrosshairTag = NewTag;
	RecoilSpread = 0.0f;

	if (Config)
	{
		const FPRCrosshairSetting& Setting = Config->GetCrosshairSetting(NewTag);
		CurrentSpread = Setting.BaseSpread;
		OnSpreadChanged.Broadcast(CurrentSpread);
	}

	OnCrosshairTagChanged.Broadcast(NewTag);
}

void UPRCrosshairViewModel::SetADSState(bool bNewIsADS)
{
	if (bIsADS != bNewIsADS)
	{
		bIsADS = bNewIsADS;
		OnADSStateChanged.Broadcast(bIsADS);
	}
}

void UPRCrosshairViewModel::OnFired()
{
	if (Config)
	{
		const FPRCrosshairSetting& Setting = Config->GetCrosshairSetting(CurrentCrosshairTag);
		RecoilSpread += Setting.RecoilSpreadIncrease;
	}
}

void UPRCrosshairViewModel::SetMovementSpeed(float Speed)
{
	CurrentMovementSpeed = FMath::Max(0.0f, Speed);
}

void UPRCrosshairViewModel::SetCanFire(bool bNewCanFire)
{
	if (bCanFire != bNewCanFire)
	{
		bCanFire = bNewCanFire;
		OnCanFireChanged.Broadcast(bCanFire);
	}
}

void UPRCrosshairViewModel::SetVisible(bool bNewVisible)
{
	if (bIsVisible != bNewVisible)
	{
		bIsVisible = bNewVisible;
		OnVisibilityChanged.Broadcast(bIsVisible);
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

const FPRCrosshairSetting& UPRCrosshairViewModel::GetCurrentSetting() const
{
	if (Config)
	{
		return Config->GetCrosshairSetting(CurrentCrosshairTag);
	}

	static FPRCrosshairSetting DefaultSetting;
	return DefaultSetting;
}

bool UPRCrosshairViewModel::Tick(float DeltaTime)
{
	if (PlayerCharacter.Get())
	{
		CurrentMovementSpeed = PlayerCharacter->GetVelocity().Length();
		MaxMovementSpeed = PlayerCharacter->GetMaxMoveSpeed();
	}
	
	UpdateADSAlpha(DeltaTime);
	RecalculateSpread(DeltaTime);
	return true;
}

void UPRCrosshairViewModel::UpdateADSAlpha(float DeltaTime)
{
	const float TargetADSAlpha = bIsADS ? 1.0f : 0.0f;

	if (!FMath::IsNearlyEqual(ADSAlpha, TargetADSAlpha))
	{
		ADSAlpha = FMath::FInterpTo(ADSAlpha, TargetADSAlpha, DeltaTime, ADSTransitionSpeed);
		OnADSAlphaChanged.Broadcast(ADSAlpha);
	}
}

void UPRCrosshairViewModel::OnTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (Tag.MatchesTag(TAG_State_UI_HideCrosshair))
	{
		HandleHideTagChanged(Tag, NewCount);
	}
	if (Tag.MatchesTag(TAG_State_Dead))
	{
		// Dead 상태이면 Hide -> -NewCount로 전달
		HandleHideTagChanged(Tag, -NewCount);
	}
	if (Tag.MatchesTag(TAG_State_Aiming))
	{
		HandleADSTagChanged(Tag, NewCount);
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
	
	// Crosshair 태그 자체인 경우 가장 구체적인 태그를 찾기
	if (CrosshairTag.MatchesTagExact(TAG_State_Weapon_Crosshair))
	{
		FGameplayTagContainer OwnedTags;
		BoundASC->GetOwnedGameplayTags(OwnedTags);

		// 깊이가 가장 깊은 태그 찾기
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
	}

	SetCrosshairTag(CrosshairTag);
}

void UPRCrosshairViewModel::HandleADSTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetADSState(NewCount > 0);
}

void UPRCrosshairViewModel::HandleCannotFireTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	SetCanFire(NewCount == 0);
}

void UPRCrosshairViewModel::RecalculateSpread(float DeltaTime)
{
	if (!Config)
	{
		return;
	}

	const FPRCrosshairSetting& Setting = Config->GetCrosshairSetting(CurrentCrosshairTag);

	// 반동 스프레드 회복
	if (RecoilSpread > 0.0f)
	{
		RecoilSpread = FMath::Max(0.0f, RecoilSpread - Setting.SpreadRecoveryRate * DeltaTime);
	}

	// 기본 스프레드
	float BaseSpread = Setting.BaseSpread;

	// 이동에 의한 스프레드 증가
	const float MovementFactor = FMath::Clamp(CurrentMovementSpeed / MaxMovementSpeed, 0.0f, 1.0f);
	const float MovementSpread = BaseSpread * (Setting.MovementSpreadMultiplier - 1.0f) * MovementFactor;

	// 총 스프레드
	float TotalSpread = BaseSpread + MovementSpread + RecoilSpread;

	// ADS 적용
	TotalSpread *= FMath::Lerp(1.0f, Setting.ADSSpreadMultiplier, ADSAlpha);

	// 최대값 제한
	TotalSpread = FMath::Clamp(TotalSpread, Setting.BaseSpread, Setting.MaxSpread);

	// 변경 시에만 브로드캐스트
	if (!FMath::IsNearlyEqual(CurrentSpread, TotalSpread, SpreadChangeTolerance))
	{
		CurrentSpread = TotalSpread;
		OnSpreadChanged.Broadcast(CurrentSpread);

		// 정규화 스프레드 계산
		const float SpreadRange = Setting.MaxSpread - Setting.BaseSpread;
		NormalizedSpread = SpreadRange > 0.0f
			                   ? FMath::Clamp((CurrentSpread - Setting.BaseSpread) / SpreadRange, 0.0f, 1.0f)
			                   : 0.0f;
	}
}
