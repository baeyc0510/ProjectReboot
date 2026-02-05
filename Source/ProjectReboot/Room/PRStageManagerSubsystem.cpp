// Fill out your copyright notice in the Description page of Project Settings.

#include "PRStageManagerSubsystem.h"
#include "PRStageConfigData.h"
#include "PRRoomWorldSubsystem.h"
#include "RogueliteSubsystem.h"
#include "Engine/AssetManager.h"

namespace PRStageHelpers
{
	/**
	 * 가중치 배열에서 랜덤 인덱스 선택
	 * @param Weights 가중치 배열
	 * @param Random 랜덤 스트림
	 * @return 선택된 인덱스 (-1 = 실패)
	 */
	int32 SelectWeightedIndex(const TArray<float>& Weights, FRandomStream& Random)
	{
		float TotalWeight = 0.0f;
		for (const float Weight : Weights)
		{
			TotalWeight += FMath::Max(0.0f, Weight);
		}

		if (TotalWeight <= 0.0f || Weights.Num() == 0)
		{
			return Weights.Num() > 0 ? 0 : -1;
		}

		const float RandomValue = Random.FRandRange(0.0f, TotalWeight);
		float AccumulatedWeight = 0.0f;

		for (int32 i = 0; i < Weights.Num(); i++)
		{
			AccumulatedWeight += FMath::Max(0.0f, Weights[i]);
			if (RandomValue <= AccumulatedWeight)
			{
				return i;
			}
		}

		return Weights.Num() - 1;
	}
}

/*~ USubsystem Interface ~*/

void UPRStageManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// RogueliteSubsystem 종속성 선언 및 이벤트 바인딩
	Collection.InitializeDependency<URogueliteSubsystem>();

	if (URogueliteSubsystem* Roguelite = URogueliteSubsystem::Get(this))
	{
		Roguelite->OnRunStarted.AddDynamic(this, &UPRStageManagerSubsystem::HandleRunStarted);
		Roguelite->OnRunEnded.AddDynamic(this, &UPRStageManagerSubsystem::HandleRunEnded);
	}
}

void UPRStageManagerSubsystem::Deinitialize()
{
	// RogueliteSubsystem 이벤트 언바인딩
	if (URogueliteSubsystem* Roguelite = URogueliteSubsystem::Get(this))
	{
		Roguelite->OnRunStarted.RemoveDynamic(this, &UPRStageManagerSubsystem::HandleRunStarted);
		Roguelite->OnRunEnded.RemoveDynamic(this, &UPRStageManagerSubsystem::HandleRunEnded);
	}

	Super::Deinitialize();
}

/*~ Static Access ~*/

UPRStageManagerSubsystem* UPRStageManagerSubsystem::Get(const UObject* WorldContextObject)
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

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UPRStageManagerSubsystem>();
}

/*~ Stage Config ~*/

void UPRStageManagerSubsystem::RegisterStageConfig(UPRStageConfigData* StageConfig)
{
	if (!IsValid(StageConfig))
	{
		return;
	}

	StageConfigs.Add(StageConfig->StageIndex, StageConfig);
	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Stage config %d registered"), StageConfig->StageIndex);
}

void UPRStageManagerSubsystem::RegisterStageConfigs(const TArray<UPRStageConfigData*>& InStageConfigs)
{
	for (UPRStageConfigData* Config : InStageConfigs)
	{
		RegisterStageConfig(Config);
	}
}

