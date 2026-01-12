// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PREquipmentTypes.generated.h"


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

// 메시 스폰 정보
USTRUCT(BlueprintType)
struct FEquipmentMeshSpawnInfo
{
	GENERATED_BODY()

	// 스태틱 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TObjectPtr<UStaticMesh> StaticMesh;

	// 스켈레탈 메시
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	// 부착 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FEquipmentAttachmentInfo AttachmentInfo;

	// 머티리얼 오버라이드 (인덱스별)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	TMap<int32, TObjectPtr<UMaterialInterface>> MaterialOverrides;
};

USTRUCT(BlueprintType)
struct FEquipmentVisualSettings
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FEquipmentMeshSpawnInfo MeshSpawnInfo;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TArray<TSubclassOf<UAnimInstance>> AnimLayersToLink;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> EquipMontage;
	// TODO: 장착 SFX 혹은 VFX?
};