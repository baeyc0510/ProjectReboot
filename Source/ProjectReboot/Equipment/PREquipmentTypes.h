// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "NativeGameplayTags.h"

#include "PREquipmentTypes.generated.h"

/*~ Equipment Tags ~*/
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_EQUIPMENT);

/*~ 전방 선언 ~*/
class UStaticMesh;
class USkeletalMesh;

// 장비 부착 소켓 정보
USTRUCT(BlueprintType)
struct FEquipmentAttachmentInfo
{
	GENERATED_BODY()

	// 부착할 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FName SocketName;

	// 위치 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FVector LocationOffset = FVector::ZeroVector;

	// 회전 오프셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FRotator RotationOffset = FRotator::ZeroRotator;

	// 스케일
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attachment")
	FVector Scale = FVector::OneVector;
};

UENUM(BlueprintType)
enum class EEquipmentMeshType : uint8
{
	SkeletalMesh,
	StaticMesh,
};

// 메시 스폰 정보
USTRUCT(BlueprintType)
struct FEquipmentMeshSpawnInfo
{
	GENERATED_BODY()

	// 메시 타입
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	EEquipmentMeshType MeshType = EEquipmentMeshType::SkeletalMesh;
	
	// 스태틱 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (EditCondition = "MeshType == EEquipmentMeshType::StaticMesh"))
	TObjectPtr<UStaticMesh> StaticMesh;

	// 스켈레탈 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (EditCondition = "MeshType == EEquipmentMeshType::SkeletalMesh"))
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	// 부착 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FEquipmentAttachmentInfo AttachmentInfo;

	// TODO: 머티리얼 오버라이드 (인덱스별)
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	// TMap<int32, TObjectPtr<UMaterialInterface>> MaterialOverrides;
	
public:
	bool IsValid() const
	{
		return StaticMesh != nullptr || SkeletalMesh != nullptr;
	}
};

USTRUCT(BlueprintType)
struct FRuleBasedMeshVisualSetting
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	int32 Priority;
	
	// EquipmentInstance의 태그가 RequiredTags를 모두 충족할 때 이 세팅을 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FGameplayTagContainer RequiredTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FEquipmentMeshSpawnInfo MeshSpawnInfo;
};

USTRUCT(BlueprintType)
struct FEquipmentVisualSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FEquipmentMeshSpawnInfo DefaultMeshSpawnInfo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	bool bUseRuleBasedMeshSpawnInfo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh", meta = (EditCondition = "bUseRuleBasedMeshSpawnInfo"))
	TArray<FRuleBasedMeshVisualSetting> RuleBasedMeshSpawnSettings;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSubclassOf<UAnimInstance>> AnimLayersToLink;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipMontage;
	// TODO: 장착 VFX?
};