// PRTraceTypes.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "PRTraceTypes.generated.h"

class AActor;

UENUM(BlueprintType)
enum class EPRTraceShape : uint8
{
	// 구형 트레이스
	Sphere,
	// 박스형 트레이스
	Box,
};

/** 트레이스 설정 구조체 */
USTRUCT(BlueprintType)
struct FPRTraceSettings
{
	GENERATED_BODY()

	// 트레이스 형태
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace")
	EPRTraceShape TraceShape = EPRTraceShape::Sphere;

	// 트레이스 반경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "TraceShape == EPRTraceShape::Sphere"))
	float TraceRadius = 250.0f;

	// 박스 트레이스 크기 (Half Extent)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "TraceShape == EPRTraceShape::Box"))
	FVector TraceBoxHalfExtent = FVector(200.0f, 200.0f, 200.0f);

	// 박스 트레이스 회전 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "TraceShape == EPRTraceShape::Box"))
	bool bUseActorRotationForBox = true;

	// 박스 회전 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "TraceShape == EPRTraceShape::Box && bUseActorRotationForBox"))
	FRotator BoxRotationOffset = FRotator::ZeroRotator;

	// 구형 트레이스 원기둥 필터 사용 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "TraceShape == EPRTraceShape::Sphere"))
	bool bUseCylinderFilter = false;

	// 원기둥 높이 (Half Height)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace", meta = (EditCondition = "TraceShape == EPRTraceShape::Sphere && bUseCylinderFilter"))
	float CylinderHalfHeight = 150.0f;

	// 트레이스 채널
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

	// 디버그 트레이스 표시 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace")
	bool bDrawDebugTrace = false;

	// 디버그 표시 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace")
	float DebugDrawTime = 1.0f;

	// 트레이스 타겟 클래스 배열 (설정된 경우 해당 클래스만 처리)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace")
	TArray<TSubclassOf<AActor>> TraceTargetClasses;

	// 최대 히트 수 (0이면 제한 없음)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace")
	int32 MaxHitCount = 0;
};
