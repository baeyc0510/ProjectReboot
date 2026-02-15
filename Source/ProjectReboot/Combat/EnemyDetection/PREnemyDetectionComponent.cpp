// PREnemyDetectionComponent.cpp

#include "PREnemyDetectionComponent.h"

#include "GenericTeamAgentInterface.h"
#include "GameFramework/Pawn.h"
#include "ProjectReboot/ProjectReboot.h"
#include "ProjectReboot/AI/PRAIController.h"

UPREnemyDetectionComponent::UPREnemyDetectionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 감지 전용: 물리 시뮬레이션 없음
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SetCollisionResponseToChannel(EPRCollision::ECC_Enemy, ECR_Overlap);
	SetGenerateOverlapEvents(true);

	// 감지 반경 기본값
	InitSphereRadius(3000.0f);

	// 렌더링 비활성화
	SetHiddenInGame(true);

	PrimaryComponentTick.bCanEverTick = false;
}

void UPREnemyDetectionComponent::BeginPlay()
{
	Super::BeginPlay();

	OnComponentBeginOverlap.AddDynamic(this, &UPREnemyDetectionComponent::HandleBeginOverlap);
	OnComponentEndOverlap.AddDynamic(this, &UPREnemyDetectionComponent::HandleEndOverlap);
}

void UPREnemyDetectionComponent::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || !IsEnemy(OtherActor))
	{
		return;
	}

	bool bChanged = false;
	if (!TrackedEnemies.Contains(OtherActor))
	{
		TrackedEnemies.Add(OtherActor);
		bChanged = true;
	}

	CleanupInvalidEntries(bChanged);

	if (bChanged)
	{
		OnTrackedEnemiesChanged.Broadcast();
	}
}

void UPREnemyDetectionComponent::HandleEndOverlap(
	UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	bool bChanged = false;
	if (TrackedEnemies.Remove(OtherActor) > 0)
	{
		bChanged = true;
	}

	CleanupInvalidEntries(bChanged);

	if (bChanged)
	{
		OnTrackedEnemiesChanged.Broadcast();
	}
}

bool UPREnemyDetectionComponent::IsEnemy(AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	// IGenericTeamAgentInterface를 통한 팀 확인
	// Actor 자체 또는 Controller에서 확인
	const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Actor);
	if (!TeamAgent)
	{
		if (const APawn* Pawn = Cast<APawn>(Actor))
		{
			TeamAgent = Cast<IGenericTeamAgentInterface>(Pawn->GetController());
		}
	}

	if (!TeamAgent)
	{
		return false;
	}

	return TeamAgent->GetGenericTeamId() == FGenericTeamId(PRTeamId::Enemy);
}

void UPREnemyDetectionComponent::CleanupInvalidEntries(bool& bOutChanged)
{
	for (auto It = TrackedEnemies.CreateIterator(); It; ++It)
	{
		if (!It->IsValid())
		{
			It.RemoveCurrent();
			bOutChanged = true;
		}
	}
}
