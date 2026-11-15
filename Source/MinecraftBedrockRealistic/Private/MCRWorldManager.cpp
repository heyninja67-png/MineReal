#include "MCRWorldManager.h"
#include "MCRBlockChunk.h"
#include "MCRWaterSurface.h"
#include "MCRWorldSaveGame.h"
#include "MCRPlayerCharacter.h"
#include "MCRInventoryComponent.h"
#include "MCRSurvivalComponent.h"
#include "MCRDayNightWeather.h"
#include "MCRCreatureActor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr TCHAR SaveSlotName[] = TEXT("MCR_World_01");
}

AMCRWorldManager::AMCRWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.35f;
}

void AMCRWorldManager::BeginPlay()
{
    Super::BeginPlay();
    LoadWorld();
    Environment = GetWorld() ? GetWorld()->SpawnActor<AMCRDayNightWeather>() : nullptr;
    if (USoundBase* Ambient=LoadObject<USoundBase>(nullptr,TEXT("/Game/Audio/AMB_Wind_Forest.AMB_Wind_Forest")))
        AmbientAudio=UGameplayStatics::SpawnSound2D(this,Ambient,0.22f,1.0f,0.0f,nullptr,true,false);
    StreamChunks(FIntPoint::ZeroValue);
    if (Environment)
    {
        if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
        {
            if (UMCRWorldSaveGame* Saved = Cast<UMCRWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
            {
                Environment->SetTimeOfDay(Saved->TimeOfDayHours);
                Environment->SetWeather(static_cast<EMCRWeather>(FMath::Clamp<int32>(Saved->WeatherType,0,3)));
            }
        }
    }
    SpawnWildlife();
}

void AMCRWorldManager::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (AmbientAudio && !AmbientAudio->IsPlaying()) AmbientAudio->Play();
    AutoSaveAccumulator += DeltaSeconds;
    if (AutoSaveAccumulator >= 45.0f) { AutoSaveAccumulator = 0.0f; SaveWorld(); }
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!PlayerPawn)
    {
        return;
    }

    const FVector Position = PlayerPawn->GetActorLocation();
    const FIntVector BlockPosition(
        FMath::FloorToInt(Position.X / MCRVoxel::BlockSize),
        FMath::FloorToInt(Position.Y / MCRVoxel::BlockSize),
        FMath::FloorToInt(Position.Z / MCRVoxel::BlockSize));
    const FIntPoint Center = WorldBlockToChunk(BlockPosition);
    if (!bHasStreamed || Center != LastStreamCenter)
    {
        StreamChunks(Center);
    }
}

void AMCRWorldManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    SaveWorld();
    Super::EndPlay(EndPlayReason);
}

int32 AMCRWorldManager::FloorDiv(int32 Value, int32 Divisor)
{
    int32 Quotient = Value / Divisor;
    const int32 Remainder = Value % Divisor;
    if (Remainder < 0)
    {
        --Quotient;
    }
    return Quotient;
}

FIntPoint AMCRWorldManager::WorldBlockToChunk(const FIntVector& WorldBlock) const
{
    return FIntPoint(FloorDiv(WorldBlock.X, MCRVoxel::ChunkSize), FloorDiv(WorldBlock.Y, MCRVoxel::ChunkSize));
}

void AMCRWorldManager::LoadWorld()
{
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
    {
        return;
    }

    UMCRWorldSaveGame* Save = Cast<UMCRWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
    if (!Save)
    {
        return;
    }

    WorldSeed = Save->WorldSeed;
    EditedBlocks.Reset();
    for (const FMCRBlockSaveRecord& Record : Save->EditedBlocks)
    {
        EditedBlocks.Add(Record.WorldBlock, Record.BlockType);
    }
}

AMCRBlockChunk* AMCRWorldManager::EnsureChunk(const FIntPoint& Coord)
{
    if (TObjectPtr<AMCRBlockChunk>* Existing = LoadedChunks.Find(Coord))
    {
        return Existing->Get();
    }

    if (!GetWorld())
    {
        return nullptr;
    }

    const FVector Location(
        Coord.X * MCRVoxel::ChunkSize * MCRVoxel::BlockSize,
        Coord.Y * MCRVoxel::ChunkSize * MCRVoxel::BlockSize,
        0.0f);
    AMCRBlockChunk* NewChunk = GetWorld()->SpawnActor<AMCRBlockChunk>(
        AMCRBlockChunk::StaticClass(), FTransform(FRotator::ZeroRotator, Location));
    if (!NewChunk)
    {
        return nullptr;
    }

    // Register first so its generated mesh can query its own voxel data through this manager.
    LoadedChunks.Add(Coord, NewChunk);
    NewChunk->Initialize(this, Coord);
    return NewChunk;
}

