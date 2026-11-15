#include "MCRWaterSurface.h"
#include "MCRWorldManager.h"
#include "MCRBlockTypes.h"
#include "ProceduralMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    constexpr float WaterUVWorldScale = 500.0f;

    FLinearColor GetWaterDepthColor(float WaterDepthInBlocks)
    {
        const float DepthFactor = FMath::Clamp((WaterDepthInBlocks - 1.0f) / 9.0f, 0.0f, 1.0f);
        const FLinearColor ShallowColor(0.055f, 0.43f, 0.48f, 0.60f);
        const FLinearColor DeepColor(0.012f, 0.105f, 0.28f, 0.86f);
        return FMath::Lerp(ShallowColor, DeepColor, DepthFactor);
    }
}

AMCRWaterSurface::AMCRWaterSurface()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = WaveUpdateInterval;

    SurfaceMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("AnimatedWaterSurface"));
    SetRootComponent(SurfaceMesh);
    SurfaceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SurfaceMesh->SetGenerateOverlapEvents(false);
    SurfaceMesh->SetCastShadow(false);
    SurfaceMesh->bUseAsyncCooking = false;

    // Created by Scripts/create_water_material.py inside the Unreal Editor.
    // If the artist has not generated the asset yet, a safe fallback is used.
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> WaterMaterial(
        TEXT("/Game/Water/Materials/M_Water_Realistic.M_Water_Realistic"));
    if (WaterMaterial.Succeeded())
    {
        SurfaceMesh->SetMaterial(0, WaterMaterial.Object);
    }
    else
    {
        SurfaceMesh->SetMaterial(0, UMaterial::GetDefaultMaterial(MD_Surface));
    }
}

void AMCRWaterSurface::Initialize(AMCRWorldManager* InWorldManager, FIntPoint InChunkCoord)
{
    if (SurfaceMesh)
    {
        if (UMaterialInterface* GeneratedWaterMaterial = LoadObject<UMaterialInterface>(
            nullptr, TEXT("/Game/Water/Materials/M_Water_Realistic.M_Water_Realistic")))
        {
            SurfaceMesh->SetMaterial(0, GeneratedWaterMaterial);
        }
    }

    WorldManager = InWorldManager;
    ChunkCoord = InChunkCoord;

    SetActorLocation(FVector(
        ChunkCoord.X * MCRVoxel::ChunkSize * MCRVoxel::BlockSize,
        ChunkCoord.Y * MCRVoxel::ChunkSize * MCRVoxel::BlockSize,
        0.0f));

    RebuildSurface();
}

void AMCRWaterSurface::AddWaterTile(int32 LocalX, int32 LocalY, int32 SurfaceBlockZ, float WaterDepthInBlocks)
{
    const float Block = MCRVoxel::BlockSize;
    const int32 Side = FMath::Max(1, SubdivisionsPerBlock);
    const int32 RowLength = Side + 1;
    const int32 StartVertex = BaseVertices.Num();
    const float SurfaceZ = (SurfaceBlockZ + 1) * Block;

    // Shallow water shifts toward turquoise; deeper columns become darker blue.
    const FLinearColor WaterColor = GetWaterDepthColor(WaterDepthInBlocks);

    for (int32 Y = 0; Y <= Side; ++Y)
    {
        for (int32 X = 0; X <= Side; ++X)
        {
            const float LocalWorldX = (LocalX + static_cast<float>(X) / Side) * Block;
            const float LocalWorldY = (LocalY + static_cast<float>(Y) / Side) * Block;
            const float GlobalWorldX = ChunkCoord.X * MCRVoxel::ChunkSize * Block + LocalWorldX;
            const float GlobalWorldY = ChunkCoord.Y * MCRVoxel::ChunkSize * Block + LocalWorldY;

            BaseVertices.Add(FVector(LocalWorldX, LocalWorldY, SurfaceZ));
            UVs.Add(FVector2D(GlobalWorldX / WaterUVWorldScale, GlobalWorldY / WaterUVWorldScale));
            VertexColors.Add(WaterColor);
            Tangents.Add(FProcMeshTangent(FVector(1.0f, 0.0f, 0.0f), false));
            Normals.Add(FVector::UpVector);
            WaveDisplacementMask.Add(1);
            WaveNormalMask.Add(1);
        }
    }

    for (int32 Y = 0; Y < Side; ++Y)
    {
        for (int32 X = 0; X < Side; ++X)
        {
            const int32 A = StartVertex + X + RowLength * Y;
            const int32 B = A + 1;
            const int32 D = A + RowLength;
            const int32 C = D + 1;
            // Counter-clockwise winding when viewed from above.
            Triangles.Add(A);
            Triangles.Add(B);
            Triangles.Add(C);
            Triangles.Add(A);
            Triangles.Add(C);
            Triangles.Add(D);
        }
    }
}

