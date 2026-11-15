#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MCRBlockTypes.h"
#include "MCRWorldSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FMCRBlockSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() FIntVector WorldBlock = FIntVector::ZeroValue;
    UPROPERTY() uint8 BlockType = 0;
};
USTRUCT(BlueprintType)
struct FMCRItemSaveRecord
{
    GENERATED_BODY()
    UPROPERTY() EMCRBlock BlockType = EMCRBlock::Air;
    UPROPERTY() int32 Count = 0;
};
UCLASS()
class MINECRAFTBEDROCKREALISTIC_API UMCRWorldSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 WorldSeed = 1337;
    UPROPERTY() TArray<FMCRBlockSaveRecord> EditedBlocks;
    UPROPERTY() TArray<FMCRItemSaveRecord> PlayerInventory;
    UPROPERTY() float PlayerHealth = 20.0f;
    UPROPERTY() float PlayerHunger = 20.0f;
    UPROPERTY() float TimeOfDayHours = 9.0f;
    UPROPERTY() uint8 WeatherType = 0;
};
