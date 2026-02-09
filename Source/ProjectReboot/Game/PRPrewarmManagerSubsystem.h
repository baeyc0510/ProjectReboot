// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PRPrewarmManagerSubsystem.generated.h"

class UNiagaraSystem;
struct FStreamableHandle;

// 프리웜 완료 델리게이트
DECLARE_MULTICAST_DELEGATE(FOnPrewarmCompleteSignature);

/**
 * 프리웜 수행 및 중복 실행을 관리하는 서브시스템
 */
UCLASS()
class PROJECTREBOOT_API UPRPrewarmManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/*~ USubsystem Interface ~*/
	// 서브시스템 초기화
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	// 서브시스템 해제
	virtual void Deinitialize() override;

	// 서브시스템 인스턴스 획득
	UFUNCTION(BlueprintCallable, Category = "Prewarm", meta = (WorldContext = "WorldContextObject"))
	static UPRPrewarmManagerSubsystem* Get(const UObject* WorldContextObject);

	// 나이아가라 프리웜 실행 (중복 방지)
	UFUNCTION(BlueprintCallable, Category = "Prewarm")
	bool TryPrewarmNiagaraSystem(UNiagaraSystem* System);

	// 프리웜 실행 (인터페이스 기반 수집)
	UFUNCTION(BlueprintCallable, Category = "Prewarm")
	void ExecutePrewarm(const TArray<UObject*>& RootObjects, bool bLoadSynchronously = true);

	// 프리웜 완료 델리게이트
	FOnPrewarmCompleteSignature OnPrewarmComplete;

	// 프리웜 기록 초기화
	UFUNCTION(BlueprintCallable, Category = "Prewarm")
	void ResetPrewarmHistory();

private:
	// 프리웜 대상 수집 (재귀)
	void CollectPrewarmFromObject(UObject* RootObject, TSet<const UObject*>& Visited, TSet<FSoftObjectPath>& OutAssets) const;

	// 비동기 로딩 완료 처리
	void HandleAsyncPrewarmLoaded();

	// 프리웜 위치
	UPROPERTY(EditDefaultsOnly, Category = "Prewarm")
	FVector PrewarmLocation = FVector(1000000.0f, 1000000.0f, -1000000.0f);

	// 프리웜 스케일
	UPROPERTY(EditDefaultsOnly, Category = "Prewarm")
	FVector PrewarmScale = FVector(0.01f, 0.01f, 0.01f);

	// 비동기 로딩 대기 목록
	TArray<FSoftObjectPath> PendingPrewarmAssets;

	// 비동기 로딩 핸들
	TSharedPtr<FStreamableHandle> PrewarmHandle;

	// 프리웜 완료된 에셋 경로
	UPROPERTY()
	TSet<FSoftObjectPath> PrewarmedAssets;
};
