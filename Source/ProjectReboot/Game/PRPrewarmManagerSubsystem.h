// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PRPrewarmManagerSubsystem.generated.h"

class UNiagaraSystem;

/**
 * 프리웜 수행 및 중복 실행을 관리하는 서브시스템
 */
UCLASS()
class PROJECTREBOOT_API UPRPrewarmManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/*~ USubsystem Interface ~*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 서브시스템 인스턴스 획득
	UFUNCTION(BlueprintCallable, Category = "Prewarm", meta = (WorldContext = "WorldContextObject"))
	static UPRPrewarmManagerSubsystem* Get(const UObject* WorldContextObject);

	// 나이아가라 프리웜 실행 (중복 방지)
	UFUNCTION(BlueprintCallable, Category = "Prewarm")
	bool TryPrewarmNiagaraSystem(UNiagaraSystem* System, const FVector& Location, const FVector& Scale);

	// 프리웜 기록 초기화
	UFUNCTION(BlueprintCallable, Category = "Prewarm")
	void ResetPrewarmHistory();

private:
	// 프리웜 완료된 에셋 경로
	UPROPERTY()
	TSet<FSoftObjectPath> PrewarmedAssets;
};
