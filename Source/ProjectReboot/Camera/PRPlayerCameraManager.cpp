// PRPlayerCameraManager.cpp
#include "PRPlayerCameraManager.h"
#include "PRCameraComponent.h"
#include "AbilitySystemComponent.h"

void APRPlayerCameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromASC();
	Super::EndPlay(EndPlayReason);
}

void APRPlayerCameraManager::BindToASC(UAbilitySystemComponent* InASC)
{
	UnbindFromASC();
	UPRCameraComponent* CameraComp = FindPRCameraComponent();
	if (!IsValid(InASC) || !CameraComp || !CameraComp->GetConfig())
	{
		return;
	}

	BoundASC = InASC;
	
	FGameplayTagContainer AllConditionTags;
	for (const FPRCameraTagSetting& Setting : CameraComp->GetConfig()->CameraSettings)
	{
		AllConditionTags.AppendTags(Setting.ConditionTags);
	}
	
	for (auto& ConditionTag : AllConditionTags)
	{
		FDelegateHandle Handle = InASC->RegisterGameplayTagEvent(ConditionTag, EGameplayTagEventType::NewOrRemoved).
									AddUObject(this, &APRPlayerCameraManager::HandleStateTagChanged);
		TagEventHandles.Add({Handle, ConditionTag});
	}
	
	ReevaluateActiveSetting();
}

void APRPlayerCameraManager::UnbindFromASC()
{
	if (BoundASC.IsValid())
	{
		for (auto& H : TagEventHandles)
		{
			BoundASC->UnregisterGameplayTagEvent(H.Handle, H.Tag, EGameplayTagEventType::NewOrRemoved);
		}
	}
	TagEventHandles.Empty();
	BoundASC.Reset();
	ActiveConditionTags.Reset();
}

UPRCameraComponent* APRPlayerCameraManager::FindPRCameraComponent() const
{
	return GetViewTarget() ? GetViewTarget()->FindComponentByClass<UPRCameraComponent>() : nullptr;
}

void APRPlayerCameraManager::HandleStateTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	ReevaluateActiveSetting();
}

void APRPlayerCameraManager::ReevaluateActiveSetting()
{
	UPRCameraComponent* CameraComp = FindPRCameraComponent();
	if (!BoundASC.IsValid() || !CameraComp || !CameraComp->GetConfig())
	{
		return;
	}

	FGameplayTagContainer OwnedTags;
	BoundASC->GetOwnedGameplayTags(OwnedTags);

	const FPRCameraTagSetting* BestSetting = CameraComp->GetConfig()->GetHighestPrioritySetting(OwnedTags);
	FGameplayTagContainer NewConditionTags = BestSetting ? BestSetting->ConditionTags : FGameplayTagContainer();

	// 이전 상태와 비교하여 변경되었는지 확인 (컨테이너 간 일치 여부 확인)
	if (!NewConditionTags.HasAllExact(ActiveConditionTags) || NewConditionTags.Num() != ActiveConditionTags.Num())
	{
		// 상태 업데이트
		ActiveConditionTags = NewConditionTags;

		if (BestSetting)
		{
			// 새로운 복합 태그 설정으로 전환 실행
			CameraComp->SetTargetSetting(
				BestSetting->CameraSetting, 
				BestSetting->TransitionDuration, 
				BestSetting->TransitionCurve
			);
		}
		else
		{
			// 일치하는 조건이 없으면 기본 설정으로 복귀
			CameraComp->ResetToDefault(
				CameraComp->GetConfig()->DefaultTransitionDuration, 
				CameraComp->GetConfig()->DefaultTransitionCurve
			);
		}
	}
}
