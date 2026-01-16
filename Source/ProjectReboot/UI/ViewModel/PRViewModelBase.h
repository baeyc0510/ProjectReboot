// PRViewModelBase.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PRViewModelBase.generated.h"

class ULocalPlayer;

/**
 * ViewModel 기본 클래스
 * Global 또는 Actor-Bound 모드로 초기화 가능
 */
UCLASS(Abstract, BlueprintType)
class PROJECTREBOOT_API UPRViewModelBase : public UObject
{
	GENERATED_BODY()

public:
	// Global ViewModel 초기화
	virtual void InitializeForPlayer(ULocalPlayer* InLocalPlayer);

	// Actor-Bound ViewModel 초기화
	virtual void InitializeForActor(AActor* InTargetActor, ULocalPlayer* InLocalPlayer);

	// ViewModel 정리
	virtual void Deinitialize();

	// 소유 LocalPlayer 반환
	UFUNCTION(BlueprintPure, Category = "ViewModel")
	ULocalPlayer* GetLocalPlayer() const { return LocalPlayer.Get(); }

	// 바인딩된 Actor 반환 (Global이면 nullptr)
	UFUNCTION(BlueprintPure, Category = "ViewModel")
	AActor* GetTargetActor() const { return TargetActor.Get(); }

	// 소유 PlayerController 반환
	UFUNCTION(BlueprintPure, Category = "ViewModel")
	APlayerController* GetOwningPlayerController() const;

	// Actor-Bound 여부
	UFUNCTION(BlueprintPure, Category = "ViewModel")
	bool IsActorBound() const { return TargetActor.IsValid(); }

	// 바인딩된 Actor 반환
	template<typename T>
	T* GetTargetActor() const
	{
		return Cast<T>(GetTargetActor());
	}

	// 소유 PlayerController 반환
	template<typename T>
	T* GetOwningPlayerController() const
	{
		return Cast<T>(GetOwningPlayerController());
	}

protected:
	TWeakObjectPtr<ULocalPlayer> LocalPlayer;
	TWeakObjectPtr<AActor> TargetActor;
};