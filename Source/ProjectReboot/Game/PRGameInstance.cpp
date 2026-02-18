// Fill out your copyright notice in the Description page of Project Settings.


#include "PRGameInstance.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "MoviePlayer.h"

void UPRGameInstance::Init()
{
	Super::Init();
	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UPRGameInstance::OnPreLoadMap);
}

void UPRGameInstance::Shutdown()
{
	if (IsValid(BGMComponent))
	{
		BGMComponent->Stop();
		BGMComponent = nullptr;
	}

	Super::Shutdown();
}

void UPRGameInstance::PlayBGM(const UObject* WorldContextObject, USoundBase* Sound, float FadeInDuration)
{
	if (!IsValid(Sound))
	{
		return;
	}

	// 이미 같은 사운드 재생 중이면 무시
	if (IsValid(BGMComponent) && BGMComponent->IsPlaying() && BGMComponent->Sound == Sound)
	{
		return;
	}

	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	UAudioComponent* Comp = EnsureBGMComponent(World, Sound);
	if (!IsValid(Comp))
	{
		return;
	}

	if (FadeInDuration > 0.f)
	{
		Comp->FadeIn(FadeInDuration);
	}
	else
	{
		Comp->Play();
	}
}

void UPRGameInstance::StopBGM(float FadeOutDuration)
{
	if (!IsValid(BGMComponent))
	{
		return;
	}

	if (FadeOutDuration > 0.f)
	{
		BGMComponent->FadeOut(FadeOutDuration, 0.f);
	}
	else
	{
		BGMComponent->Stop();
	}
}

void UPRGameInstance::SetBGMPaused(bool bPaused)
{
	if (!IsValid(BGMComponent))
	{
		return;
	}

	BGMComponent->SetPaused(bPaused);
}

void UPRGameInstance::SetBGMFloatParameter(FName ParameterName, float Value)
{
	if (!IsValid(BGMComponent))
	{
		return;
	}

	BGMComponent->SetFloatParameter(ParameterName, Value);
}

bool UPRGameInstance::IsBGMPlaying() const
{
	return IsValid(BGMComponent) && BGMComponent->IsPlaying();
}

void UPRGameInstance::OnPreLoadMap(const FString& MapName)
{
	if (!IsMoviePlayerEnabled())
	{
		return;
	}

	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;
	LoadingScreen.bMoviesAreSkippable = false;
	LoadingScreen.bWaitForManualStop = false;
	LoadingScreen.MinimumLoadingScreenDisplayTime = 1.f;

	if (LoadingScreenWidgetClass)
	{
		UUserWidget* LoadingWidget = CreateWidget<UUserWidget>(this, LoadingScreenWidgetClass);
		if (IsValid(LoadingWidget))
		{
			LoadingScreen.WidgetLoadingScreen = LoadingWidget->TakeWidget();
		}
	}

	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	GetMoviePlayer()->PlayMovie();
}

UAudioComponent* UPRGameInstance::EnsureBGMComponent(UWorld* World, USoundBase* Sound)
{
	if (IsValid(BGMComponent))
	{
		BGMComponent->SetSound(Sound);
		return BGMComponent;
	}

	if (!IsValid(World) || !IsValid(Sound))
	{
		return nullptr;
	}

	BGMComponent = UGameplayStatics::CreateSound2D(World, Sound, 1.0f, 1.0f, 0.f, nullptr, false, false);
	if (IsValid(BGMComponent))
	{
		BGMComponent->bAutoDestroy = false;
	}

	return BGMComponent;
}
