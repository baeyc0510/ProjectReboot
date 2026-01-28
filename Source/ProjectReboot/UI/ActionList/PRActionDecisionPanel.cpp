// Fill out your copyright notice in the Description page of Project Settings.

#include "PRActionDecisionPanel.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "PRActionListItemWidget.h"
#include "RogueliteBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Camera/CameraActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Equipment/PREquipmentBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/UI/PRUIBlueprintLibrary.h"

void UPRActionDecisionPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked.AddDynamic(this, &UPRActionDecisionPanel::HandleConfirmButtonClicked);
		ConfirmButton->SetIsEnabled(false);
	}
}

void UPRActionDecisionPanel::NativeDestruct()
{
	RestoreCamera();
	CleanupPreviewCamera();
	RestoreOriginalEquipment();
	UnlockPlayerState();
	HandleOtherUIVisibility(true);

	Super::NativeDestruct();
}

void UPRActionDecisionPanel::SetSourceActor(AActor* InActor)
{
	SourceActor = InActor;

	// 원래 장비 상태 저장
	SaveOriginalEquipmentState();

	// 카메라 설정 및 활성화
	SetupPreviewCamera();
	ActivatePreviewCamera();

	// 플레이어 상태 잠금 (회전 고정 + 카메라 향해 회전)
	LockPlayerState();
	
	// 크로스 헤어 등 UI 감춤
	HandleOtherUIVisibility(false);
}

void UPRActionDecisionPanel::SetDecisionList(const TArray<URogueliteActionData*>& ActionList)
{
	ClearDecisionList();

	if (!DecisionListBox || !DecisionItemWidgetClass)
	{
		return;
	}

	for (URogueliteActionData* ActionData : ActionList)
	{
		if (!ActionData)
		{
			continue;
		}

		UPRActionListItemWidget* ItemWidget = CreateWidget<UPRActionListItemWidget>(this, DecisionItemWidgetClass);
		if (!ItemWidget)
		{
			continue;
		}

		ItemWidget->InitWidget(ActionData);
		ItemWidget->OnClicked.AddDynamic(this, &UPRActionDecisionPanel::HandleItemClicked);

		DecisionListBox->AddChild(ItemWidget);
		ItemWidgets.Add(ItemWidget);
	}
}

void UPRActionDecisionPanel::ClearDecisionList()
{
	// 선택 해제
	if (SelectedItem)
	{
		DeselectItem(SelectedItem);
	}

	SelectedItem = nullptr;
	UpdateConfirmButtonState();

	// 위젯 제거
	if (DecisionListBox)
	{
		DecisionListBox->ClearChildren();
	}

	ItemWidgets.Empty();
}

URogueliteActionData* UPRActionDecisionPanel::GetSelectedActionData() const
{
	return SelectedItem ? SelectedItem->GetActionData() : nullptr;
}

void UPRActionDecisionPanel::HandleItemClicked(UPRActionListItemWidget* ClickedItem)
{
	if (!ClickedItem)
	{
		return;
	}

	// 이미 선택된 아이템을 다시 클릭하면 선택 해제
	if (SelectedItem == ClickedItem)
	{
		DeselectItem(ClickedItem);
		SelectedItem = nullptr;
		UpdateConfirmButtonState();
		return;
	}

	// 기존 선택 해제
	if (SelectedItem)
	{
		DeselectItem(SelectedItem);
	}

	// 새 아이템 선택
	SelectItem(ClickedItem);
	SelectedItem = ClickedItem;
	UpdateConfirmButtonState();
}

void UPRActionDecisionPanel::HandleConfirmButtonClicked()
{
	RestoreOriginalEquipment();
	
	if (SelectedItem)
	{
		if (URogueliteActionData* ActionToAcquire = SelectedItem->GetActionData())
		{
			URogueliteBlueprintLibrary::AcquireAction(this, ActionToAcquire);
		}
	}

	// 장비 확정 플래그 설정 (NativeDestruct에서 복원하지 않음)
	bEquipmentConfirmed = true;

	UPRUIBlueprintLibrary::PopUI(GetOwningPlayer(), this);

	if (IsInViewport())
	{
		RemoveFromParent();
	}
}

