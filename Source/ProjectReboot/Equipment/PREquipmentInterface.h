#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PREquipmentInterface.generated.h"

class UPREquipmentManagerComponent;

UINTERFACE()
class UPREquipmentInterface : public UInterface
{
	GENERATED_BODY()
};


class PROJECTREBOOT_API IPREquipmentInterface
{
	GENERATED_BODY()
	
public:
	virtual UPREquipmentManagerComponent* GetEquipmentManager() const = 0;
};
