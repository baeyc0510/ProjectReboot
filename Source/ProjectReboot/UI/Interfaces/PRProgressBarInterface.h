#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PRProgressBarInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UPRProgressBarInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 프로그레스 바 어댑터 인터페이스
 * UProgressBar 대신 BP 라이브러리 위젯을 사용하기 위한 공통 인터페이스
 */
class PROJECTREBOOT_API IPRProgressBarInterface
{
	GENERATED_BODY()

public:
	// 프로그레스 바 퍼센트 설정 (0~1)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|ProgressBar")
	void SetPercent(float Value);

	// 프로그레스 바 세그먼트 설정 (NumSegments: 0~100, Spacing: 0~1)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "UI|ProgressBar")
	void SetSegments(int32 NumSegments, float Spacing);
};
