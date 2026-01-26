// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/EngineTypes.h"
#include "ProjectReboot/Combat/PRTraceTypes.h"
#include "CombatBlueprintFunctionLibrary.generated.h"
class USkeletalMeshComponent;
/**
 * 
 */
UCLASS()
class PROJECTREBOOT_API UCombatBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * HitResult에서 피격 방향을 계산
	 * @param HitResult 피격 정보
	 * @param HitActor 피격당한 액터
	 * @return 4방향 피격 방향
	 */
	UFUNCTION(BlueprintCallable)
	static EPRHitDirection GetHitDirectionFromHitResult(const FHitResult& HitResult, const AActor* HitActor);
	
	/**
	 * 두 액터 위치로 피격 방향을 계산 (HitResult 없을 때 대체용)
	 * @param Instigator 공격자
	 * @param HitActor 피격당한 액터
	 * @return 4방향 피격 방향
	 */
	UFUNCTION(BlueprintCallable)
	static EPRHitDirection GetHitDirectionFromInstigator(const AActor* Instigator, const AActor* HitActor);

	/*~ Trace ~*/

	// SweepTrace cpp 유틸
	static bool SphereSweepTraceByStartEnd(
		const UObject* WorldContextObject,
		const FVector& Start,
		const FVector& End,
		const FVector& Direction,
		float Radius,
		TEnumAsByte<ECollisionChannel> TraceChannel,
		const FCollisionQueryParams& QueryParams,
		TArray<FHitResult>& OutHits,
		bool bDrawDebug = false,
		float DebugDrawTime = 1.0f,
		const TArray<TSubclassOf<AActor>>& TargetClasses = {},
		bool bUseCylinderFilter = false,
		float CylinderHalfHeight = 0.0f,
		int32 MaxHitCount = 0);

	// 박스 스윕 트레이스 유틸
	static bool BoxSweepTraceByStartEnd(
		const UObject* WorldContextObject,
		const FVector& Start,
		const FVector& End,
		const FVector& Direction,
		const FVector& HalfExtent,
		const FRotator& BoxRotation,
		TEnumAsByte<ECollisionChannel> TraceChannel,
		const FCollisionQueryParams& QueryParams,
		TArray<FHitResult>& OutHits,
		bool bDrawDebug = false,
		float DebugDrawTime = 1.0f,
		const TArray<TSubclassOf<AActor>>& TargetClasses = {},
		int32 MaxHitCount = 0);

	// 원기둥 내부 여부 확인 (Z축 기준)
	static bool IsInsideCylinder(const FVector& TestPoint, const FVector& CylinderCenter, float Radius, float HalfHeight);

	// 설정 기반 트레이스 유틸
	static bool TraceBySettings(
		const UObject* WorldContextObject,
		const FVector& Start,
		const FVector& End,
		const FVector& Direction,
		const FPRTraceSettings& TraceSettings,
		const FCollisionQueryParams& QueryParams,
		TArray<FHitResult>& OutHits,
		const FRotator& BoxRotation);

	// 히트 결과를 CombatInterface에 전달
	UFUNCTION(BlueprintCallable, Category = "Combat|Trace")
	static void NotifyCombatInterfaceHit(const TArray<FHitResult>& HitResults);
	
	/*~ Ragdoll ~*/

	// 래그돌 활성화
	UFUNCTION(BlueprintCallable, Category = "Combat|Ragdoll")
	static void EnableRagdoll(ACharacter* Character, const FName& RagdollCollisionProfile = TEXT("Ragdoll"));

	// 래그돌 활성화 + Impulse 적용
	UFUNCTION(BlueprintCallable, Category = "Combat|Ragdoll")
	static void EnableRagdollWithImpulse(
		ACharacter* Character,
		const FVector& Impulse,
		const FName& BoneName = NAME_None,
		const FName& RagdollCollisionProfile = TEXT("Ragdoll"));

	// 래그돌 비활성화
	UFUNCTION(BlueprintCallable, Category = "Combat|Ragdoll")
	static void DisableRagdoll(ACharacter* Character);
	
	// 래그돌 물리 정지 (Sleep)
	UFUNCTION(BlueprintCallable, Category = "Combat|Ragdoll")
	static void FreezeRagdoll(ACharacter* Character);
};
