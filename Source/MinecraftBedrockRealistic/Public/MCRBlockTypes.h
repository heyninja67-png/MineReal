#pragma once

#include "CoreMinimal.h"
#include "MCRBlockTypes.generated.h"

UENUM(BlueprintType)
enum class EMCRBlock : uint8
{
    Air          UMETA(DisplayName="Ar"),
    Grass        UMETA(DisplayName="Grama"),
    Dirt         UMETA(DisplayName="Terra"),
    Stone        UMETA(DisplayName="Pedra"),
    Sand         UMETA(DisplayName="Areia"),
    Water        UMETA(DisplayName="Água"),
    Wood         UMETA(DisplayName="Madeira"),
    Leaves       UMETA(DisplayName="Folhas"),
    CoalOre      UMETA(DisplayName="Minério de carvão"),
    IronOre      UMETA(DisplayName="Minério de ferro"),
    Cobblestone  UMETA(DisplayName="Pedregulho"),
    Planks       UMETA(DisplayName="Tábuas"),
    Gravel       UMETA(DisplayName="Cascalho"),
    Sandstone    UMETA(DisplayName="Arenito"),
    Snow         UMETA(DisplayName="Neve"),
    Ice          UMETA(DisplayName="Gelo"),
    Clay         UMETA(DisplayName="Argila"),
    CopperOre    UMETA(DisplayName="Minério de cobre"),
    GoldOre      UMETA(DisplayName="Minério de ouro"),
    DiamondOre   UMETA(DisplayName="Minério de cristal"),
    Glass        UMETA(DisplayName="Vidro"),
    Torch        UMETA(DisplayName="Tocha"),
    CraftingTable UMETA(DisplayName="Bancada"),
    Chest        UMETA(DisplayName="Baú"),
    Furnace      UMETA(DisplayName="Fornalha"),
    Bedrock      UMETA(DisplayName="Rocha matriz")
};

namespace MCRVoxel
{
    constexpr int32 ChunkSize = 16;
    constexpr int32 ChunkHeight = 64;
    constexpr float BlockSize = 100.0f;
    constexpr int32 SeaLevel = 22;
    constexpr int32 MaxBlockSections = 32;

    inline FLinearColor GetBlockColor(EMCRBlock Block)
    {
        switch (Block)
        {
        case EMCRBlock::Grass:         return FLinearColor(0.20f, 0.43f, 0.13f, 1.0f);
        case EMCRBlock::Dirt:          return FLinearColor(0.36f, 0.23f, 0.13f, 1.0f);
        case EMCRBlock::Stone:         return FLinearColor(0.42f, 0.44f, 0.46f, 1.0f);
        case EMCRBlock::Sand:          return FLinearColor(0.78f, 0.69f, 0.48f, 1.0f);
        case EMCRBlock::Water:         return FLinearColor(0.08f, 0.30f, 0.58f, 0.72f);
        case EMCRBlock::Wood:          return FLinearColor(0.40f, 0.25f, 0.13f, 1.0f);
        case EMCRBlock::Leaves:        return FLinearColor(0.12f, 0.34f, 0.12f, 1.0f);
        case EMCRBlock::CoalOre:       return FLinearColor(0.16f, 0.17f, 0.18f, 1.0f);
        case EMCRBlock::IronOre:       return FLinearColor(0.61f, 0.49f, 0.39f, 1.0f);
        case EMCRBlock::Cobblestone:   return FLinearColor(0.37f, 0.39f, 0.41f, 1.0f);
        case EMCRBlock::Planks:        return FLinearColor(0.58f, 0.38f, 0.20f, 1.0f);
        case EMCRBlock::Gravel:        return FLinearColor(0.48f, 0.47f, 0.43f, 1.0f);
        case EMCRBlock::Sandstone:     return FLinearColor(0.68f, 0.57f, 0.37f, 1.0f);
        case EMCRBlock::Snow:          return FLinearColor(0.88f, 0.93f, 0.96f, 1.0f);
        case EMCRBlock::Ice:           return FLinearColor(0.48f, 0.75f, 0.86f, 0.82f);
        case EMCRBlock::Clay:          return FLinearColor(0.55f, 0.49f, 0.43f, 1.0f);
        case EMCRBlock::CopperOre:     return FLinearColor(0.32f, 0.55f, 0.43f, 1.0f);
        case EMCRBlock::GoldOre:       return FLinearColor(0.78f, 0.62f, 0.23f, 1.0f);
        case EMCRBlock::DiamondOre:    return FLinearColor(0.30f, 0.77f, 0.82f, 1.0f);
        case EMCRBlock::Glass:         return FLinearColor(0.62f, 0.83f, 0.90f, 0.28f);
        case EMCRBlock::Torch:         return FLinearColor(1.0f, 0.68f, 0.22f, 1.0f);
        case EMCRBlock::CraftingTable: return FLinearColor(0.48f, 0.29f, 0.15f, 1.0f);
        case EMCRBlock::Chest:         return FLinearColor(0.52f, 0.30f, 0.12f, 1.0f);
        case EMCRBlock::Furnace:       return FLinearColor(0.31f, 0.32f, 0.34f, 1.0f);
        case EMCRBlock::Bedrock:       return FLinearColor(0.18f, 0.19f, 0.20f, 1.0f);
        default:                       return FLinearColor::Transparent;
        }
    }

