// Fill out your copyright notice in the Description page of Project Settings.

#include "PRANS_LoopSound.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectReboot/Character/PRCharacterBase.h"

UPRANS_LoopSound::UPRANS_LoopSound()
{
	bShouldFireInEditor = true;
}

void UPRANS_LoopSound::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!IsValid(MeshComp) || !IsValid(Sound))
	{
		return;
	}

#if WITH_EDITORONLY_DATA
	UWorld* World = MeshComp->GetWorld();
	if (IsValid(World) && World->WorldType == EWorldType::EditorPreview)
	{
		// 기존 프리뷰 오디오 정리 (NotifyEnd 없이 재호출될 수 있음)
		if (PreviewAudioComponent.IsValid())
		{
			PreviewAudioComponent->Stop();
			PreviewAudioComponent.Reset();
		}

		if (bPreviewIgnoreAttenuation)
		{
			PreviewAudioComponent = UGameplayStatics::SpawnSound2D(World, Sound, VolumeMultiplier);
		}
		else
		{
			PreviewAudioComponent = UGameplayStatics::SpawnSoundAttached(
				Sound, MeshComp, NAME_None,
				FVector::ZeroVector, EAttachLocation::KeepRelativeOffset,
				false, VolumeMultiplier);
		}
		return;
	}
#endif

	// 런타임: 캐릭터에 위임
	if (APRCharacterBase* Character = Cast<APRCharacterBase>(MeshComp->GetOwner()))
	{
		if (SoundTag.IsValid())
		{
			Character->PlayLoopSound(SoundTag, Sound);
		}
	}
}

void UPRANS_LoopSound::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (!IsValid(MeshComp))
	{
		return;
	}

#if WITH_EDITORONLY_DATA
	UWorld* World = MeshComp->GetWorld();
	if (IsValid(World) && World->WorldType == EWorldType::EditorPreview)
	{
		if (PreviewAudioComponent.IsValid())
		{
			if (FadeOutDuration > 0.0f)
			{
				PreviewAudioComponent->FadeOut(FadeOutDuration, 0.0f);
			}
			else
			{
				PreviewAudioComponent->Stop();
			}
			PreviewAudioComponent.Reset();
		}
		return;
	}
#endif

	// 런타임: 캐릭터에 위임
	if (APRCharacterBase* Character = Cast<APRCharacterBase>(MeshComp->GetOwner()))
	{
		if (SoundTag.IsValid())
		{
			Character->StopLoopSound(SoundTag, FadeOutDuration);
		}
	}
}

FString UPRANS_LoopSound::GetNotifyName_Implementation() const
{
	if (IsValid(Sound))
	{
		return FString::Printf(TEXT("Loop Sound: %s"), *Sound->GetName());
	}
	return TEXT("Loop Sound");
}