void AMCRWaterSurface::AddWaterSideQuad(
    const FVector& P0, const FVector& P1, const FVector& P2, const FVector& P3,
    const FVector& SideNormal, const FVector& SideTangent, const FLinearColor& Color)
{
    const int32 StartVertex = BaseVertices.Num();
    BaseVertices.Add(P0);
    BaseVertices.Add(P1);
    BaseVertices.Add(P2);
    BaseVertices.Add(P3);
    Normals.Add(SideNormal);
    Normals.Add(SideNormal);
    Normals.Add(SideNormal);
    Normals.Add(SideNormal);
    VertexColors.Add(Color);
    VertexColors.Add(Color);
    VertexColors.Add(Color);
    VertexColors.Add(Color);
    Tangents.Add(FProcMeshTangent(SideTangent, false));
    Tangents.Add(FProcMeshTangent(SideTangent, false));
    Tangents.Add(FProcMeshTangent(SideTangent, false));
    Tangents.Add(FProcMeshTangent(SideTangent, false));

    const FVector Origin = GetActorLocation();
    UVs.Add(FVector2D((Origin.X + P0.X) / WaterUVWorldScale, (Origin.Y + P0.Y) / WaterUVWorldScale));
    UVs.Add(FVector2D((Origin.X + P1.X) / WaterUVWorldScale, (Origin.Y + P1.Y) / WaterUVWorldScale));
    UVs.Add(FVector2D((Origin.X + P2.X) / WaterUVWorldScale, (Origin.Y + P2.Y) / WaterUVWorldScale));
    UVs.Add(FVector2D((Origin.X + P3.X) / WaterUVWorldScale, (Origin.Y + P3.Y) / WaterUVWorldScale));

    // Side wall bottom vertices stay fixed; only the top edge follows the swell.
    WaveDisplacementMask.Add(0);
    WaveDisplacementMask.Add(0);
    WaveDisplacementMask.Add(1);
    WaveDisplacementMask.Add(1);
    WaveNormalMask.Add(0);
    WaveNormalMask.Add(0);
    WaveNormalMask.Add(0);
    WaveNormalMask.Add(0);

    Triangles.Add(StartVertex);
    Triangles.Add(StartVertex + 1);
    Triangles.Add(StartVertex + 2);
    Triangles.Add(StartVertex);
    Triangles.Add(StartVertex + 2);
    Triangles.Add(StartVertex + 3);
}

