#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "MCRWaterSurface.generated.h"

class AMCRWorldManager;

/**
 * Independent animated water surface for a voxel chunk.
 * Water is deliberately not merged into the opaque terrain mesh: doing so would
 * prevent translucency, refraction and physically-lit highlights from working.
 */
UCLASS()
class MINECRAFTBEDROCKREALISTIC_API AMCRWaterSurface : public AActor
{
    GENERATED_BODY()

public:
    AMCRWaterSurface();

    void Initialize(AMCRWorldManager* InWorldManager, FIntPoint InChunkCoord);
    void RebuildSurface();

    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY(VisibleAnywhere, Category="Water")
    TObjectPtr<UProceduralMeshComponent> SurfaceMesh;

    UPROPERTY(EditDefaultsOnly, Category="Water|Waves", meta=(ClampMin="0.0", ClampMax="30.0"))
    float PrimaryWaveAmplitude = 7.0f;

    UPROPERTY(EditDefaultsOnly, Category="Water|Waves", meta=(ClampMin="0.0", ClampMax="15.0"))
    float SecondaryWaveAmplitude = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category="Water|Waves", meta=(ClampMin="1", ClampMax="8"))
    int32 SubdivisionsPerBlock = 2;

    UPROPERTY(EditDefaultsOnly, Category="Water|Waves", meta=(ClampMin="0.02", ClampMax="0.25"))
    float WaveUpdateInterval = 0.0667f;

    TWeakObjectPtr<AMCRWorldManager> WorldManager;
    FIntPoint ChunkCoord = FIntPoint::ZeroValue;
    float WaveTime = 0.0f;
    float UpdateAccumulator = 0.0f;

    TArray<FVector> BaseVertices;
    TArray<FVector> AnimatedVertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FLinearColor> VertexColors;
    TArray<FProcMeshTangent> Tangents;
    TArray<uint8> WaveDisplacementMask;
    TArray<uint8> WaveNormalMask;

    void AddWaterTile(int32 LocalX, int32 LocalY, int32 SurfaceBlockZ, float WaterDepthInBlocks);
    void AddWaterSideQuad(const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3,
        const FVector& SideNormal, const FVector& SideTangent, const FLinearColor& Color);
    float EvaluateWave(const FVector& WorldPosition, FVector& OutNormal) const;
    void UpdateAnimatedMesh();
};
