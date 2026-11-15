#include "MCRSurvivalComponent.h"
#include "GameFramework/Actor.h"

UMCRSurvivalComponent::UMCRSurvivalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
}
void UMCRSurvivalComponent::BeginPlay()
{
    Super::BeginPlay();
    Health = MaxHealth; Hunger = MaxHunger;
    OnHealthChanged.Broadcast(Health, MaxHealth); OnHungerChanged.Broadcast(Hunger, MaxHunger);
}
void UMCRSurvivalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bCreativeMode) return;
    HungerAccumulator += DeltaTime;
    if (HungerAccumulator >= 18.0f)
    {
        HungerAccumulator = 0.0f;
        Hunger = FMath::Max(0.0f, Hunger - 0.12f);
        OnHungerChanged.Broadcast(Hunger, MaxHunger);
    }
    RecoveryAccumulator += DeltaTime;
    if (Hunger >= 18.0f && Health < MaxHealth && RecoveryAccumulator >= 4.0f)
    {
        RecoveryAccumulator = 0.0f; RestoreHealth(0.5f); Hunger = FMath::Max(0.0f, Hunger - 0.25f); OnHungerChanged.Broadcast(Hunger, MaxHunger);
    }
    if (Hunger <= 0.0f && Health > 1.0f && RecoveryAccumulator >= 8.0f)
    {
        RecoveryAccumulator = 0.0f; ApplyDamage(1.0f);
    }
}
void UMCRSurvivalComponent::ApplyDamage(float Amount)
{
    if (bCreativeMode || Amount <= 0.0f) return;
    Health = FMath::Clamp(Health - Amount, 0.0f, MaxHealth); OnHealthChanged.Broadcast(Health, MaxHealth);
}
void UMCRSurvivalComponent::RestoreHealth(float Amount)
{
    if (Amount <= 0.0f) return;
    Health = FMath::Clamp(Health + Amount, 0.0f, MaxHealth); OnHealthChanged.Broadcast(Health, MaxHealth);
}
void UMCRSurvivalComponent::ConsumeFood(float HungerRestored)
{
    Hunger = FMath::Clamp(Hunger + HungerRestored, 0.0f, MaxHunger); OnHungerChanged.Broadcast(Hunger, MaxHunger);
}
void UMCRSurvivalComponent::SetCreativeMode(bool bEnabled)
{
    bCreativeMode = bEnabled; if (bEnabled) { Health=MaxHealth; Hunger=MaxHunger; OnHealthChanged.Broadcast(Health,MaxHealth); OnHungerChanged.Broadcast(Hunger,MaxHunger); }
}
void UMCRSurvivalComponent::ApplyFallDamage(float FallHeight)
{
    if (!bCreativeMode && FallHeight > 650.0f) ApplyDamage(FMath::FloorToFloat((FallHeight - 550.0f) / 180.0f));
}