void UPRActionDecisionPanel::SelectItem(UPRActionListItemWidget* Item)
{
	if (!Item)
	{
		return;
	}

	Item->SetSelected(true);

	// EquipAction인 경우 장비 장착
	if (UPREquipActionData* EquipAction = Cast<UPREquipActionData>(Item->GetActionData()))
	{
		HandleEquipAction(EquipAction, true);
	}
}

void UPRActionDecisionPanel::DeselectItem(UPRActionListItemWidget* Item)
{
	if (!Item)
	{
		return;
	}

	Item->SetSelected(false);

	// EquipAction인 경우 장비 해제
	if (UPREquipActionData* EquipAction = Cast<UPREquipActionData>(Item->GetActionData()))
	{
		HandleEquipAction(EquipAction, false);
	}
}

void UPRActionDecisionPanel::HandleEquipAction(UPREquipActionData* EquipAction, bool bEquip)
{
	if (!IsValid(SourceActor) || !IsValid(EquipAction))
	{
		return;
	}
	
	UPREquipmentManagerComponent* EquipmentManager = UPREquipmentBlueprintLibrary::GetEquipmentManager(SourceActor);
	if (!IsValid(EquipmentManager))
	{
		return;
	}

	if (bEquip)
	{
		// 새 장비 미리보기 적용
		UPREquipmentBlueprintLibrary::OverrideEquipAction(SourceActor, EquipAction);
	}
	else
	{
		// 선택 해제 시 원래 장비로 복원
		RestoreSlotToOriginal(EquipAction->EquipmentSlot);
	}
}

void UPRActionDecisionPanel::UpdateConfirmButtonState()
{
	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(SelectedItem != nullptr);
	}
}

/*~ 카메라 관리 ~*/

void UPRActionDecisionPanel::SetupPreviewCamera()
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(SourceActor))
	{
		return;
	}

	// 카메라 액터 스폰
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PreviewCameraActor = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), SpawnParams);
	if (!PreviewCameraActor)
	{
		return;
	}

	// 플레이어 정면 + 측면 오프셋 위치에 카메라 배치
	FVector PlayerLocation = SourceActor->GetActorLocation();
	FRotator PlayerRotation = SourceActor->GetActorRotation();

	// 카메라 방향: 플레이어 정면 + Yaw 오프셋
	FRotator CameraDirection = PlayerRotation;
	CameraDirection.Yaw += CameraYawOffset;

	FVector CameraOffset = CameraDirection.Vector() * CameraDistance;
	CameraOffset.Z = CameraHeightOffset;

	FVector CameraLocation = PlayerLocation + CameraOffset;

	// 화면 좌우 오프셋 적용
	const FVector ScreenOffset = PlayerRotation.Quaternion().GetRightVector() * ScreenHorizontalOffset;
	const FVector LookAtTarget = PlayerLocation + FVector(0, 0, CameraHeightOffset * 0.5f) + ScreenOffset;
	FRotator CameraRotation = (LookAtTarget - CameraLocation).Rotation();

	PreviewCameraActor->SetActorLocationAndRotation(CameraLocation, CameraRotation);
}

void UPRActionDecisionPanel::ActivatePreviewCamera()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC || !PreviewCameraActor)
	{
		return;
	}

	OriginalViewTarget = PC->GetViewTarget();
	PC->SetViewTargetWithBlend(PreviewCameraActor, BlendTime, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
}

void UPRActionDecisionPanel::RestoreCamera()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	if (OriginalViewTarget.IsValid())
	{
		PC->SetViewTargetWithBlend(OriginalViewTarget.Get(), BlendTime, EViewTargetBlendFunction::VTBlend_Cubic, 2.0f);
	}
}

