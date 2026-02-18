// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PRGameInstance.generated.h"

class UAudioComponent;
class UUserWidget;

/**
 *
 */
UCLASS()
class PROJECTREBOOT_API UPRGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	/*~ UGameInstance Interface ~*/
	virtual void Init() override;
	virtual void Shutdown() override;

	/*~ BGM ~*/

	// BGM 재생 (이미 재생 중이면 교체)
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM", meta = (WorldContext = "WorldContextObject"))
	void PlayBGM(const UObject* WorldContextObject, USoundBase* Sound, float FadeInDuration = 0.5f);

	// BGM 정지
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void StopBGM(float FadeOutDuration = 0.5f);

	// BGM 일시정지/재개
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void SetBGMPaused(bool bPaused);

	// MetaSound float 파라미터 세팅
	UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
	void SetBGMFloatParameter(FName ParameterName, float Value);

	// 현재 BGM 재생 중인지
	UFUNCTION(BlueprintPure, Category = "Audio|BGM")
	bool IsBGMPlaying() const;

	// BGM AudioComponent 직접 접근
	UFUNCTION(BlueprintPure, Category = "Audio|BGM")
	UAudioComponent* GetBGMComponent() const { return BGMComponent; }

private:
	// 맵 로드 시작 시 로딩 스크린 설정
	void OnPreLoadMap(const FString& MapName);

	// 유효한 BGM AudioComponent 확보 (없으면 CreateSound2D로 생성)
	UAudioComponent* EnsureBGMComponent(UWorld* World, USoundBase* Sound);

	// 로딩 스크린 위젯 클래스 (BP에서 디자인)
	UPROPERTY(EditDefaultsOnly, Category = "Loading")
	TSubclassOf<UUserWidget> LoadingScreenWidgetClass;

	// BGM 전용 AudioComponent
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> BGMComponent;
};
