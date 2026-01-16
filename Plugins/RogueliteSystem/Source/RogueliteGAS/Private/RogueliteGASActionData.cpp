#include "RogueliteGASActionData.h"

bool URogueliteGASActionData::HasGASElements() const
{
	const bool bHasAbility = Abilities.Num() > 0;
	const bool bHasEffect = Effects.Num() > 0;
	const bool bHasTagToGrant = !TagsToGrant.IsEmpty() && bGrantTagsToAbilitySystem;
	
	return bHasAbility || bHasEffect || bHasTagToGrant;
}