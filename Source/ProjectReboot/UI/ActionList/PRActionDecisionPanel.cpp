// Fill out your copyright notice in the Description page of Project Settings.

#include "PRActionDecisionPanel.h"

#include "PRActionListItemWidget.h"
#include "Components/VerticalBox.h"
#include "ProjectReboot/Equipment/PREquipmentBlueprintLibrary.h"
#include "ProjectReboot/Equipment/PREquipActionData.h"
#include "ProjectReboot/UI/Preview/PRActorPreviewPanel.h"

void UPRActionDecisionPanel::SetSourceActor(AActor* InActor, TSubclassOf<AActor> PreviewActorClass)
{
	SourceActor = InActor;
	
	if (ActorPreviewPanel)
	{
		ActorPreviewPanel->SetPreviewActorByClass(PreviewActorClass);
	}
	
	RefreshWidget();
}

void UPRActionDecisionPanel::RefreshWidget()
{
	if (!ActorPreviewPanel)
	{
		return;
	}

	if (!IsValid(SourceActor))
	{
		return;
	}

	AActor* PreviewActor = ActorPreviewPanel->GetPreviewActor();
	if (!PreviewActor || PreviewActor == SourceActor)
	{
		return;
	}

	UPREquipmentBlueprintLibrary::SyncEquipmentManager(SourceActor, PreviewActor);
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
	if (!EquipAction || !ActorPreviewPanel)
	{
		return;
	}

	AActor* PreviewActor = ActorPreviewPanel->GetPreviewActor();
	if (!PreviewActor)
	{
		return;
	}

	if (bEquip)
	{
		// 새 장비 장착
		UPREquipmentBlueprintLibrary::OverrideEquipAction(PreviewActor, EquipAction);
	}
	else
	{
		// 장비 해제 후 SourceActor의 원래 장비 상태로 동기화
		RefreshWidget();
	}
}
