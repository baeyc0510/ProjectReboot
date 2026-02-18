// Fill out your copyright notice in the Description page of Project Settings.

#include "PRStageManagerSubsystem.h"
#include "PRStageConfigData.h"
#include "PRThemeData.h"
#include "PRRoomWorldSubsystem.h"
#include "RogueliteSubsystem.h"
#include "Engine/AssetManager.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Character/PREnemyCharacter.h"

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

int32 UPRStageManagerSubsystem::GetCurrentRoomStep() const
{
	if (auto NodeInfo = GetCurrentRoomNodeInfo())
	{
		return NodeInfo->StepIndex;
	}
	return 0;
}

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

	// 모든 방 (보스 포함)을 레벨 스트리밍으로 처리
	if (NodeInfo->Template.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("PRStageManagerSubsystem: Cannot enter room %d - template is null"), RoomIndex);
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!IsValid(GameInstance))
	{
		UE_LOG(LogTemp, Error, TEXT("PRStageManagerSubsystem: Cannot enter room - no game instance"));
		return;
	}

	UWorld* World = GameInstance->GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Error, TEXT("PRStageManagerSubsystem: Cannot enter room - no world"));
		return;
	}

	if (UPRRoomWorldSubsystem* RoomSubsystem = World->GetSubsystem<UPRRoomWorldSubsystem>())
	{
		RoomSubsystem->EnterRoom(RoomIndex, *NodeInfo);

		UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Entering room %d (Type: %d, Template: %s)"),
			RoomIndex, static_cast<int32>(NodeInfo->RoomType), *NodeInfo->Template.ToString());
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
	if (!IsValid(StageConfig) || StageConfig->Steps.Num() == 0)
	{
		return;
	}

	int32 NextRoomIndex = 0;

	/*~ 시작 방 생성 (Step 0) ~*/
	FPRRoomNodeInfo StartNode;
	StartNode.RoomIndex = NextRoomIndex++;
	StartNode.StepIndex = 0;
	StartNode.RoomType = EPRRoomType::Default;
	StartNode.Template = StageConfig->StartRoomTemplate;
	StartNode.Seed = RandomStream.RandRange(0, MAX_int32);
	if (const FPRRoomFlowConfig* FlowConfig = StageConfig->FlowConfigs.Find(EPRRoomType::Default))
	{
		StartNode.FlowConfig = *FlowConfig;
	}
	RoomGraph.Add(StartNode.RoomIndex, StartNode);
	StartRoomIndices.Add(StartNode.RoomIndex);

	// 스텝별 RoomIndex 매핑
	TMap<int32, TArray<int32>> StepToRoomIndices;
	StepToRoomIndices.Add(0, { StartNode.RoomIndex });

	/*~ 각 스텝별로 연결 및 노드 생성 ~*/
	// 노드 수는 이전 스텝의 연결에서 자연스럽게 도출됨
	// 연결 시 기존 노드 재사용 또는 새 노드 생성 (최소 1개의 전이 보장)

	for (int32 StepArrayIndex = 0; StepArrayIndex < StageConfig->Steps.Num(); StepArrayIndex++)
	{
		const FPRRoomStep& Step = StageConfig->Steps[StepArrayIndex];
		const int32 CurrentStepIndex = StepArrayIndex;        // 연결 출발점 (0=시작방, 1~N=스텝)
		const int32 NextStepIndex = StepArrayIndex + 1;       // 연결 도착점

		const TArray<int32>* CurrentStepRooms = StepToRoomIndices.Find(CurrentStepIndex);
		if (!CurrentStepRooms)
		{
			continue;
		}

		TArray<int32> NextStepRoomIndices;

		// 현재 스텝의 각 노드에서 다음 스텝으로 연결 생성
		for (int32 CurrentRoomIndex : *CurrentStepRooms)
		{
			FPRRoomNodeInfo* CurrentNode = RoomGraph.Find(CurrentRoomIndex);
			if (!CurrentNode)
			{
				continue;
			}

			// 이 노드의 분기 수 결정 (시작 방은 1개 고정, 나머지는 Min~Max 범위 내 랜덤)
			int32 BranchCount;
			if (CurrentStepIndex == 0)
			{
				BranchCount = 1;
			}
			else
			{
				FRandomStream BranchRandom = CreateDeterministicRandom(StageIndex, CurrentRoomIndex, CurrentNode->RoomType);
				BranchCount = BranchRandom.RandRange(Step.MinBranchCount, Step.MaxBranchCount);
			}

			// 각 분기마다 연결 대상 결정
			// 보상 카테고리 다양성: 가능하면 서로 다른 카테고리를 배정
			TSet<FGameplayTag> UsedCategories;

			for (int32 BranchIndex = 0; BranchIndex < BranchCount; BranchIndex++)
			{
				FRandomStream ConnectionRandom = CreateDeterministicRandom(StageIndex, CurrentRoomIndex * 100 + BranchIndex, CurrentNode->RoomType);

				// 전이 가능한 타입 목록 조회
				TArray<EPRRoomType> ValidTypes;
				TArray<float> ValidWeights;
				GetValidTypesForTransition(CurrentNode->RoomType, Step, ValidTypes, ValidWeights);

				if (ValidTypes.Num() == 0)
				{
					continue;
				}

				// 카테고리가 모두 소진된 타입 제외 (새 카테고리를 제공할 수 없는 타입)
				TArray<EPRRoomType> FilteredTypes;
				TArray<float> FilteredWeights;
				for (int32 i = 0; i < ValidTypes.Num(); i++)
				{
					if (HasAvailableCategory(ValidTypes[i], UsedCategories))
					{
						FilteredTypes.Add(ValidTypes[i]);
						FilteredWeights.Add(ValidWeights[i]);
					}
				}

				// 필터링 후 타입이 없으면 제약 완화 (전체 타입 사용)
				if (FilteredTypes.Num() == 0)
				{
					FilteredTypes = ValidTypes;
					FilteredWeights = ValidWeights;
				}

				// 타입 선택
				const int32 TypeIndex = PRStageHelpers::SelectWeightedIndex(FilteredWeights, ConnectionRandom);
				if (!FilteredTypes.IsValidIndex(TypeIndex))
				{
					continue;
				}
				const EPRRoomType SelectedType = FilteredTypes[TypeIndex];

				// 기존 노드 재사용 시도: 카테고리가 중복되지 않는 노드 우선
				int32 TargetRoomIndex = INDEX_NONE;
				for (int32 ExistingRoomIndex : NextStepRoomIndices)
				{
					if (const FPRRoomNodeInfo* ExistingNode = RoomGraph.Find(ExistingRoomIndex))
					{
						if (ExistingNode->RoomType == SelectedType &&
							!UsedCategories.Contains(ExistingNode->RewardCategory))
						{
							if (ConnectionRandom.FRand() < 0.5f)
							{
								TargetRoomIndex = ExistingRoomIndex;
								break;
							}
						}
					}
				}

				// 재사용 불가 시 새 노드 생성
				if (TargetRoomIndex == INDEX_NONE)
				{
					FRandomStream NodeRandom = CreateDeterministicRandom(StageIndex, NextStepIndex * 1000 + NextStepRoomIndices.Num(), SelectedType);
					FPRRoomNodeInfo NewNode = CreateRoomNode(NextRoomIndex, NextStepIndex, SelectedType, Step.Difficulty, NodeRandom, UsedCategories);

					// 동일 타입+카테고리 노드가 이미 존재하면 재사용
					for (int32 ExistingRoomIndex : NextStepRoomIndices)
					{
						if (const FPRRoomNodeInfo* ExistingNode = RoomGraph.Find(ExistingRoomIndex))
						{
							if (ExistingNode->RoomType == NewNode.RoomType &&
								ExistingNode->RewardCategory == NewNode.RewardCategory)
							{
								TargetRoomIndex = ExistingRoomIndex;
								break;
							}
						}
					}

					if (TargetRoomIndex == INDEX_NONE)
					{
						NextRoomIndex++;
						RoomGraph.Add(NewNode.RoomIndex, NewNode);
						NextStepRoomIndices.Add(NewNode.RoomIndex);
						TargetRoomIndex = NewNode.RoomIndex;
					}
				}

				// 연결 추가 (중복 방지)
				if (const FPRRoomNodeInfo* TargetNode = RoomGraph.Find(TargetRoomIndex))
				{
					UsedCategories.Add(TargetNode->RewardCategory);
				}

				if (!CurrentNode->NextRoomIndices.Contains(TargetRoomIndex))
				{
					CurrentNode->NextRoomIndices.Add(TargetRoomIndex);
				}
			}
		}

		StepToRoomIndices.Add(NextStepIndex, NextStepRoomIndices);
	}

	/*~ 보스 방 생성 및 연결 (Step N+1) ~*/
	const int32 LastStepIndex = StageConfig->Steps.Num();
	const int32 BossStepIndex = LastStepIndex + 1;

	FPRRoomNodeInfo BossNode;
	BossNode.RoomIndex = NextRoomIndex++;
	BossNode.StepIndex = BossStepIndex;
	BossNode.RoomType = EPRRoomType::Boss;
	BossNode.Template = StageConfig->BossMap;
	BossNode.Seed = RandomStream.RandRange(0, MAX_int32);
	if (const FPRRoomFlowConfig* FlowConfig = StageConfig->FlowConfigs.Find(EPRRoomType::Boss))
	{
		BossNode.FlowConfig = *FlowConfig;
	}
	RoomGraph.Add(BossNode.RoomIndex, BossNode);

	// 마지막 스텝의 모든 노드를 보스 방에 연결
	const TArray<int32>* LastStepRooms = StepToRoomIndices.Find(LastStepIndex);
	if (LastStepRooms)
	{
		for (int32 RoomIndex : *LastStepRooms)
		{
			if (FPRRoomNodeInfo* Node = RoomGraph.Find(RoomIndex))
			{
				Node->NextRoomIndices.Add(BossNode.RoomIndex);
			}
		}
	}

	OnRoomGraphBuilt.Broadcast(StageIndex, StartRoomIndices);

	UE_LOG(LogTemp, Log, TEXT("PRStageManagerSubsystem: Room graph built for stage %d with %d nodes"), 
		StageIndex, RoomGraph.Num());
}

