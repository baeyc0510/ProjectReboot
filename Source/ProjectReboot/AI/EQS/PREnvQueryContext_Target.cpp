#include "PREnvQueryContext_Target.h"
#include "PREQSHelper.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void UPREnvQueryContext_Target::ProvideContext(FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
	APRAIController* Controller = PREQSHelper::GetQuerierPRAIController(QueryInstance);
	if (!Controller)
	{
		return;
	}

	if (AActor* CombatTarget = Controller->GetCombatTarget())
	{
		UEnvQueryItemType_Actor::SetContextHelper(ContextData, CombatTarget);
	}
}
