// PRGA_MeleeAttack.h
#pragma once

#include "CoreMinimal.h"
#include "ProjectReboot/AbilitySystem/PRGameplayAbility.h"
#include "PRGA_MeleeAttack.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class USkeletalMeshComponent;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EPRMeleeTraceDirection : uint8
{
	// 소켓 전방 기준
	SocketForward,
	// 액터 전방 기준
	ActorForward,
};

/**
 * 근접 공격 어빌리티 (트레이스 방식)
 */
UCLASS()
class PROJECTREBOOT_API UPRGA_MeleeAttack : public UPRGameplayAbility
{
	GENERATED_BODY()

public:
	UPRGA_MeleeAttack();

	/*~ UGameplayAbility Interfaces ~*/

	// 어빌리티 활성화
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// 어빌리티 종료
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	/*~ UPRGA_MeleeAttack Interfaces ~*/

	// 근접 공격 몽타주 재생
	void PlayAttackMontage();

	// 공격 이벤트 수신
	UFUNCTION()
	void OnAttackEventReceived(FGameplayEventData Payload);

	// 몽타주 종료 콜백
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageBlendOut();

	UFUNCTION()
	void OnMontageCancelled();

	// 트레이스 수행
	void PerformAttackTrace();

	// 데미지 적용
	void ApplyMeleeDamage(const FHitResult& HitResult);

	// 메시 컴포넌트 조회
	USkeletalMeshComponent* GetMeshComponent() const;

protected:
	// 공격 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Montage")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 몽타주 재생 속도
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Montage")
	float MontagePlayRate = 1.0f;

	// 공격 이벤트 다중 수신 허용 여부
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	bool bAllowMultipleEvents = false;

	// 트레이스 시작 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	FName TraceStartSocketName;

	// 트레이스 시작 오프셋 (소켓 로컬)
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	FVector TraceStartSocketOffset = FVector::ZeroVector;

	// 트레이스 거리
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	float TraceDistance = 150.0f;

	// 트레이스 진행 방향 기준
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	EPRMeleeTraceDirection TraceDirectionType = EPRMeleeTraceDirection::ActorForward;

	// 트레이스 반경
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	float TraceRadius = 25.0f;

	// 트레이스 채널
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	// 트레이스 타겟 클래스 (설정된 경우 해당 클래스의 액터만 Trace)
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	TSubclassOf<AActor> TraceTargetClass = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	int32 MaxTraceCount = 1;
	
	// 디버그 트레이스 표시 여부
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	bool bDrawDebugTrace = false;

	// 디버그 표시 시간
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Trace")
	float DebugDrawTime = 1.0f;

	// 데미지 GE 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 기본 데미지
	UPROPERTY(EditDefaultsOnly, Category = "Melee|Damage")
	float BaseDamage = 10.0f;

private:
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* MontageTask;

	UPROPERTY()
	UAbilityTask_WaitGameplayEvent* WaitEventTask;

	// 중복 히트 방지용 캐시
	TSet<TWeakObjectPtr<AActor>> HitActors;
};
