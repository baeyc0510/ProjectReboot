// Fill out your copyright notice in the Description page of Project Settings.


#include "PRBlueprintFunctionLibrary.h"

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
