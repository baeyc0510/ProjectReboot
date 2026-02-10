// Fill out your copyright notice in the Description page of Project Settings.

#include "PRThemeData.h"

#include "ProjectReboot/Character/PREnemyCharacter.h"

void UPRThemeData::GetPrewarmChildren(TArray<UObject*>& OutChildren) const
{
	for (const FPREnemySpawnEntry& Entry : NormalEnemies)
	{
		if (Entry.EnemyClass)
		{
			OutChildren.Add(Entry.EnemyClass);
		}
	}

	for (const FPREnemySpawnEntry& Entry : EliteEnemies)
	{
		if (Entry.EnemyClass)
		{
			OutChildren.Add(Entry.EnemyClass);
		}
	}

	for (const FPREnemySpawnEntry& Entry : MiniBosses)
	{
		if (Entry.EnemyClass)
		{
			OutChildren.Add(Entry.EnemyClass);
		}
	}
}