AMCRWaterSurface* AMCRWorldManager::EnsureWaterSurface(const FIntPoint& Coord)
{
    if (TObjectPtr<AMCRWaterSurface>* Existing = LoadedWaterSurfaces.Find(Coord))
    {
        return Existing->Get();
    }

    if (!GetWorld() || !LoadedChunks.Contains(Coord))
    {
        return nullptr;
    }

    AMCRWaterSurface* NewSurface = GetWorld()->SpawnActor<AMCRWaterSurface>(AMCRWaterSurface::StaticClass());
    if (!NewSurface)
    {
        return nullptr;
    }

    LoadedWaterSurfaces.Add(Coord, NewSurface);
    NewSurface->Initialize(this, Coord);
    return NewSurface;
}

void AMCRWorldManager::StreamChunks(const FIntPoint& Center)
{
    LastStreamCenter = Center;
    bHasStreamed = true;

    TArray<FIntPoint> ToRemove;
    for (const TPair<FIntPoint, TObjectPtr<AMCRBlockChunk>>& Pair : LoadedChunks)
    {
        if (FMath::Abs(Pair.Key.X - Center.X) > LoadRadius + 1 ||
            FMath::Abs(Pair.Key.Y - Center.Y) > LoadRadius + 1)
        {
            ToRemove.Add(Pair.Key);
        }
    }
    for (const FIntPoint& Coord : ToRemove)
    {
        if (TObjectPtr<AMCRWaterSurface>* Water = LoadedWaterSurfaces.Find(Coord))
        {
            if (IsValid(Water->Get()))
            {
                Water->Get()->Destroy();
            }
        }
        LoadedWaterSurfaces.Remove(Coord);

        if (TObjectPtr<AMCRBlockChunk>* Chunk = LoadedChunks.Find(Coord))
        {
            if (IsValid(Chunk->Get()))
            {
                Chunk->Get()->Destroy();
            }
        }
        LoadedChunks.Remove(Coord);
    }

    for (int32 X = -LoadRadius; X <= LoadRadius; ++X)
    {
        for (int32 Y = -LoadRadius; Y <= LoadRadius; ++Y)
        {
            const FIntPoint Coord = Center + FIntPoint(X, Y);
            EnsureChunk(Coord);
            EnsureWaterSurface(Coord);
        }
    }

    // Rebuild terrain and water after every neighbour has been registered.
    for (const TPair<FIntPoint, TObjectPtr<AMCRBlockChunk>>& Pair : LoadedChunks)
    {
        if (IsValid(Pair.Value.Get()))
        {
            Pair.Value->RebuildMesh();
        }
        if (TObjectPtr<AMCRWaterSurface>* Water = LoadedWaterSurfaces.Find(Pair.Key))
        {
            if (IsValid(Water->Get()))
            {
                Water->Get()->RebuildSurface();
            }
        }
    }
}

EMCRBlock AMCRWorldManager::GetBlockAtWorld(const FIntVector& WorldBlock) const
{
    if (WorldBlock.Z < 0 || WorldBlock.Z >= MCRVoxel::ChunkHeight)
    {
        return EMCRBlock::Air;
    }

    const FIntPoint ChunkCoord = WorldBlockToChunk(WorldBlock);
    const TObjectPtr<AMCRBlockChunk>* Chunk = LoadedChunks.Find(ChunkCoord);
    if (!Chunk || !IsValid(Chunk->Get()))
    {
        return EMCRBlock::Air;
    }

    const int32 LocalX = WorldBlock.X - ChunkCoord.X * MCRVoxel::ChunkSize;
    const int32 LocalY = WorldBlock.Y - ChunkCoord.Y * MCRVoxel::ChunkSize;
    return Chunk->Get()->GetBlockLocal(LocalX, LocalY, WorldBlock.Z);
}

bool AMCRWorldManager::SetBlockAtWorld(const FIntVector& WorldBlock, EMCRBlock NewBlock)
{
    if (WorldBlock.Z < 0 || WorldBlock.Z >= MCRVoxel::ChunkHeight)
    {
        return false;
    }

    const FIntPoint ChunkCoord = WorldBlockToChunk(WorldBlock);
    AMCRBlockChunk* Chunk = EnsureChunk(ChunkCoord);
    if (!Chunk)
    {
        return false;
    }
    EnsureWaterSurface(ChunkCoord);

    const int32 LocalX = WorldBlock.X - ChunkCoord.X * MCRVoxel::ChunkSize;
    const int32 LocalY = WorldBlock.Y - ChunkCoord.Y * MCRVoxel::ChunkSize;
    Chunk->SetBlockLocal(LocalX, LocalY, WorldBlock.Z, NewBlock);
    EditedBlocks.Add(WorldBlock, static_cast<uint8>(NewBlock));
    RebuildChunkAndNeighbors(WorldBlock);
    SaveWorld();
    return true;
}

