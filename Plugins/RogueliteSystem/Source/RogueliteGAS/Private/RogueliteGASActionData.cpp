#include "RogueliteGASActionData.h"

#include "Abilities/GameplayAbility.h"


bool FRogueliteAbilityEntry::IsValidData() const
{
	return IsValid(AbilityClass);
}

bool FRogueliteEffectEntry::IsValidData() const
{
	return IsValid(EffectClass);
}

bool URogueliteGASActionData::HasGASElements() const
{
	const bool bHasAbility = Abilities.Num() > 0;
	const bool bHasEffect = Effects.Num() > 0;
	const bool bHasTagToGrant = !TagsToGrant.IsEmpty() && bGrantTagsToAbilitySystem;
	
	return bHasAbility || bHasEffect || bHasTagToGrant;
}
