// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace EPRCollision
{
	constexpr ECollisionChannel ECC_Attack = ECC_GameTraceChannel1;
	constexpr ECollisionChannel ECC_Enemy = ECC_GameTraceChannel4;
	constexpr ECollisionChannel ECC_Ground = ECC_GameTraceChannel6;
	constexpr ECollisionChannel ECC_Interaction = ECC_GameTraceChannel7;
}