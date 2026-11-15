#include "MCRCraftingComponent.h"
#include "MCRInventoryComponent.h"

UMCRCraftingComponent::UMCRCraftingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    auto AddRecipe = [this](FName Id, EMCRBlock A, int32 CountA, EMCRBlock B, int32 CountB, EMCRBlock Out, int32 OutCount)
    {
        FMCRCraftingRecipeView R; R.RecipeId=Id; R.IngredientA=A; R.AmountA=CountA; R.IngredientB=B; R.AmountB=CountB; R.Result=Out; R.ResultCount=OutCount; Recipes.Add(R);
    };
    AddRecipe(TEXT("wood_to_planks"), EMCRBlock::Wood, 1, EMCRBlock::Air, 0, EMCRBlock::Planks, 4);
    AddRecipe(TEXT("planks_to_table"), EMCRBlock::Planks, 4, EMCRBlock::Air, 0, EMCRBlock::CraftingTable, 1);
    AddRecipe(TEXT("stone_to_cobble"), EMCRBlock::Stone, 1, EMCRBlock::Air, 0, EMCRBlock::Cobblestone, 1);
    AddRecipe(TEXT("sand_to_glass"), EMCRBlock::Sand, 4, EMCRBlock::Air, 0, EMCRBlock::Glass, 1);
    AddRecipe(TEXT("cobble_to_furnace"), EMCRBlock::Cobblestone, 8, EMCRBlock::Air, 0, EMCRBlock::Furnace, 1);
    AddRecipe(TEXT("planks_to_chest"), EMCRBlock::Planks, 8, EMCRBlock::Air, 0, EMCRBlock::Chest, 1);
    AddRecipe(TEXT("coal_to_torches"), EMCRBlock::CoalOre, 1, EMCRBlock::Wood, 1, EMCRBlock::Torch, 4);
}

bool UMCRCraftingComponent::Craft(FName RecipeId)
{
    if (!Inventory) Inventory = GetOwner() ? GetOwner()->FindComponentByClass<UMCRInventoryComponent>() : nullptr;
    if (!Inventory) return false;
    const FMCRCraftingRecipeView* Recipe = Recipes.FindByPredicate([RecipeId](const FMCRCraftingRecipeView& R){ return R.RecipeId == RecipeId; });
    if (!Recipe || Inventory->CountBlock(Recipe->IngredientA) < Recipe->AmountA) return false;
    if (Recipe->AmountB > 0 && Inventory->CountBlock(Recipe->IngredientB) < Recipe->AmountB) return false;
    if (!Inventory->RemoveBlock(Recipe->IngredientA, Recipe->AmountA)) return false;
    if (Recipe->AmountB > 0 && !Inventory->RemoveBlock(Recipe->IngredientB, Recipe->AmountB))
    {
        Inventory->AddBlock(Recipe->IngredientA, Recipe->AmountA);
        return false;
    }
    const int32 Overflow = Inventory->AddBlock(Recipe->Result, Recipe->ResultCount);
    if (Overflow > 0)
    {
        if (Recipe->AmountB > 0) Inventory->AddBlock(Recipe->IngredientB, Recipe->AmountB);
        Inventory->AddBlock(Recipe->IngredientA, Recipe->AmountA);
        return false;
    }
    return true;
}

TArray<FMCRCraftingRecipeView> UMCRCraftingComponent::GetAvailableRecipes() const
{
    return Recipes;
}
