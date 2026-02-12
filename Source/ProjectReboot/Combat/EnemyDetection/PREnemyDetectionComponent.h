// PREnemyDetectionComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "PREnemyDetectionComponent.generated.h"

// 감지 목록 변경 이벤트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTrackedEnemiesChanged);

/**
 * 적 감지 컴포넌트
 * 플레이어 캐릭터에 부착하여 주변 적을 이벤트 기반으로 감지
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPREnemyDetectionComponent : public USphereComponent
{
	GENERATED_BODY()

public:
	UPREnemyDetectionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// 현재 감지 중인 적 목록 반환
	const TSet<TWeakObjectPtr<AActor>>& GetTrackedEnemies() const { return TrackedEnemies; }

	// 감지 목록 변경 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Enemy Detection|Events")
	FOnTrackedEnemiesChanged OnTrackedEnemiesChanged;

protected:
	/*~ UActorComponent Interface ~*/
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 적 팀 여부 확인
	bool IsEnemy(AActor* Actor) const;

	// 죽은 적 정리
	void CleanupInvalidEntries(bool& bOutChanged);

private:
	// 현재 감지 중인 적 목록
	TSet<TWeakObjectPtr<AActor>> TrackedEnemies;
};
