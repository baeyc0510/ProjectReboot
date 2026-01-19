// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CombatBlueprintFunctionLibrary.generated.h"

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
