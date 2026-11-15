#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MCRBlockTypes.h"
#include "MCRCraftingComponent.generated.h"

class UMCRInventoryComponent;
USTRUCT(BlueprintType)
struct FMCRCraftingRecipeView
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName RecipeId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EMCRBlock IngredientA = EMCRBlock::Air;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AmountA = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EMCRBlock IngredientB = EMCRBlock::Air;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 AmountB = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EMCRBlock Result = EMCRBlock::Air;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ResultCount = 1;
};

UCLASS(ClassGroup=(MCR), meta=(BlueprintSpawnableComponent))
class MINECRAFTBEDROCKREALISTIC_API UMCRCraftingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UMCRCraftingComponent();
    UFUNCTION(BlueprintCallable, Category="Crafting") bool Craft(FName RecipeId);
    UFUNCTION(BlueprintPure, Category="Crafting") TArray<FMCRCraftingRecipeView> GetAvailableRecipes() const;
private:
    UPROPERTY() TObjectPtr<UMCRInventoryComponent> Inventory;
    TArray<FMCRCraftingRecipeView> Recipes;
};