void AMCRWaterSurface::RebuildSurface()
{
    if (!SurfaceMesh || !WorldManager.IsValid())
    {
        return;
    }

    BaseVertices.Reset();
    AnimatedVertices.Reset();
    Triangles.Reset();
    Normals.Reset();
    UVs.Reset();
    VertexColors.Reset();
    Tangents.Reset();
    WaveDisplacementMask.Reset();
    WaveNormalMask.Reset();

    const int32 BaseX = ChunkCoord.X * MCRVoxel::ChunkSize;
    const int32 BaseY = ChunkCoord.Y * MCRVoxel::ChunkSize;

    for (int32 X = 0; X < MCRVoxel::ChunkSize; ++X)
    {
        for (int32 Y = 0; Y < MCRVoxel::ChunkSize; ++Y)
        {
            int32 SurfaceBlockZ = INDEX_NONE;
            for (int32 ScanZ = MCRVoxel::ChunkHeight - 1; ScanZ >= 0; --ScanZ)
            {
                if (WorldManager->GetBlockAtWorld(FIntVector(BaseX + X, BaseY + Y, ScanZ)) == EMCRBlock::Water &&
                    WorldManager->GetBlockAtWorld(FIntVector(BaseX + X, BaseY + Y, ScanZ + 1)) == EMCRBlock::Air)
                {
                    SurfaceBlockZ = ScanZ;
                    break;
                }
            }
            if (SurfaceBlockZ == INDEX_NONE)
            {
                continue;
            }

            // Only the highest exposed water voxel in this column gets a surface mesh.
            float Depth = 0.0f;
            for (int32 Z = SurfaceBlockZ; Z >= 0; --Z)
            {
                if (WorldManager->GetBlockAtWorld(FIntVector(BaseX + X, BaseY + Y, Z)) != EMCRBlock::Water)
                {
                    break;
                }
                Depth += 1.0f;
            }
            const float SafeDepth = FMath::Max(1.0f, Depth);
            const FLinearColor WaterColor = GetWaterDepthColor(SafeDepth);
            AddWaterTile(X, Y, SurfaceBlockZ, SafeDepth);

            const float Block = MCRVoxel::BlockSize;
            const float TopZ = (SurfaceBlockZ + 1) * Block;
            const float BottomZ = TopZ - SafeDepth * Block;
            const float X0 = X * Block;
            const float X1 = (X + 1) * Block;
            const float Y0 = Y * Block;
            const float Y1 = (Y + 1) * Block;
            const FIntVector CellWorld(BaseX + X, BaseY + Y, SurfaceBlockZ);

            // Build only exposed shoreline walls. A neighboring land column covers its
            // own boundary; open neighboring columns need a visible water-volume edge.
            if (WorldManager->GetBlockAtWorld(CellWorld + FIntVector(1, 0, 0)) == EMCRBlock::Air)
            {
                AddWaterSideQuad(FVector(X1, Y0, BottomZ), FVector(X1, Y1, BottomZ),
                    FVector(X1, Y1, TopZ), FVector(X1, Y0, TopZ), FVector(1, 0, 0), FVector(0, 1, 0), WaterColor);
            }
            if (WorldManager->GetBlockAtWorld(CellWorld + FIntVector(-1, 0, 0)) == EMCRBlock::Air)
            {
                AddWaterSideQuad(FVector(X0, Y1, BottomZ), FVector(X0, Y0, BottomZ),
                    FVector(X0, Y0, TopZ), FVector(X0, Y1, TopZ), FVector(-1, 0, 0), FVector(0, -1, 0), WaterColor);
            }
            if (WorldManager->GetBlockAtWorld(CellWorld + FIntVector(0, 1, 0)) == EMCRBlock::Air)
            {
                AddWaterSideQuad(FVector(X1, Y1, BottomZ), FVector(X0, Y1, BottomZ),
                    FVector(X0, Y1, TopZ), FVector(X1, Y1, TopZ), FVector(0, 1, 0), FVector(-1, 0, 0), WaterColor);
            }
            if (WorldManager->GetBlockAtWorld(CellWorld + FIntVector(0, -1, 0)) == EMCRBlock::Air)
            {
                AddWaterSideQuad(FVector(X0, Y0, BottomZ), FVector(X1, Y0, BottomZ),
                    FVector(X1, Y0, TopZ), FVector(X0, Y0, TopZ), FVector(0, -1, 0), FVector(1, 0, 0), WaterColor);
            }
        }
    }

    if (BaseVertices.IsEmpty())
    {
        SurfaceMesh->ClearAllMeshSections();
        return;
    }

    SurfaceMesh->ClearAllMeshSections();
    AnimatedVertices = BaseVertices;
    UpdateAnimatedMesh();
    SurfaceMesh->CreateMeshSection_LinearColor(
        0, AnimatedVertices, Triangles, Normals, UVs, VertexColors, Tangents, false, false);
}