    inline const TCHAR* GetBlockMaterialPath(EMCRBlock Block)
    {
        switch (Block)
        {
        case EMCRBlock::Grass: return TEXT("/Game/Blocks/Materials/M_Grass.M_Grass");
        case EMCRBlock::Dirt: return TEXT("/Game/Blocks/Materials/M_Dirt.M_Dirt");
        case EMCRBlock::Stone: return TEXT("/Game/Blocks/Materials/M_Stone.M_Stone");
        case EMCRBlock::Sand: return TEXT("/Game/Blocks/Materials/M_Sand.M_Sand");
        case EMCRBlock::Wood: return TEXT("/Game/Blocks/Materials/M_Wood.M_Wood");
        case EMCRBlock::Leaves: return TEXT("/Game/Blocks/Materials/M_Leaves.M_Leaves");
        case EMCRBlock::CoalOre: return TEXT("/Game/Blocks/Materials/M_CoalOre.M_CoalOre");
        case EMCRBlock::IronOre: return TEXT("/Game/Blocks/Materials/M_IronOre.M_IronOre");
        case EMCRBlock::Cobblestone: return TEXT("/Game/Blocks/Materials/M_Cobblestone.M_Cobblestone");
        case EMCRBlock::Planks: return TEXT("/Game/Blocks/Materials/M_Planks.M_Planks");
        case EMCRBlock::Gravel: return TEXT("/Game/Blocks/Materials/M_Gravel.M_Gravel");
        case EMCRBlock::Sandstone: return TEXT("/Game/Blocks/Materials/M_Sandstone.M_Sandstone");
        case EMCRBlock::Snow: return TEXT("/Game/Blocks/Materials/M_Snow.M_Snow");
        case EMCRBlock::Ice: return TEXT("/Game/Blocks/Materials/M_Ice.M_Ice");
        case EMCRBlock::Clay: return TEXT("/Game/Blocks/Materials/M_Clay.M_Clay");
        case EMCRBlock::CopperOre: return TEXT("/Game/Blocks/Materials/M_CopperOre.M_CopperOre");
        case EMCRBlock::GoldOre: return TEXT("/Game/Blocks/Materials/M_GoldOre.M_GoldOre");
        case EMCRBlock::DiamondOre: return TEXT("/Game/Blocks/Materials/M_DiamondOre.M_DiamondOre");
        default: return TEXT("");
        }
    }
}