void UPRStageManagerSubsystem::LoadAndRegisterStageConfigs(FPrimaryAssetType AssetType)
{
	UAssetManager& AssetManager = UAssetManager::Get();

	// 해당 타입의 모든 Primary Asset ID 조회
	TArray<FPrimaryAssetId> AssetIds;
	AssetManager.GetPrimaryAssetIdList(AssetType, AssetIds);

	if (AssetIds.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("PRStageManagerSubsystem: No assets found for type %s"), *AssetType.ToString());
		return;
	}

	// 동기 로드 및 등록
	for (const FPrimaryAssetId& AssetId : AssetIds)
	{
		FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetPath(AssetId);
		if (AssetPath.IsValid())
		{
			UObject* LoadedAsset = AssetPath.TryLoad();
			if (UPRStageConfigData* StageConfig = Cast<UPRStageConfigData>(LoadedAsset))
			{
				RegisterStageConfig(StageConfig);
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Loaded %d stage configs from type %s"), StageConfigs.Num(), *AssetType.ToString());
}

UPRStageConfigData* UPRStageManagerSubsystem::GetStageConfig(int32 StageIndex) const
{
	if (const TObjectPtr<UPRStageConfigData>* Found = StageConfigs.Find(StageIndex))
	{
		return *Found;
	}
	return nullptr;
}

UPRStageConfigData* UPRStageManagerSubsystem::GetCurrentStageConfig() const
{
	return GetStageConfig(StageProgress.CurrentStageIndex);
}

/*~ RogueliteSubsystem 연동 ~*/

void UPRStageManagerSubsystem::HandleRunStarted()
{
	// 스테이지 진행 초기화
	StageProgress.Reset();
	StageProgress.MasterSeed = FMath::Rand();
	RandomStream.Initialize(StageProgress.MasterSeed);
	RoomGraph.Empty();
	StartRoomIndices.Empty();
	GraphStageIndex = INDEX_NONE;
	BuildRoomGraphForStage(StageProgress.CurrentStageIndex);

	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Stage progress initialized with seed %d"), StageProgress.MasterSeed);
}

void UPRStageManagerSubsystem::HandleRunEnded(bool bCompleted)
{
	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Run ended. Completed: %s"), bCompleted ? TEXT("Yes") : TEXT("No"));
}

/*~ Room Graph ~*/

const FPRRoomNodeInfo* UPRStageManagerSubsystem::GetRoomNodeInfo(int32 RoomIndex) const
{
	return RoomGraph.Find(RoomIndex);
}

const FPRRoomNodeInfo* UPRStageManagerSubsystem::GetCurrentRoomNodeInfo() const
{
	return GetRoomNodeInfo(StageProgress.CurrentRoomIndex);
}

TArray<int32> UPRStageManagerSubsystem::GetNextRoomIndices(int32 RoomIndex) const
{
	if (const FPRRoomNodeInfo* NodeInfo = GetRoomNodeInfo(RoomIndex))
	{
		return NodeInfo->NextRoomIndices;
	}
	return TArray<int32>();
}

/*~ Room Transition ~*/

void UPRStageManagerSubsystem::EnterRoomByIndex(int32 RoomIndex)
{
	const FPRRoomNodeInfo* NodeInfo = GetRoomNodeInfo(RoomIndex);
	if (!NodeInfo)
	{
		UE_LOG(LogTemp, Warning, TEXT("PRStageManagerSubsystem: Cannot enter room %d - node not found"), RoomIndex);
		return;
	}

	// 상태 업데이트
	StageProgress.CurrentRoomIndex = RoomIndex;

	OnRoomEntered.Broadcast(RoomIndex);

	if (NodeInfo->RoomType == EPRRoomType::Boss)
	{
		// 보스 맵 이동
		const UPRStageConfigData* StageConfig = GetCurrentStageConfig();
		if (!IsValid(StageConfig))
		{
			UE_LOG(LogTemp, Warning, TEXT("PRStageManagerSubsystem: Cannot enter boss room - no stage config"));
			return;
		}

		if (StageConfig->BossMap.IsNull())
		{
			UE_LOG(LogTemp, Warning, TEXT("PRStageManagerSubsystem: Cannot enter boss room - BossMap is null"));
			return;
		}

		SaveStageProgress();

		UGameInstance* GameInstance = GetGameInstance();
		if (!IsValid(GameInstance))
		{
			UE_LOG(LogTemp, Error, TEXT("PRStageManagerSubsystem: Cannot travel to boss map - no game instance"));
			return;
		}

		UWorld* World = GameInstance->GetWorld();
		if (!IsValid(World))
		{
			UE_LOG(LogTemp, Error, TEXT("PRStageManagerSubsystem: Cannot travel to boss map - no world"));
			return;
		}

		const FString MapPath = StageConfig->BossMap.ToString();
		UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Traveling to boss map: %s"), *MapPath);
		World->ServerTravel(MapPath, true);
	}
	else
	{
		// Level Instance 로드
		if (!NodeInfo->Template.IsNull())
		{
			UGameInstance* GameInstance = GetGameInstance();
			if (!IsValid(GameInstance))
			{
				UE_LOG(LogTemp, Error, TEXT("PRStageManagerSubsystem: Cannot load room - no game instance"));
				return;
			}

			UWorld* World = GameInstance->GetWorld();
			if (!IsValid(World))
			{
				UE_LOG(LogTemp, Error, TEXT("PRStageManagerSubsystem: Cannot load room - no world"));
				return;
			}

			if (UPRRoomWorldSubsystem* RoomSubsystem = World->GetSubsystem<UPRRoomWorldSubsystem>())
			{
				// 레벨 인스턴스 로드 (방 위치는 추후 맵 레이아웃에 따라 계산)
				const FVector RoomLocation = FVector(RoomIndex * 5000.0f, 0.0f, 0.0f);
				RoomSubsystem->LoadRoomTemplate(RoomIndex, NodeInfo->Template, RoomLocation);

				UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Loading room template %s at index %d"), *NodeInfo->Template.ToString(), RoomIndex);
			}
		}
	}
}

void UPRStageManagerSubsystem::OnRoomCleared()
{
	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Room %d cleared"), StageProgress.CurrentRoomIndex);
	// 문 선택은 플레이어가 직접 수행 (UI/문 상호작용 → EnterRoomByIndex 호출)
}

void UPRStageManagerSubsystem::OnBossDefeated()
{
	const int32 NextStageIndex = StageProgress.CurrentStageIndex + 1;

	if (StageConfigs.Contains(NextStageIndex))
	{
		// 다음 스테이지로
		StageProgress.CurrentStageIndex = NextStageIndex;
		StageProgress.CurrentRoomIndex = -1;
		BuildRoomGraphForStage(NextStageIndex);

		OnStageChanged.Broadcast(NextStageIndex);

		UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Moving to stage %d"), NextStageIndex);
	}
	else
	{
		// 런 완료 - RogueliteSubsystem에 위임
		if (URogueliteSubsystem* Roguelite = URogueliteSubsystem::Get(this))
		{
			Roguelite->EndRun(true);
		}
	}
}

/*~ Save/Load ~*/

void UPRStageManagerSubsystem::SaveStageProgress()
{
	SavedStageProgress = StageProgress;
	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Stage progress saved"));
}

void UPRStageManagerSubsystem::LoadStageProgress()
{
	StageProgress = SavedStageProgress;
	RandomStream.Initialize(StageProgress.MasterSeed);
	BuildRoomGraphForStage(StageProgress.CurrentStageIndex);

	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Stage progress loaded"));
}

/*~ Internal ~*/

void UPRStageManagerSubsystem::BuildRoomGraphForStage(int32 StageIndex)
{
	RoomGraph.Empty();
	StartRoomIndices.Empty();
	GraphStageIndex = StageIndex;

	const UPRStageConfigData* StageConfig = GetStageConfig(StageIndex);
	if (!IsValid(StageConfig))
	{
		return;
	}

	int32 NextRoomIndex = 0;
	
	// 각 슬롯에 대해 모든 가능한 이전 RoomType 조합으로 노드 생성
	// 단순화: 첫 슬롯부터 순차적으로 그래프 구축
	TArray<EPRRoomType> RoomTypes = {
		EPRRoomType::None,
		EPRRoomType::Combat,
		EPRRoomType::Elite,
		EPRRoomType::MiniBoss,
		EPRRoomType::Shop,
		EPRRoomType::Treasure,
		EPRRoomType::Rest,
		EPRRoomType::Boss
	};

	// 슬롯별로 방 노드 생성
	// 간소화된 그래프: 슬롯당 ChoiceCount개의 노드 생성
	for (int32 SlotIndex = 0; SlotIndex < StageConfig->Slots.Num(); SlotIndex++)
	{
		const FPRRoomSlot& Slot = StageConfig->Slots[SlotIndex];
		
		// 이전 슬롯의 마지막 노드 타입 (첫 슬롯은 None)
		EPRRoomType LastType = EPRRoomType::None;
		if (SlotIndex > 0)
		{
			// 이전 슬롯의 첫 번째 노드 타입 사용 (간소화)
			for (const auto& Pair : RoomGraph)
			{
				if (Pair.Value.SlotIndex == SlotIndex - 1)
				{
					LastType = Pair.Value.RoomType;
					break;
				}
			}
		}

		TArray<FPRRoomNodeInfo> SlotNodes = BuildSlotNodes(SlotIndex, LastType, NextRoomIndex);

		// 첫 슬롯의 노드들은 시작 방
		if (SlotIndex == 0)
		{
			for (const FPRRoomNodeInfo& Node : SlotNodes)
			{
				StartRoomIndices.Add(Node.RoomIndex);
			}
		}

		// 이전 슬롯 노드들의 NextRoomIndices 업데이트
		if (SlotIndex > 0)
		{
			TArray<int32> CurrentSlotIndices;
			for (const FPRRoomNodeInfo& Node : SlotNodes)
			{
				CurrentSlotIndices.Add(Node.RoomIndex);
			}

			for (auto& Pair : RoomGraph)
			{
				if (Pair.Value.SlotIndex == SlotIndex - 1)
				{
					Pair.Value.NextRoomIndices = CurrentSlotIndices;
				}
			}
		}

		// 그래프에 노드 추가
		for (const FPRRoomNodeInfo& Node : SlotNodes)
		{
			RoomGraph.Add(Node.RoomIndex, Node);
		}
	}

	// 마지막 슬롯 노드들에 보스 방 연결
	FPRRoomNodeInfo BossNode;
	BossNode.RoomIndex = NextRoomIndex;
	BossNode.SlotIndex = StageConfig->Slots.Num();
	BossNode.RoomType = EPRRoomType::Boss;
	BossNode.Template = StageConfig->BossMap;
	BossNode.Seed = RandomStream.RandRange(0, MAX_int32);
	RoomGraph.Add(BossNode.RoomIndex, BossNode);

	// 마지막 슬롯 노드들에 보스 방 연결
	for (auto& Pair : RoomGraph)
	{
		if (Pair.Value.SlotIndex == StageConfig->Slots.Num() - 1)
		{
			Pair.Value.NextRoomIndices.Add(BossNode.RoomIndex);
		}
	}

	OnRoomGraphBuilt.Broadcast(StageIndex, StartRoomIndices);

	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Room graph built for stage %d with %d nodes, %d start rooms"), 
		StageIndex, RoomGraph.Num(), StartRoomIndices.Num());
}

TArray<FPRRoomNodeInfo> UPRStageManagerSubsystem::BuildSlotNodes(int32 SlotIndex, EPRRoomType LastSelectedRoomType, int32& OutNextRoomIndex) const
{
	TArray<FPRRoomNodeInfo> Nodes;

	const UPRStageConfigData* StageConfig = GetStageConfig(StageProgress.CurrentStageIndex);
	if (!IsValid(StageConfig))
	{
		return Nodes;
	}

	if (!StageConfig->Slots.IsValidIndex(SlotIndex))
	{
		return Nodes;
	}

	FRandomStream LocalRandom = CreateDeterministicRandom(StageProgress.CurrentStageIndex, SlotIndex, LastSelectedRoomType);
	const FPRRoomSlot& Slot = StageConfig->Slots[SlotIndex];

	// 1. 방 타입 결정 (슬롯당 하나)
	TMap<EPRRoomType, float> Weights = GetModifiedWeightsForLastType(SlotIndex, LastSelectedRoomType);
	EPRRoomType SelectedType = SelectWeightedRandom(Weights, LocalRandom);

	// 2. 해당 방 타입의 보상 카테고리 풀 획득
	const FPRRewardCategoryPool* RewardPool = StageConfig->RewardsByType.Find(SelectedType);

	// 3. 보상 카테고리 선택 (ChoiceCount만큼, 중복 없이)
	TArray<FPRRewardCategoryEntry> SelectedCategories;
	if (RewardPool)
	{
		SelectedCategories = SelectRewardCategories(*RewardPool, Slot.ChoiceCount, LocalRandom);
	}

	// 4. 노드 생성 (같은 방 타입, 다른 보상 카테고리)
	for (int32 i = 0; i < Slot.ChoiceCount; i++)
	{
		FPRRoomNodeInfo Node;
		Node.RoomIndex = OutNextRoomIndex++;
		Node.SlotIndex = SlotIndex;
		Node.RoomType = SelectedType;
		Node.Template = SelectTemplate(SelectedType, LocalRandom);
		Node.Difficulty = Slot.Difficulty;

		if (SelectedCategories.IsValidIndex(i))
		{
			Node.RewardCategory = SelectedCategories[i].Category;
			Node.RewardPoolPreset = SelectedCategories[i].PoolPreset;
		}

		Node.Seed = LocalRandom.RandRange(0, MAX_int32);
		Nodes.Add(Node);
	}

	return Nodes;
}

FRandomStream UPRStageManagerSubsystem::CreateDeterministicRandom(int32 StageIndex, int32 SlotIndex, EPRRoomType LastSelectedRoomType) const
{
	const uint32 SeedStage = HashCombine(GetTypeHash(StageProgress.MasterSeed), GetTypeHash(StageIndex));
	const uint32 SeedSlot = HashCombine(GetTypeHash(SlotIndex), GetTypeHash(static_cast<uint8>(LastSelectedRoomType)));
	const uint32 Seed = HashCombine(SeedStage, SeedSlot);

	return FRandomStream(Seed);
}

TMap<EPRRoomType, float> UPRStageManagerSubsystem::GetModifiedWeightsForLastType(int32 SlotIndex, EPRRoomType LastSelectedRoomType) const
{
	const UPRStageConfigData* StageConfig = GetCurrentStageConfig();
	if (!IsValid(StageConfig) || !StageConfig->Slots.IsValidIndex(SlotIndex))
	{
		return TMap<EPRRoomType, float>();
	}

	const FPRRoomSlot& Slot = StageConfig->Slots[SlotIndex];
	TMap<EPRRoomType, float> Weights = Slot.PossibleTypes;

	// 첫 슬롯이거나 스테이지 시작 시 (None)는 전이 규칙 미적용
	if (LastSelectedRoomType == EPRRoomType::None)
	{
		return Weights;
	}

	// 이전에 선택한 방 타입의 전이 규칙 적용
	if (const FPRRoomTypeTransition* Transition = StageConfig->TypeTransitions.Find(LastSelectedRoomType))
	{
		for (auto& Elem : Weights)
		{
			if (const float* Mod = Transition->WeightModifiers.Find(Elem.Key))
			{
				Elem.Value *= *Mod;
			}
		}
	}

	return Weights;
}

EPRRoomType UPRStageManagerSubsystem::SelectWeightedRandom(const TMap<EPRRoomType, float>& Weights, FRandomStream& Random) const
{
	if (Weights.Num() == 0)
	{
		return EPRRoomType::Combat;
	}

	// TMap을 배열로 변환
	TArray<EPRRoomType> Types;
	TArray<float> WeightArray;
	for (const auto& Elem : Weights)
	{
		Types.Add(Elem.Key);
		WeightArray.Add(Elem.Value);
	}

	const int32 SelectedIndex = PRStageHelpers::SelectWeightedIndex(WeightArray, Random);
	return Types.IsValidIndex(SelectedIndex) ? Types[SelectedIndex] : EPRRoomType::Combat;
}

TArray<FPRRewardCategoryEntry> UPRStageManagerSubsystem::SelectRewardCategories(const FPRRewardCategoryPool& Pool, int32 Count, FRandomStream& Random) const
{
	TArray<FPRRewardCategoryEntry> Result;
	TArray<FPRRewardCategoryEntry> Available = Pool.Categories;

	Count = FMath::Min(Count, Available.Num());

	for (int32 i = 0; i < Count && Available.Num() > 0; i++)
	{
		// 가중치 배열 생성
		TArray<float> Weights;
		for (const auto& Entry : Available)
		{
			Weights.Add(Entry.Weight);
		}

		const int32 SelectedIndex = PRStageHelpers::SelectWeightedIndex(Weights, Random);
		if (!Available.IsValidIndex(SelectedIndex))
		{
			break;
		}

		Result.Add(Available[SelectedIndex]);
		Available.RemoveAt(SelectedIndex);
	}

	return Result;
}

TSoftObjectPtr<UWorld> UPRStageManagerSubsystem::SelectTemplate(EPRRoomType RoomType, FRandomStream& Random) const
{
	const UPRStageConfigData* StageConfig = GetCurrentStageConfig();
	if (!IsValid(StageConfig))
	{
		return nullptr;
	}

	const FPRRoomTemplatePool* Pool = StageConfig->Templates.Find(RoomType);
	if (!Pool || Pool->Entries.Num() == 0)
	{
		return nullptr;
	}

	// 가중치 배열 생성
	TArray<float> Weights;
	for (const auto& Entry : Pool->Entries)
	{
		Weights.Add(Entry.Weight);
	}

	const int32 SelectedIndex = PRStageHelpers::SelectWeightedIndex(Weights, Random);
	return Pool->Entries.IsValidIndex(SelectedIndex) ? Pool->Entries[SelectedIndex].Level : nullptr;
}
