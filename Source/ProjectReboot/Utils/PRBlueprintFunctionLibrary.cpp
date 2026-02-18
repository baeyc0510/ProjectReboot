// Fill out your copyright notice in the Description page of Project Settings.


#include "PRBlueprintFunctionLibrary.h"

#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"
#include "ProjectReboot/Game/PRGameInstance.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "ProjectReboot/Room/PRFieldDropActor.h"
#include "ProjectReboot/Room/PRRoomController.h"

TArray<UMaterialInstanceDynamic*> UPRBlueprintFunctionLibrary::GetAllDynamicMaterials(UMeshComponent* TargetMesh,
	bool bCreateIfMissing)
{
	TArray<UMaterialInstanceDynamic*> DynamicMaterials;

	if (!TargetMesh)
	{
		return DynamicMaterials;
	}

	const int32 NumMaterials = TargetMesh->GetNumMaterials();
	DynamicMaterials.Reserve(NumMaterials);

	for (int32 i = 0; i < NumMaterials; i++)
	{
		// 현재 슬롯의 머티리얼 가져오기
		UMaterialInterface* CurrentMat = TargetMesh->GetMaterial(i);
		if (!CurrentMat) continue;

		// 이미 MID인지 확인
		UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(CurrentMat);

		// MID가 아니고, 생성 옵션(bCreateIfMissing)이 켜져 있다면 -> 새로 생성
		if (!MID && bCreateIfMissing)
		{
			MID = TargetMesh->CreateAndSetMaterialInstanceDynamic(i);
		}

		// 유효한 MID가 있다면 배열에 추가
		if (MID)
		{
			DynamicMaterials.Add(MID);
		}
	}

	return DynamicMaterials;
}

APRFieldDropActor* UPRBlueprintFunctionLibrary::TrySpawnFieldDrop(
	const UObject* WorldContextObject,
	const FVector& SpawnLocation,
	const FRogueliteQuery& DropQuery,
	TSubclassOf<APRFieldDropActor> DropActorClass,
	float DropChance)
{
	if (!IsValid(WorldContextObject) || !DropActorClass)
	{
		return nullptr;
	}

	// 확률 판정
	if (DropChance < 1.0f && FMath::FRand() > DropChance)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	URogueliteSubsystem* RogueliteSystem = URogueliteSubsystem::Get(WorldContextObject);
	if (!IsValid(RogueliteSystem))
	{
		return nullptr;
	}

	TArray<URogueliteActionData*> QueryResults = RogueliteSystem->ExecuteQuery(DropQuery);
	if (QueryResults.Num() == 0)
	{
		return nullptr;
	}

	URogueliteActionData* ActionData = QueryResults[0];
	if (!IsValid(ActionData))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APRFieldDropActor* DropActor = World->SpawnActor<APRFieldDropActor>(DropActorClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (!IsValid(DropActor))
	{
		return nullptr;
	}

	DropActor->SetActionData(ActionData);

	// 현재 RoomController에 스폰 액터 등록
	APRGameplayGameState* GameState = World->GetGameState<APRGameplayGameState>();
	if (IsValid(GameState))
	{
		APRRoomController* RoomController = GameState->GetCurrentRoomController();
		if (IsValid(RoomController))
		{
			RoomController->RegisterSpawnedActor(DropActor);
		}
	}

	return DropActor;
}

/*~ BGM ~*/

void UPRBlueprintFunctionLibrary::PlayBGM(const UObject* WorldContextObject, USoundBase* Sound, float FadeInDuration)
{
	if (UPRGameInstance* GI = GetPRGameInstance(WorldContextObject))
	{
		GI->PlayBGM(WorldContextObject, Sound, FadeInDuration);
	}
}

void UPRBlueprintFunctionLibrary::StopBGM(const UObject* WorldContextObject, float FadeOutDuration)
{
	if (UPRGameInstance* GI = GetPRGameInstance(WorldContextObject))
	{
		GI->StopBGM(FadeOutDuration);
	}
}

void UPRBlueprintFunctionLibrary::SetBGMPaused(const UObject* WorldContextObject, bool bPaused)
{
	if (UPRGameInstance* GI = GetPRGameInstance(WorldContextObject))
	{
		GI->SetBGMPaused(bPaused);
	}
}

void UPRBlueprintFunctionLibrary::SetBGMFloatParameter(const UObject* WorldContextObject, FName ParameterName, float Value)
{
	if (UPRGameInstance* GI = GetPRGameInstance(WorldContextObject))
	{
		GI->SetBGMFloatParameter(ParameterName, Value);
	}
}

bool UPRBlueprintFunctionLibrary::IsBGMPlaying(const UObject* WorldContextObject)
{
	if (UPRGameInstance* GI = GetPRGameInstance(WorldContextObject))
	{
		return GI->IsBGMPlaying();
	}
	return false;
}

UPRGameInstance* UPRBlueprintFunctionLibrary::GetPRGameInstance(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	return Cast<UPRGameInstance>(World->GetGameInstance());
}
