// Fill out your copyright notice in the Description page of Project Settings.

#include "PRActionDecisionPanel.h"

#include "PRActionListItemWidget.h"
#include "RogueliteBlueprintLibrary.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "ProjectReboot/PRGameplayTags.h"
#include "ProjectReboot/Game/PRGameplayGameState.h"
#include "ProjectReboot/Camera/PRCameraBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipmentBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"
#include "ProjectReboot/Equipment/PREquipmentManagerComponent.h"
#include "ProjectReboot/UI/PRUIBlueprintLibrary.h"


UPRActionDecisionPanel::UPRActionDecisionPanel(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// 크로스헤어 및 상호작용 UI 감춤
	FocusParams.ViewModelVisibilityOverrides.Add({TAG_UI_ViewModel_Crosshair,false});
	FocusParams.ViewModelVisibilityOverrides.Add({TAG_UI_ViewModel_Interaction,false});
}

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
	// 포커스 해제
	if (UPRCameraBlueprintLibrary::IsFocusing(GetOwningPlayer()))
	{
		UPRCameraBlueprintLibrary::RestoreFocus(GetOwningPlayer());	
	}

	RestoreOriginalEquipment();

	Super::NativeDestruct();
}

void UPRActionDecisionPanel::SetSourceActor(AActor* InActor)
{
	SourceActor = InActor;

	// 원래 장비 상태 저장
	SaveOriginalEquipmentState();

	// 액터 포커스 활성화
	UPRCameraBlueprintLibrary::FocusOnActor(GetOwningPlayer(), SourceActor, FocusParams);
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

	if (UWorld* World = GetWorld())
	{
		if (APRGameplayGameState* GS = World->GetGameState<APRGameplayGameState>())
		{
			GS->SendRoomEvent(TAG_Event_Reward_Selected);
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
