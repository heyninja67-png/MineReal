#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MCRBlockTypes.h"
#include "MCRBlockChunk.generated.h"

class UProceduralMeshComponent;
class AMCRWorldManager;

UCLASS()
class MINECRAFTBEDROCKREALISTIC_API AMCRBlockChunk : public AActor
{
    GENERATED_BODY()
public:
    AMCRBlockChunk();

    void Initialize(AMCRWorldManager* InManager, FIntPoint InChunkCoord);
    void GenerateBlocks();
    void RebuildMesh();
    EMCRBlock GetBlockLocal(int32 X, int32 Y, int32 Z) const;
    void SetBlockLocal(int32 X, int32 Y, int32 Z, EMCRBlock Block);
    FIntPoint GetChunkCoord() const { return ChunkCoord; }

private:
    UPROPERTY(VisibleAnywhere) TObjectPtr<UProceduralMeshComponent> VoxelMesh;
    TWeakObjectPtr<AMCRWorldManager> WorldManager;
    FIntPoint ChunkCoord = FIntPoint::ZeroValue;
    TArray<uint8> Blocks;

    int32 GetIndex(int32 X, int32 Y, int32 Z) const;
    void SetGeneratedBlock(int32 X, int32 Y, int32 Z, EMCRBlock Block);
    void AddFace(
        const FVector& A, const FVector& B, const FVector& C, const FVector& D,
        const FLinearColor& Color,
        TArray<FVector>& Vertices, TArray<int32>& Triangles,
        TArray<FVector>& Normals, TArray<FVector2D>& UVs,
        TArray<FLinearColor>& Colors, TArray<FProcMeshTangent>& Tangents) const;
};