FRandomStream UPRStageManagerSubsystem::CreateDeterministicRandom(int32 StageIndex, int32 StepIndex, EPRRoomType LastSelectedRoomType) const
{
	const uint32 SeedStage = HashCombine(GetTypeHash(StageProgress.MasterSeed), GetTypeHash(StageIndex));
	const uint32 SeedStep = HashCombine(GetTypeHash(StepIndex), GetTypeHash(static_cast<uint8>(LastSelectedRoomType)));
	const uint32 Seed = HashCombine(SeedStage, SeedStep);

	return FRandomStream(Seed);
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

TArray<FPRRewardCategoryEntry> UPRStageManagerSubsystem::SelectRewardCategories(const FPRRewardCategoryPool& Pool, int32 Count, FRandomStream& Random, const TSet<FGameplayTag>& ExcludedCategories) const
{
	TArray<FPRRewardCategoryEntry> Result;

	// 제외 카테고리를 필터링한 후보 목록 생성
	TArray<FPRRewardCategoryEntry> Available;
	for (const FPRRewardCategoryEntry& Entry : Pool.Categories)
	{
		if (!ExcludedCategories.Contains(Entry.Category))
		{
			Available.Add(Entry);
		}
	}

	// 필터링 후 후보가 없으면 제약 완화 (전체 풀 사용)
	if (Available.Num() == 0)
	{
		Available = Pool.Categories;
	}

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

FPRRoomNodeInfo UPRStageManagerSubsystem::CreateRoomNode(int32 RoomIndex, int32 StepIndex, EPRRoomType RoomType, float Difficulty, FRandomStream& Random, const TSet<FGameplayTag>& ExcludedCategories) const
{
	const UPRStageConfigData* StageConfig = GetCurrentStageConfig();

	FPRRoomNodeInfo Node;
	Node.RoomIndex = RoomIndex;
	Node.StepIndex = StepIndex;
	Node.RoomType = RoomType;
	Node.Template = SelectTemplate(RoomType, Random);
	Node.Difficulty = Difficulty;
	Node.Seed = Random.RandRange(0, MAX_int32);

	// 보상 카테고리 설정 (제외 카테고리 필터링 적용)
	if (IsValid(StageConfig))
	{
		if (const FPRRewardCategoryPool* RewardPool = StageConfig->RewardsByType.Find(RoomType))
		{
			TArray<FPRRewardCategoryEntry> SelectedCategories = SelectRewardCategories(*RewardPool, 1, Random, ExcludedCategories);
			if (SelectedCategories.Num() > 0)
			{
				Node.RewardCategory = SelectedCategories[0].Category;
				Node.RewardPoolPreset = SelectedCategories[0].PoolPreset;
			}
		}

		// FlowConfig 주입
		if (const FPRRoomFlowConfig* FlowConfig = StageConfig->FlowConfigs.Find(RoomType))
		{
			Node.FlowConfig = *FlowConfig;
		}

		// SpawnInfo 생성
		if (const FPRRoomSpawnConfig* SpawnConfig = StageConfig->SpawnConfigs.Find(RoomType))
		{
			Node.SpawnInfo = CreateSpawnInfo(*SpawnConfig, RoomType, Difficulty, Random);
		}
	}

	return Node;
}

float UPRStageManagerSubsystem::GetTransitionWeight(EPRRoomType FromType, EPRRoomType ToType) const
{
	const UPRStageConfigData* StageConfig = GetCurrentStageConfig();
	if (!IsValid(StageConfig))
	{
		return 1.0f;
	}

	if (const FPRRoomTypeTransition* Transition = StageConfig->TypeTransitions.Find(FromType))
	{
		if (const float* Modifier = Transition->WeightModifiers.Find(ToType))
		{
			return *Modifier;
		}
	}

	return 1.0f; // 기본 가중치
}

void UPRStageManagerSubsystem::GetValidTypesForTransition(EPRRoomType FromType, const FPRRoomStep& Step, TArray<EPRRoomType>& OutTypes, TArray<float>& OutWeights) const
{
	OutTypes.Empty();
	OutWeights.Empty();

	const UPRStageConfigData* StageConfig = GetCurrentStageConfig();
	if (!IsValid(StageConfig))
	{
		return;
	}

	// 전이 규칙에서 유효한 타입 추출 (가중치 > 0 && PossibleTypes에 포함)
	if (const FPRRoomTypeTransition* Transition = StageConfig->TypeTransitions.Find(FromType))
	{
		for (const auto& Pair : Transition->WeightModifiers)
		{
			if (Pair.Value > 0.0f && Step.PossibleTypes.Contains(Pair.Key))
			{
				OutTypes.Add(Pair.Key);
				OutWeights.Add(Pair.Value);
			}
		}
	}

	// 유효 타입이 없으면 PossibleTypes 전체 사용
	if (OutTypes.Num() == 0)
	{
		for (const auto& Pair : Step.PossibleTypes)
		{
			OutTypes.Add(Pair.Key);
			OutWeights.Add(Pair.Value);
		}
	}
}

bool UPRStageManagerSubsystem::HasAvailableCategory(EPRRoomType RoomType, const TSet<FGameplayTag>& UsedCategories) const
{
	const UPRStageConfigData* StageConfig = GetCurrentStageConfig();
	if (!IsValid(StageConfig))
	{
		return true;
	}

	const FPRRewardCategoryPool* Pool = StageConfig->RewardsByType.Find(RoomType);
	if (!Pool || Pool->Categories.Num() == 0)
	{
		// 카테고리 풀이 없는 타입은 카테고리 제약 없이 사용 가능
		return true;
	}

	for (const FPRRewardCategoryEntry& Entry : Pool->Categories)
	{
		if (!UsedCategories.Contains(Entry.Category))
		{
			return true;
		}
	}

	return false;
}

FPRRoomSpawnInfo UPRStageManagerSubsystem::CreateSpawnInfo(const FPRRoomSpawnConfig& Config, EPRRoomType RoomType, float Difficulty, FRandomStream& Random) const
{
	FPRRoomSpawnInfo SpawnInfo;

	const UPRStageConfigData* StageConfig = GetCurrentStageConfig();
	if (!IsValid(StageConfig) || !IsValid(StageConfig->ThemeData))
	{
		return SpawnInfo;
	}

	const UPRThemeData* ThemeData = StageConfig->ThemeData;

	// 웨이브 수 결정
	const int32 WaveCount = Random.RandRange(Config.MinWaveCount, Config.MaxWaveCount);

	for (int32 WaveIndex = 0; WaveIndex < WaveCount; WaveIndex++)
	{
		FPRWaveSpawnInfo WaveInfo;

		// 일반 적 수 결정 및 선택
		const int32 NormalCount = Random.RandRange(Config.MinNormalEnemies, Config.MaxNormalEnemies);
		for (int32 i = 0; i < NormalCount; i++)
		{
			if (TSubclassOf<APREnemyCharacter> EnemyClass = SelectEnemyFromPool(ThemeData->NormalEnemies, Difficulty, Random))
			{
				WaveInfo.EnemySpawnMap.FindOrAdd(EnemyClass) += 1;
			}
		}

		// 엘리트 적 수 결정 및 선택
		const int32 EliteCount = Random.RandRange(Config.MinEliteEnemies, Config.MaxEliteEnemies);
		for (int32 i = 0; i < EliteCount; i++)
		{
			if (TSubclassOf<APREnemyCharacter> EnemyClass = SelectEnemyFromPool(ThemeData->EliteEnemies, Difficulty, Random))
			{
				WaveInfo.EnemySpawnMap.FindOrAdd(EnemyClass) += 1;
			}
		}

		// 미니보스 (첫 웨이브에만)
		if (WaveIndex == 0)
		{
			const int32 MiniBossCount = Random.RandRange(Config.MinMiniBosses, Config.MaxMiniBosses);
			for (int32 i = 0; i < MiniBossCount; i++)
			{
				if (TSubclassOf<APREnemyCharacter> EnemyClass = SelectEnemyFromPool(ThemeData->MiniBosses, Difficulty, Random))
				{
					WaveInfo.EnemySpawnMap.FindOrAdd(EnemyClass) += 1;
				}
			}
		}

		SpawnInfo.Waves.Add(WaveInfo);
	}

	return SpawnInfo;
}

TSubclassOf<APREnemyCharacter> UPRStageManagerSubsystem::SelectEnemyFromPool(const TArray<FPREnemySpawnEntry>& Pool, float Difficulty, FRandomStream& Random) const
{
	// 난이도 필터링
	TArray<FPREnemySpawnEntry> ValidEntries;
	for (const FPREnemySpawnEntry& Entry : Pool)
	{
		// MinDifficulty 체크
		if (Difficulty < Entry.MinDifficulty)
		{
			continue;
		}

		// MaxDifficulty 체크 (0 = 무제한)
		if (Entry.MaxDifficulty > 0.0f && Difficulty > Entry.MaxDifficulty)
		{
			continue;
		}

		ValidEntries.Add(Entry);
	}

	if (ValidEntries.Num() == 0)
	{
		return nullptr;
	}

	// 가중치 배열 생성
	TArray<float> Weights;
	for (const FPREnemySpawnEntry& Entry : ValidEntries)
	{
		Weights.Add(Entry.Weight);
	}

	const int32 SelectedIndex = PRStageHelpers::SelectWeightedIndex(Weights, Random);
	if (ValidEntries.IsValidIndex(SelectedIndex))
	{
		return ValidEntries[SelectedIndex].EnemyClass;
	}

	return nullptr;
}
