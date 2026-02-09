#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PRPrewarmInterface.generated.h"

class UNiagaraSystem;

UINTERFACE(MinimalAPI, BlueprintType)
class UPRPrewarmInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTREBOOT_API IPRPrewarmInterface
{
	GENERATED_BODY()

public:
	/*~ IPRPrewarmInterface ~*/
	// 프리웜 대상 에셋 목록 수집
	virtual void GetPrewarmNiagaraAssets(TArray<TSoftObjectPtr<UNiagaraSystem>>& OutAssets) const {}

	// 프리웜 대상 자식 오브젝트 수집
	virtual void GetPrewarmChildren(TArray<UObject*>& OutChildren) const {}
};