void UPRActionDecisionPanel::CleanupPreviewCamera(bool bImmediate)
{
	if (!IsValid(PreviewCameraActor))
	{
		return;
	}

	if (!bImmediate && BlendTime > 0.0f)
	{
		PreviewCameraActor->SetLifeSpan(BlendTime);
		PreviewCameraActor = nullptr;
		return;
	}

	PreviewCameraActor->Destroy();
	PreviewCameraActor = nullptr;
}

/*~ 장비 관리 ~*/

void UPRActionDecisionPanel::SaveOriginalEquipmentState()
{
	OriginalEquipmentState.Empty();
	if (!IsValid(SourceActor))
	{
		return;
	}

	UPREquipmentManagerComponent* EquipmentManager = UPREquipmentBlueprintLibrary::GetEquipmentManager(SourceActor);
	if (!EquipmentManager)
	{
		return;
	}

	TArray<UPREquipActionData*> AllActions = EquipmentManager->GetAllActionData();
	for (auto Action : AllActions)
	{
		OriginalEquipmentState.Add(Action->EquipmentSlot, Action);
	}
}

void UPRActionDecisionPanel::RestoreOriginalEquipment()
{
	// 확정되었다면 복원하지 않음
	if (bEquipmentConfirmed || !IsValid(SourceActor))
	{
		return;
	}
	
	if (!SelectedItem)
	{
		return;
	}
	
	if (UPREquipActionData* EquipAction = Cast<UPREquipActionData>(SelectedItem->GetActionData()))
	{
		RestoreSlotToOriginal(EquipAction->EquipmentSlot);
	}
}

void UPRActionDecisionPanel::RestoreSlotToOriginal(FGameplayTag SlotTag)
{
	if (!IsValid(SourceActor))
	{
		return;
	}

	UPREquipmentManagerComponent* EquipmentManager = SourceActor->GetComponentByClass<UPREquipmentManagerComponent>();
	if (!EquipmentManager)
	{
		return;
	}
	
	// 원래 장비가 있었다면 복원
	if (UPREquipActionData** OriginalAction = OriginalEquipmentState.Find(SlotTag))
	{
		if (IsValid(*OriginalAction))
		{
			UPREquipmentBlueprintLibrary::OverrideEquipAction(SourceActor, *OriginalAction);
		}
	}
	// 현재 슬롯의 장비 해제
	else
	{
		EquipmentManager->Unequip(SlotTag);	
	}
}

/*~ 플레이어 상태 관리 ~*/

void UPRActionDecisionPanel::LockPlayerState()
{
	if (!IsValid(SourceActor))
	{
		return;
	}

	OriginalActorRotation = SourceActor->GetActorRotation();

	if (ACharacter* Character = Cast<ACharacter>(SourceActor))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			bOriginalOrientRotationToMovement = MovementComp->bOrientRotationToMovement;
			MovementComp->bOrientRotationToMovement = false;
		}
	}

	// 카메라를 향해 캐릭터 회전
	if (PreviewCameraActor)
	{
		FVector ToCamera = PreviewCameraActor->GetActorLocation() - SourceActor->GetActorLocation();
		ToCamera.Z = 0;
		FRotator LookAtRotation = ToCamera.Rotation();
		SourceActor->SetActorRotation(LookAtRotation);
	}
}

void UPRActionDecisionPanel::UnlockPlayerState()
{
	if (!IsValid(SourceActor))
	{
		return;
	}

	if (ACharacter* Character = Cast<ACharacter>(SourceActor))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			MovementComp->bOrientRotationToMovement = bOriginalOrientRotationToMovement;
		}
	}
}

void UPRActionDecisionPanel::HandleOtherUIVisibility(bool bIsVisible)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	if (!ASC)
	{
		return;
	}
	
	if (bIsVisible)
	{
		ASC->RemoveLooseGameplayTag(TAG_State_UI_HideCrosshair);
	}
	else
	{
		ASC->AddLooseGameplayTag(TAG_State_UI_HideCrosshair);
	}
}
