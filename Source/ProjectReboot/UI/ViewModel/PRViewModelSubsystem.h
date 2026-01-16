// PRViewModelSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PRViewModelSubsystem.generated.h"

class UPRViewModelBase;

/**
 * ViewModel 중앙 레지스트리
 * - Global ViewModel: 플레이어 전역 상태 (HUD, 설정 등)
 * - Actor-Bound ViewModel: 특정 Actor에 종속된 상태 (적 체력바, NPC 등)
 */
UCLASS()
class PROJECTREBOOT_API UPRViewModelSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /*~ Global ViewModel (Actor 무관) ~*/

    // Global ViewModel 등록 (이미 존재하면 기존 반환)
    UFUNCTION(BlueprintCallable, Category = "ViewModel|Global", meta = (DeterminesOutputType = "ViewModelClass"))
    UPRViewModelBase* RegisterGlobalViewModel(TSubclassOf<UPRViewModelBase> ViewModelClass);

    // Global ViewModel 조회 (없으면 nullptr)
    UFUNCTION(BlueprintCallable, Category = "ViewModel|Global", meta = (DeterminesOutputType = "ViewModelClass"))
    UPRViewModelBase* GetGlobalViewModel(TSubclassOf<UPRViewModelBase> ViewModelClass) const;

    // Global ViewModel 조회 또는 생성
    UFUNCTION(BlueprintCallable, Category = "ViewModel|Global", meta = (DeterminesOutputType = "ViewModelClass"))
    UPRViewModelBase* GetOrCreateGlobalViewModel(TSubclassOf<UPRViewModelBase> ViewModelClass);

    // Global ViewModel 등록 해제
    UFUNCTION(BlueprintCallable, Category = "ViewModel|Global")
    bool UnregisterGlobalViewModel(TSubclassOf<UPRViewModelBase> ViewModelClass);

    // Global ViewModel 조회 또는 생성 (C++ 템플릿)
    template<typename T>
    T* GetOrCreateGlobalViewModel()
    {
        static_assert(TIsDerivedFrom<T, UPRViewModelBase>::Value, "T must derive from UPRViewModelBase");
        if (T* Existing = Cast<T>(GetGlobalViewModel(T::StaticClass())))
        {
            return Existing;
        }
        return Cast<T>(RegisterGlobalViewModel(T::StaticClass()));
    }

    // Global ViewModel 조회 (C++ 템플릿)
    template<typename T>
    T* GetGlobalViewModel() const
    {
        static_assert(TIsDerivedFrom<T, UPRViewModelBase>::Value, "T must derive from UPRViewModelBase");
        return Cast<T>(GetGlobalViewModel(T::StaticClass()));
    }

    /*~ Actor-Bound ViewModel ~*/

    // Actor에 바인딩된 ViewModel 조회 또는 생성
    UFUNCTION(BlueprintCallable, Category = "ViewModel|Actor", meta = (DeterminesOutputType = "ViewModelClass"))
    UPRViewModelBase* GetOrCreateActorViewModel(AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass);

    // Actor에 바인딩된 ViewModel 조회 (없으면 nullptr)
    UFUNCTION(BlueprintCallable, Category = "ViewModel|Actor", meta = (DeterminesOutputType = "ViewModelClass"))
    UPRViewModelBase* FindActorViewModel(AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass) const;

    // Actor에 바인딩된 특정 ViewModel 제거
    UFUNCTION(BlueprintCallable, Category = "ViewModel|Actor")
    bool RemoveActorViewModel(AActor* TargetActor, TSubclassOf<UPRViewModelBase> ViewModelClass);

    // Actor에 바인딩된 모든 ViewModel 제거
    UFUNCTION(BlueprintCallable, Category = "ViewModel|Actor")
    void RemoveAllViewModelsForActor(AActor* TargetActor);

    // Actor에 바인딩된 ViewModel 조회 또는 생성 (C++ 템플릿)
    template<typename T>
    T* GetOrCreateActorViewModel(AActor* TargetActor)
    {
        static_assert(TIsDerivedFrom<T, UPRViewModelBase>::Value, "T must derive from UPRViewModelBase");
        return Cast<T>(GetOrCreateActorViewModel(TargetActor, T::StaticClass()));
    }

    // Actor에 바인딩된 ViewModel 조회 (C++ 템플릿)
    template<typename T>
    T* FindActorViewModel(AActor* TargetActor) const
    {
        static_assert(TIsDerivedFrom<T, UPRViewModelBase>::Value, "T must derive from UPRViewModelBase");
        return Cast<T>(FindActorViewModel(TargetActor, T::StaticClass()));
    }

private:
    // Actor 파괴 시 자동 정리
    UFUNCTION()
    void HandleActorDestroyed(AActor* DestroyedActor);

private:
    // Internal Actor-Bound Key
    struct FActorViewModelKey
    {
        TWeakObjectPtr<AActor> Actor;
        UClass* ViewModelClass;

        FActorViewModelKey() = default;
        FActorViewModelKey(AActor* InActor, UClass* InClass)
            : Actor(InActor), ViewModelClass(InClass) {}

        bool operator==(const FActorViewModelKey& Other) const
        {
            return Actor == Other.Actor && ViewModelClass == Other.ViewModelClass;
        }

        friend uint32 GetTypeHash(const FActorViewModelKey& Key)
        {
            return HashCombine(GetTypeHash(Key.Actor), GetTypeHash(Key.ViewModelClass));
        }
    };

    // Global ViewModel 저장소
    UPROPERTY()
    TMap<TSubclassOf<UPRViewModelBase>, TObjectPtr<UPRViewModelBase>> GlobalViewModelMap;

    // Actor-Bound ViewModel 저장소
    TMap<FActorViewModelKey, TObjectPtr<UPRViewModelBase>> ActorViewModelMap;

    // 이미 OnDestroyed에 바인딩된 Actor 추적 (중복 바인딩 방지)
    TSet<TWeakObjectPtr<AActor>> BoundActors;
};