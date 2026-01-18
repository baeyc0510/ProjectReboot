// PRCrosshairViewModel.cpp
#include "PRCrosshairViewModel.h"

#include "AbilitySystemComponent.h"
#include "Engine/LocalPlayer.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/AbilitySystem/PRCommonAttributeSet.h"
#include "ProjectReboot/AbilitySystem/PRWeaponAttributeSet.h"
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
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}

	Config = nullptr;
	Super::Deinitialize();
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
    CrosshairTagHandle = InASC->RegisterGameplayTagEvent(
        TAG_State_Weapon_Crosshair,
        EGameplayTagEventType::NewOrRemoved
    ).AddUObject(this, &UPRCrosshairViewModel::HandleCrosshairTagChanged);
	
	// ADS 태그
    ADSTagHandle = InASC->RegisterGameplayTagEvent(
        TAG_State_Aiming,
        EGameplayTagEventType::NewOrRemoved
    ).AddUObject(this, &UPRCrosshairViewModel::HandleADSTagChanged);

    // 발사 불가 태그
    CannotFireTagHandle = InASC->RegisterGameplayTagEvent(
        TAG_State_Weapon_CannotFire,
        EGameplayTagEventType::NewOrRemoved
    ).AddUObject(this, &UPRCrosshairViewModel::HandleCannotFireTagChanged);

    // 탄약 Attribute
    AmmoHandle = InASC->GetGameplayAttributeValueChangeDelegate(
        UPRWeaponAttributeSet::GetAmmoAttribute()
    ).AddUObject(this, &UPRCrosshairViewModel::HandleAmmoChanged);

    MaxAmmoHandle = InASC->GetGameplayAttributeValueChangeDelegate(
        UPRWeaponAttributeSet::GetMaxAmmoAttribute()
    ).AddUObject(this, &UPRCrosshairViewModel::HandleMaxAmmoChanged);

    // 이동 속도 Attribute
    MoveSpeedHandle = InASC->GetGameplayAttributeValueChangeDelegate(
        UPRCommonAttributeSet::GetMoveSpeedAttribute()
    ).AddUObject(this, &UPRCrosshairViewModel::HandleMoveSpeedChanged);

    // 현재 값으로 초기화
    HandleCrosshairTagChanged(TAG_State_Weapon_Crosshair, InASC->HasMatchingGameplayTag(TAG_State_Weapon_Crosshair) ? 1 : 0);
    HandleADSTagChanged(TAG_State_Aiming, InASC->HasMatchingGameplayTag(TAG_State_Aiming) ? 1 : 0);
    HandleCannotFireTagChanged(TAG_State_Weapon_CannotFire, InASC->HasMatchingGameplayTag(TAG_State_Weapon_CannotFire) ? 1 : 0);

    if (const UPRWeaponAttributeSet* WeaponAttr = InASC->GetSet<UPRWeaponAttributeSet>())
    {
        CurrentAmmo = FMath::TruncToInt(WeaponAttr->GetAmmo());
        MaxAmmo = FMath::TruncToInt(WeaponAttr->GetMaxAmmo());
        OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
    }

    if (const UPRCommonAttributeSet* CommonAttr = InASC->GetSet<UPRCommonAttributeSet>())
    {
        MovementSpeed = CommonAttr->GetMoveSpeed();
    }
}

void UPRCrosshairViewModel::UnbindFromASC()
{
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
	MovementSpeed = FMath::Max(0.0f, Speed);
}

void UPRCrosshairViewModel::SetAmmo(int32 Current, int32 Max)
{
	if (CurrentAmmo != Current || MaxAmmo != Max)
	{
		CurrentAmmo = Current;
		MaxAmmo = Max;
		OnAmmoChanged.Broadcast(CurrentAmmo, MaxAmmo);
	}
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
	UpdateADSAlpha(DeltaTime);
	RecalculateSpread(DeltaTime);
	return true;
}

void UPRCrosshairViewModel::UpdateADSAlpha(float DeltaTime)
{
	const float TargetADSAlpha = bIsADS ? 1.0f : 0.0f;

	if (!FMath::IsNearlyEqual(ADSAlpha, TargetADSAlpha, KINDA_SMALL_NUMBER))
	{
		ADSAlpha = FMath::FInterpTo(ADSAlpha, TargetADSAlpha, DeltaTime, ADSTransitionSpeed);
		OnADSAlphaChanged.Broadcast(ADSAlpha);
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

void UPRCrosshairViewModel::HandleAmmoChanged(const FOnAttributeChangeData& Data)
{
	SetAmmo(FMath::TruncToInt(Data.NewValue), MaxAmmo);
}

void UPRCrosshairViewModel::HandleMaxAmmoChanged(const FOnAttributeChangeData& Data)
{
	SetAmmo(CurrentAmmo, FMath::TruncToInt(Data.NewValue));
}

void UPRCrosshairViewModel::HandleMoveSpeedChanged(const FOnAttributeChangeData& Data)
{
	SetMovementSpeed(Data.NewValue);
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
	const float MovementFactor = FMath::Clamp(MovementSpeed / MaxWalkSpeed, 0.0f, 1.0f);
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
