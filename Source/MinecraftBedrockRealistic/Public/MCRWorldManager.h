#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCRBlockTypes.h"
#include "MCRWorldManager.generated.h"

class AMCRBlockChunk;
class AMCRWaterSurface;
class UMCRWorldSaveGame;
class AMCRDayNightWeather;
class AMCRCreatureActor;
class UAudioComponent;

UCLASS()
class MINECRAFTBEDROCKREALISTIC_API AMCRWorldManager : public AActor
{
    GENERATED_BODY()
public:
    AMCRWorldManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    EMCRBlock GetBlockAtWorld(const FIntVector& WorldBlock) const;
    bool SetBlockAtWorld(const FIntVector& WorldBlock, EMCRBlock NewBlock);
    int32 GetWorldSeed() const { return WorldSeed; }
    void ApplySavedOverrides(AMCRBlockChunk* Chunk);
    void SaveWorld();
    AMCRDayNightWeather* GetEnvironment() const { return Environment; }

private:
    UPROPERTY(EditAnywhere, Category="World", meta=(ClampMin="1", ClampMax="6"))
    int32 LoadRadius = 1;

    UPROPERTY(EditAnywhere, Category="World")
    int32 WorldSeed = 1337;

    UPROPERTY() TMap<FIntPoint, TObjectPtr<AMCRBlockChunk>> LoadedChunks;
    UPROPERTY() TMap<FIntPoint, TObjectPtr<AMCRWaterSurface>> LoadedWaterSurfaces;
    TMap<FIntVector, uint8> EditedBlocks;
    FIntPoint LastStreamCenter = FIntPoint(999999, 999999);
    bool bHasStreamed = false;
    bool bWildlifeSpawned = false;
    float AutoSaveAccumulator = 0.0f;
    UPROPERTY() TObjectPtr<AMCRDayNightWeather> Environment;
    UPROPERTY() TObjectPtr<UAudioComponent> AmbientAudio;
    UPROPERTY() TArray<TObjectPtr<AMCRCreatureActor>> Wildlife;

    static int32 FloorDiv(int32 Value, int32 Divisor);
    FIntPoint WorldBlockToChunk(const FIntVector& WorldBlock) const;
    void LoadWorld();
    void StreamChunks(const FIntPoint& Center);
    AMCRBlockChunk* EnsureChunk(const FIntPoint& Coord);
    AMCRWaterSurface* EnsureWaterSurface(const FIntPoint& Coord);
    void RebuildChunkAndNeighbors(const FIntVector& ChangedBlock);
    void SpawnWildlife();
};
