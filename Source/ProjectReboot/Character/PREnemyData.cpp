#include "PREnemyData.h"
#include "ProjectReboot/AbilitySystem/PRAbilitySet.h"

void UPREnemyData::GetPrewarmChildren(TArray<UObject*>& OutChildren) const
{
	if (!IsValid(AbilitySet))
	{
		return;
	}

	for (const FPRAbilityEntry& AbilityEntry : AbilitySet->Abilities)
	{
		if (AbilityEntry.AbilityClass)
		{
			OutChildren.Add(AbilityEntry.AbilityClass.Get());
		}
	}
}
