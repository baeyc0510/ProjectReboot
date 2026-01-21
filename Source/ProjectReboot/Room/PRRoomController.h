// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PRRoomTypes.h"
#include "GameFramework/Actor.h"
#include "PRRoomController.generated.h"

class UPRRoomStateTreeComponent;

UCLASS()
class PROJECTREBOOT_API APRRoomController : public AActor
{
	GENERATED_BODY()

public:
	APRRoomController();
	
	UFUNCTION(BlueprintPure, Category = "Room")
	UPRRoomStateTreeComponent* GetStateTreeComponent() const {return StateTreeComponent;}
	
	void InitRoom(const FPRRoomConfig& InRoomConfig);
	
	void ActivateRoom();
	
	int32 GetRoomIndex() const;
	
protected:
	virtual void BeginPlay() override;
	
public:
	// 방 안에 배치된 문(Door) 액터들, TODO: Interface 활용한 Open / Close 처리
	UPROPERTY(EditAnywhere, Category = "Room")
	TArray<AActor*> EntryDoors;

	UPROPERTY(EditAnywhere, Category = "Room")
	TArray<AActor*> ExitDoors;
	
	UPROPERTY(VisibleAnywhere, Category = "Room")
	FPRRoomConfig RoomConfig;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UPRRoomStateTreeComponent> StateTreeComponent;
};