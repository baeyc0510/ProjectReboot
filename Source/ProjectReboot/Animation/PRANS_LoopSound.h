// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "PRANS_LoopSound.generated.h"

class USoundBase;
class UAudioComponent;

/**
 * 루프 사운드 재생 AnimNotifyState
 * 캐릭터의 PlayLoopSound/StopLoopSound에 위임
 * 에디터 프리뷰에서도 재생 지원
 */
UCLASS(meta = (DisplayName = "PR Loop Sound"))
class PROJECTREBOOT_API UPRANS_LoopSound : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPRANS_LoopSound();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

protected:
	// 재생할 사운드
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundBase> Sound;

	// 사운드 식별용 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	FGameplayTag SoundTag;

	// 볼륨 배율
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0"))
	float VolumeMultiplier = 1.0f;

	// 페이드아웃 시간 (0이면 즉시 정지)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound", meta = (ClampMin = "0.0"))
	float FadeOutDuration = 0.5f;

#if WITH_EDITORONLY_DATA
	// 에디터 프리뷰에서 감쇠 무시
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	bool bPreviewIgnoreAttenuation = true;
#endif

private:
	// 에디터 프리뷰용 오디오 컴포넌트
	UPROPERTY(Transient)
	TWeakObjectPtr<UAudioComponent> PreviewAudioComponent;
};
