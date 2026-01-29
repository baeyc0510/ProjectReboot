// PRWeaponExecCalc.h
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectExecutionCalculation.h"
#include "PRExecCalc_SetByCallerMappings.h"
#include "PRWeaponExecCalc.generated.h"

/**
 * 무기 어트리뷰트 SetByCaller 적용용 ExecCalc
 */
UCLASS()
class PROJECTREBOOT_API UPRWeaponExecCalc : public UPRExecCalc_SetByCallerMappings
{
	GENERATED_BODY()

public:
	// 기본 생성자
	UPRWeaponExecCalc();
};