float AMCRWaterSurface::EvaluateWave(const FVector& WorldPosition, FVector& OutNormal) const
{
    // Three directional wave components: long swell, cross swell and fine ripples.
    // Heights are in centimetres; phase uses absolute world coordinates so adjacent
    // chunks share exactly the same wave pattern and do not produce visible seams.
    struct FWave
    {
        FVector2D Direction;
        float Amplitude;
        float Wavelength;
        float AngularSpeed;
    };

    const FWave Waves[] = {
        { FVector2D(0.82f, 0.57f).GetSafeNormal(), PrimaryWaveAmplitude, 950.0f, 0.72f },
        { FVector2D(-0.36f, 0.93f).GetSafeNormal(), SecondaryWaveAmplitude, 430.0f, 1.11f },
        { FVector2D(0.91f, -0.41f).GetSafeNormal(), 1.0f, 165.0f, 1.83f }
    };

    float Height = 0.0f;
    float DerivativeX = 0.0f;
    float DerivativeY = 0.0f;

    for (const FWave& Wave : Waves)
    {
        const float K = 2.0f * PI / Wave.Wavelength;
        const float Phase = K * (Wave.Direction.X * WorldPosition.X + Wave.Direction.Y * WorldPosition.Y)
            + Wave.AngularSpeed * WaveTime;
        Height += Wave.Amplitude * FMath::Sin(Phase);
        const float Slope = Wave.Amplitude * K * FMath::Cos(Phase);
        DerivativeX += Slope * Wave.Direction.X;
        DerivativeY += Slope * Wave.Direction.Y;
    }

    OutNormal = FVector(-DerivativeX, -DerivativeY, 1.0f).GetSafeNormal();
    return Height;
}

void AMCRWaterSurface::UpdateAnimatedMesh()
{
    if (!SurfaceMesh || BaseVertices.IsEmpty())
    {
        return;
    }

    AnimatedVertices.SetNumUninitialized(BaseVertices.Num());
    const FVector ActorOrigin = GetActorLocation();
    for (int32 Index = 0; Index < BaseVertices.Num(); ++Index)
    {
        if (WaveDisplacementMask.IsValidIndex(Index) && WaveDisplacementMask[Index] != 0)
        {
            const FVector WorldPosition = ActorOrigin + BaseVertices[Index];
            FVector WaveNormal = FVector::UpVector;
            const float Height = EvaluateWave(WorldPosition, WaveNormal);
            AnimatedVertices[Index] = BaseVertices[Index] + FVector(0.0f, 0.0f, Height);
            if (WaveNormalMask.IsValidIndex(Index) && WaveNormalMask[Index] != 0)
            {
                Normals[Index] = WaveNormal;
            }
        }
        else
        {
            AnimatedVertices[Index] = BaseVertices[Index];
        }
    }

    if (SurfaceMesh->GetProcMeshSection(0))
    {
        SurfaceMesh->UpdateMeshSection_LinearColor(
            0, AnimatedVertices, Normals, UVs, VertexColors, Tangents, false);
    }
}

void AMCRWaterSurface::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (BaseVertices.IsEmpty())
    {
        return;
    }

    WaveTime += DeltaSeconds;
    UpdateAccumulator += DeltaSeconds;
    if (UpdateAccumulator >= WaveUpdateInterval)
    {
        UpdateAccumulator = 0.0f;
        UpdateAnimatedMesh();
    }
}
