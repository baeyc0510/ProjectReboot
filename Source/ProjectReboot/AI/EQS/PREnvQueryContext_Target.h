#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "PREnvQueryContext_Target.generated.h"

/**
 * PRAIController의 CombatTarget을 Context로 설정
 */
UCLASS()
class PROJECTREBOOT_API UPREnvQueryContext_Target : public UEnvQueryContext
{
	GENERATED_BODY()
	
public:
	virtual void ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const override;
};
