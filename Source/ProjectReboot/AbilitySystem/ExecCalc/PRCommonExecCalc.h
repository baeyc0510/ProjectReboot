// PRCommonExecCalc.h
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PRExecCalc_SetByCallerMappings.h"
#include "PRCommonExecCalc.generated.h"

/**
 * 공통 어트리뷰트 SetByCaller 적용용 ExecCalc
 */
UCLASS()
class PROJECTREBOOT_API UPRCommonExecCalc : public UPRExecCalc_SetByCallerMappings
{
	GENERATED_BODY()

public:
	// 기본 생성자
	UPRCommonExecCalc();
};