void AMCRWorldManager::RebuildChunkAndNeighbors(const FIntVector& ChangedBlock)
{
    const FIntPoint Center = WorldBlockToChunk(ChangedBlock);
    TSet<FIntPoint> ChunksToRebuild;
    ChunksToRebuild.Add(Center);

    const int32 LocalX = ChangedBlock.X - Center.X * MCRVoxel::ChunkSize;
    const int32 LocalY = ChangedBlock.Y - Center.Y * MCRVoxel::ChunkSize;
    if (LocalX == 0) ChunksToRebuild.Add(Center + FIntPoint(-1, 0));
    if (LocalX == MCRVoxel::ChunkSize - 1) ChunksToRebuild.Add(Center + FIntPoint(1, 0));
    if (LocalY == 0) ChunksToRebuild.Add(Center + FIntPoint(0, -1));
    if (LocalY == MCRVoxel::ChunkSize - 1) ChunksToRebuild.Add(Center + FIntPoint(0, 1));

    for (const FIntPoint& Coord : ChunksToRebuild)
    {
        if (TObjectPtr<AMCRBlockChunk>* Chunk = LoadedChunks.Find(Coord))
        {
            if (IsValid(Chunk->Get()))
            {
                Chunk->Get()->RebuildMesh();
            }
        }
        if (LoadedChunks.Contains(Coord))
        {
            if (AMCRWaterSurface* Water = EnsureWaterSurface(Coord))
            {
                Water->RebuildSurface();
            }
        }
    }
}

void AMCRWorldManager::ApplySavedOverrides(AMCRBlockChunk* Chunk)
{
    if (!Chunk)
    {
        return;
    }

    const FIntPoint Coord = Chunk->GetChunkCoord();
    for (const TPair<FIntVector, uint8>& Pair : EditedBlocks)
    {
        if (WorldBlockToChunk(Pair.Key) == Coord)
        {
            const int32 LocalX = Pair.Key.X - Coord.X * MCRVoxel::ChunkSize;
            const int32 LocalY = Pair.Key.Y - Coord.Y * MCRVoxel::ChunkSize;
            Chunk->SetBlockLocal(LocalX, LocalY, Pair.Key.Z, static_cast<EMCRBlock>(Pair.Value));
        }
    }
}

void AMCRWorldManager::SpawnWildlife()
{
    if (bWildlifeSpawned || !GetWorld()) return;
    bWildlifeSpawned = true;
    const FIntPoint Spots[] = { FIntPoint(4,4), FIntPoint(10,5), FIntPoint(6,11), FIntPoint(12,12), FIntPoint(2,9) };
    const EMCRCreatureType Types[] = { EMCRCreatureType::Sheep, EMCRCreatureType::Cow, EMCRCreatureType::Chicken, EMCRCreatureType::Boar, EMCRCreatureType::Sheep };
    for (int32 I=0; I<UE_ARRAY_COUNT(Spots); ++I)
    {
        const FIntVector Column(Spots[I].X,Spots[I].Y,MCRVoxel::ChunkHeight-1);
        int32 SurfaceZ=INDEX_NONE;
        for (int32 Z=MCRVoxel::ChunkHeight-1; Z>=0; --Z)
        {
            const EMCRBlock B=GetBlockAtWorld(FIntVector(Column.X,Column.Y,Z));
            if (B!=EMCRBlock::Air && B!=EMCRBlock::Water) { SurfaceZ=Z; break; }
        }
        if (SurfaceZ==INDEX_NONE) continue;
        const FVector Location((Column.X+.5f)*MCRVoxel::BlockSize,(Column.Y+.5f)*MCRVoxel::BlockSize,(SurfaceZ+1)*MCRVoxel::BlockSize);
        if (AMCRCreatureActor* Creature=GetWorld()->SpawnActor<AMCRCreatureActor>(AMCRCreatureActor::StaticClass(),Location,FRotator::ZeroRotator))
        {
            Creature->SetCreatureType(Types[I]); Wildlife.Add(Creature);
        }
    }
}

void AMCRWorldManager::SaveWorld()
{
    UMCRWorldSaveGame* Save = Cast<UMCRWorldSaveGame>(UGameplayStatics::CreateSaveGameObject(UMCRWorldSaveGame::StaticClass()));
    if (!Save)
    {
        return;
    }

    Save->WorldSeed = WorldSeed;
    Save->EditedBlocks.Reserve(EditedBlocks.Num());
    for (const TPair<FIntVector, uint8>& Pair : EditedBlocks)
    {
        FMCRBlockSaveRecord Record;
        Record.WorldBlock = Pair.Key;
        Record.BlockType = Pair.Value;
        Save->EditedBlocks.Add(Record);
    }
    if (AMCRPlayerCharacter* Player = Cast<AMCRPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0)))
    {
        if (UMCRInventoryComponent* Inventory = Player->GetInventoryComponent())
        {
            Save->PlayerInventory.Reset();
            for (const FMCRItemStack& Stack : Inventory->Slots)
            {
                FMCRItemSaveRecord Record; Record.BlockType = Stack.BlockType; Record.Count = Stack.Count;
                Save->PlayerInventory.Add(Record);
            }
        }
        if (UMCRSurvivalComponent* Survival = Player->GetSurvivalComponent())
        {
            Save->PlayerHealth = Survival->Health; Save->PlayerHunger = Survival->Hunger;
        }
    }
    if (Environment)
    {
        Save->TimeOfDayHours = Environment->TimeOfDayHours;
        Save->WeatherType = static_cast<uint8>(Environment->CurrentWeather);
    }
    UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
}
