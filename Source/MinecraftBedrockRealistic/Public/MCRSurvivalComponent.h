#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCRSurvivalComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMCRVitalChanged, float, CurrentValue, float, MaximumValue);
UCLASS(ClassGroup=(MCR), meta=(BlueprintSpawnableComponent))
class MINECRAFTBEDROCKREALISTIC_API UMCRSurvivalComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UMCRSurvivalComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival") float MaxHealth = 20.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival") float MaxHunger = 20.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") float Health = 20.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Survival") float Hunger = 20.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Survival") bool bCreativeMode = false;
    UPROPERTY(BlueprintAssignable, Category="Survival") FMCRVitalChanged OnHealthChanged;
    UPROPERTY(BlueprintAssignable, Category="Survival") FMCRVitalChanged OnHungerChanged;
    UFUNCTION(BlueprintCallable, Category="Survival") void ApplyDamage(float Amount);
    UFUNCTION(BlueprintCallable, Category="Survival") void RestoreHealth(float Amount);
    UFUNCTION(BlueprintCallable, Category="Survival") void ConsumeFood(float HungerRestored);
    UFUNCTION(BlueprintCallable, Category="Survival") void SetCreativeMode(bool bEnabled);
    void ApplyFallDamage(float FallHeight);
private:
    float HungerAccumulator = 0.0f;
    float RecoveryAccumulator = 0.0f;
};
