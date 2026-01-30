// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "PRBillboardWidgetComponent.generated.h"

/**
 * 플레이어 카메라 방향으로 자동 회전하는 World Space 위젯 컴포넌트
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class PROJECTREBOOT_API UPRBillboardWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UPRBillboardWidgetComponent();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	// 빌보드 활성화 여부
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard")
	bool bEnableBillboard = true;

	// Pitch 회전도 적용할지 (false면 Yaw만)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Billboard")
	bool bRotatePitch = false;
};
