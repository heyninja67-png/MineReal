#include "MCRBlockChunk.h"
#include "MCRWorldManager.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"

AMCRBlockChunk::AMCRBlockChunk()
{
    PrimaryActorTick.bCanEverTick = false;
    VoxelMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("VoxelMesh"));
    SetRootComponent(VoxelMesh);
    VoxelMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    VoxelMesh->SetCollisionObjectType(ECC_WorldStatic);
    VoxelMesh->bUseAsyncCooking = true;

    // This engine debug material visualizes vertex colours. Replace it with a project PBR
    // material using VertexColor as Base Color before a polished art pass.
    if (UMaterialInterface* VertexMaterial = LoadObject<UMaterialInterface>(nullptr,
        TEXT("/Engine/EngineDebugMaterials/VertexColorViewMode_Mat.VertexColorViewMode_Mat")))
    {
        VoxelMesh->SetMaterial(0, VertexMaterial);
    }
    else
    {
        VoxelMesh->SetMaterial(0, UMaterial::GetDefaultMaterial(MD_Surface));
    }
}

void AMCRBlockChunk::Initialize(AMCRWorldManager* InManager, FIntPoint InChunkCoord)
{
    WorldManager = InManager;
    ChunkCoord = InChunkCoord;
    SetActorLocation(FVector(
        ChunkCoord.X * MCRVoxel::ChunkSize * MCRVoxel::BlockSize,
        ChunkCoord.Y * MCRVoxel::ChunkSize * MCRVoxel::BlockSize,
        0.0f));
    GenerateBlocks();
}

int32 AMCRBlockChunk::GetIndex(int32 X, int32 Y, int32 Z) const
{
    return X + MCRVoxel::ChunkSize * (Y + MCRVoxel::ChunkSize * Z);
}

EMCRBlock AMCRBlockChunk::GetBlockLocal(int32 X, int32 Y, int32 Z) const
{
    if (X < 0 || X >= MCRVoxel::ChunkSize || Y < 0 || Y >= MCRVoxel::ChunkSize ||
        Z < 0 || Z >= MCRVoxel::ChunkHeight)
    {
        return EMCRBlock::Air;
    }
    const int32 Index = GetIndex(X, Y, Z);
    return Blocks.IsValidIndex(Index) ? static_cast<EMCRBlock>(Blocks[Index]) : EMCRBlock::Air;
}

void AMCRBlockChunk::SetBlockLocal(int32 X, int32 Y, int32 Z, EMCRBlock Block)
{
    if (X < 0 || X >= MCRVoxel::ChunkSize || Y < 0 || Y >= MCRVoxel::ChunkSize ||
        Z < 0 || Z >= MCRVoxel::ChunkHeight)
    {
        return;
    }
    const int32 Index = GetIndex(X, Y, Z);
    if (Blocks.IsValidIndex(Index))
    {
        Blocks[Index] = static_cast<uint8>(Block);
    }
}

void AMCRBlockChunk::SetGeneratedBlock(int32 X, int32 Y, int32 Z, EMCRBlock Block)
{
    SetBlockLocal(X, Y, Z, Block);
}

void AMCRBlockChunk::GenerateBlocks()
{
    Blocks.Init(static_cast<uint8>(EMCRBlock::Air), MCRVoxel::ChunkSize * MCRVoxel::ChunkSize * MCRVoxel::ChunkHeight);
    const int32 Seed = WorldManager.IsValid() ? WorldManager->GetWorldSeed() : 1337;
    FRandomStream Random(Seed ^ (ChunkCoord.X * 73856093) ^ (ChunkCoord.Y * 19349663));

    for (int32 X = 0; X < MCRVoxel::ChunkSize; ++X)
    {
        for (int32 Y = 0; Y < MCRVoxel::ChunkSize; ++Y)
        {
            const int32 GX = ChunkCoord.X * MCRVoxel::ChunkSize + X;
            const int32 GY = ChunkCoord.Y * MCRVoxel::ChunkSize + Y;
            const float BroadNoise = FMath::PerlinNoise2D(FVector2D(GX * 0.012f, GY * 0.012f));
            const float DetailNoise = FMath::PerlinNoise2D(FVector2D(GX * 0.045f + 41.0f, GY * 0.045f - 17.0f));
            const float BiomeNoise = FMath::PerlinNoise2D(FVector2D(GX * 0.0025f + 200.0f, GY * 0.0025f - 90.0f));
            const float TemperatureNoise = FMath::PerlinNoise2D(FVector2D(GX * 0.004f - 71.0f, GY * 0.004f + 131.0f));
            const int32 SurfaceZ = FMath::Clamp(FMath::RoundToInt(27.0f + BroadNoise * 13.0f + DetailNoise * 4.0f), 8, 49);
            const bool bDesert = BiomeNoise > 0.28f;
            const bool bSnowy = TemperatureNoise < -0.48f && SurfaceZ > 30;
            const bool bForest = BiomeNoise < 0.20f && !bSnowy;

            for (int32 Z = 0; Z <= SurfaceZ; ++Z)
            {
                EMCRBlock Block = EMCRBlock::Stone;
                if (Z == SurfaceZ)
                {
                    Block = bSnowy ? EMCRBlock::Snow : (bDesert ? EMCRBlock::Sand : EMCRBlock::Grass);
                }
                else if (Z >= SurfaceZ - 3)
                {
                    Block = bDesert ? (Z < SurfaceZ - 1 ? EMCRBlock::Sandstone : EMCRBlock::Sand) : EMCRBlock::Dirt;
                }

                const float CaveNoise = FMath::PerlinNoise2D(FVector2D((GX + Z * 23) * 0.075f, (GY - Z * 11) * 0.075f));
                if (Z > 4 && Z < SurfaceZ - 3 && CaveNoise > 0.64f)
                {
                    continue;
                }

                if (Z < SurfaceZ - 4)
                {
                    const float OreNoise = FMath::PerlinNoise2D(FVector2D((GX + 117) * 0.13f + Z, (GY - 63) * 0.13f - Z));
                    if (Z < 10 && OreNoise > 0.82f) Block = EMCRBlock::DiamondOre;
                    else if (Z < 18 && OreNoise > 0.78f) Block = EMCRBlock::GoldOre;
                    else if (Z < 25 && OreNoise > 0.73f) Block = EMCRBlock::IronOre;
                    else if (Z < 32 && OreNoise > 0.76f) Block = EMCRBlock::CopperOre;
                    else if (OreNoise > 0.70f) Block = EMCRBlock::CoalOre;
                }
                SetGeneratedBlock(X, Y, Z, Block);
            }

            if (SurfaceZ < MCRVoxel::SeaLevel)
            {
                for (int32 Z = SurfaceZ + 1; Z <= MCRVoxel::SeaLevel; ++Z)
                {
                    SetGeneratedBlock(X, Y, Z, EMCRBlock::Water);
                }
            }

            // A simple forest/plains tree pass. Trees stay within their owning chunk to
            // keep generation deterministic and avoid cross-chunk write races.
            if (bForest && X >= 2 && X <= 13 && Y >= 2 && Y <= 13 &&
                SurfaceZ > MCRVoxel::SeaLevel + 1 && Random.FRand() < 0.045f)
            {
                const int32 TrunkHeight = Random.RandRange(4, 6);
                for (int32 H = 1; H <= TrunkHeight && SurfaceZ + H < MCRVoxel::ChunkHeight; ++H)
                {
                    SetGeneratedBlock(X, Y, SurfaceZ + H, EMCRBlock::Wood);
                }
                const int32 CrownZ = SurfaceZ + TrunkHeight;
                for (int32 DX = -2; DX <= 2; ++DX)
                {
                    for (int32 DY = -2; DY <= 2; ++DY)
                    {
                        for (int32 DZ = -1; DZ <= 2; ++DZ)
                        {
                            const int32 LX = X + DX;
                            const int32 LY = Y + DY;
                            const int32 LZ = CrownZ + DZ;
                            if (FMath::Abs(DX) + FMath::Abs(DY) + FMath::Abs(DZ) <= 5 &&
                                GetBlockLocal(LX, LY, LZ) == EMCRBlock::Air)
                            {
                                SetGeneratedBlock(LX, LY, LZ, EMCRBlock::Leaves);
                            }
                        }
                    }
                }
            }
        }
    }

    if (WorldManager.IsValid())
    {
        WorldManager->ApplySavedOverrides(this);
    }
}

void AMCRBlockChunk::AddFace(
    const FVector& A, const FVector& B, const FVector& C, const FVector& D,
    const FLinearColor& Color,
    TArray<FVector>& Vertices, TArray<int32>& Triangles,
    TArray<FVector>& Normals, TArray<FVector2D>& UVs,
    TArray<FLinearColor>& Colors, TArray<FProcMeshTangent>& Tangents) const
{
    const int32 Start = Vertices.Num();
    Vertices.Add(A);
    Vertices.Add(B);
    Vertices.Add(C);
    Vertices.Add(D);
    Triangles.Add(Start);
    Triangles.Add(Start + 1);
    Triangles.Add(Start + 2);
    Triangles.Add(Start);
    Triangles.Add(Start + 2);
    Triangles.Add(Start + 3);
    const FVector Normal = FVector::CrossProduct(B - A, C - A).GetSafeNormal();
    const FVector Tangent = (B - A).GetSafeNormal();
    for (int32 Index = 0; Index < 4; ++Index)
    {
        Normals.Add(Normal);
        Colors.Add(Color);
        Tangents.Add(FProcMeshTangent(Tangent, false));
    }
    UVs.Add(FVector2D(0, 0));
    UVs.Add(FVector2D(1, 0));
    UVs.Add(FVector2D(1, 1));
    UVs.Add(FVector2D(0, 1));
}

void AMCRBlockChunk::RebuildMesh()
{
    if (!VoxelMesh || !WorldManager.IsValid()) return;

    struct FSectionBuffers
    {
        TArray<FVector> Vertices;
        TArray<int32> Triangles;
        TArray<FVector> Normals;
        TArray<FVector2D> UVs;
        TArray<FLinearColor> Colors;
        TArray<FProcMeshTangent> Tangents;
    };
    TArray<FSectionBuffers> Sections;
    Sections.SetNum(MCRVoxel::MaxBlockSections);

    const float S=MCRVoxel::BlockSize;
    for (int32 Z=0; Z<MCRVoxel::ChunkHeight; ++Z)
    {
        for (int32 X=0; X<MCRVoxel::ChunkSize; ++X)
        {
            for (int32 Y=0; Y<MCRVoxel::ChunkSize; ++Y)
            {
                const EMCRBlock Block=GetBlockLocal(X,Y,Z);
                if (Block==EMCRBlock::Air || Block==EMCRBlock::Water) continue;
                const int32 SectionIndex=static_cast<int32>(Block);
                if (!Sections.IsValidIndex(SectionIndex)) continue;
                FSectionBuffers& Buffer=Sections[SectionIndex];
                const FIntVector WorldBlock(ChunkCoord.X*MCRVoxel::ChunkSize+X,ChunkCoord.Y*MCRVoxel::ChunkSize+Y,Z);
                const FVector P(X*S,Y*S,Z*S);
                const FVector P000=P, P100=P+FVector(S,0,0), P010=P+FVector(0,S,0), P110=P+FVector(S,S,0);
                const FVector P001=P+FVector(0,0,S), P101=P+FVector(S,0,S), P011=P+FVector(0,S,S), P111=P+FVector(S,S,S);
                const FLinearColor Color=MCRVoxel::GetBlockColor(Block);
                auto IsVisible=[&](const FIntVector& Neighbor)
                {
                    const EMCRBlock Other=WorldManager->GetBlockAtWorld(Neighbor);
                    return Other==EMCRBlock::Air || Other==EMCRBlock::Water || Other==EMCRBlock::Glass || Other!=Block;
                };
                if (IsVisible(WorldBlock+FIntVector(1,0,0))) AddFace(P100,P110,P111,P101,Color,Buffer.Vertices,Buffer.Triangles,Buffer.Normals,Buffer.UVs,Buffer.Colors,Buffer.Tangents);
                if (IsVisible(WorldBlock+FIntVector(-1,0,0))) AddFace(P000,P001,P011,P010,Color,Buffer.Vertices,Buffer.Triangles,Buffer.Normals,Buffer.UVs,Buffer.Colors,Buffer.Tangents);
                if (IsVisible(WorldBlock+FIntVector(0,1,0))) AddFace(P010,P011,P111,P110,Color,Buffer.Vertices,Buffer.Triangles,Buffer.Normals,Buffer.UVs,Buffer.Colors,Buffer.Tangents);
                if (IsVisible(WorldBlock+FIntVector(0,-1,0))) AddFace(P000,P100,P101,P001,Color,Buffer.Vertices,Buffer.Triangles,Buffer.Normals,Buffer.UVs,Buffer.Colors,Buffer.Tangents);
                if (IsVisible(WorldBlock+FIntVector(0,0,1))) AddFace(P001,P101,P111,P011,Color,Buffer.Vertices,Buffer.Triangles,Buffer.Normals,Buffer.UVs,Buffer.Colors,Buffer.Tangents);
                if (IsVisible(WorldBlock+FIntVector(0,0,-1))) AddFace(P000,P010,P110,P100,Color,Buffer.Vertices,Buffer.Triangles,Buffer.Normals,Buffer.UVs,Buffer.Colors,Buffer.Tangents);
            }
        }
    }

    VoxelMesh->ClearAllMeshSections();
    for (int32 SectionIndex=1; SectionIndex<Sections.Num(); ++SectionIndex)
    {
        FSectionBuffers& Buffer=Sections[SectionIndex];
        if (Buffer.Vertices.IsEmpty()) continue;
        VoxelMesh->CreateMeshSection_LinearColor(SectionIndex,Buffer.Vertices,Buffer.Triangles,Buffer.Normals,Buffer.UVs,Buffer.Colors,Buffer.Tangents,true);
        const EMCRBlock Block=static_cast<EMCRBlock>(SectionIndex);
        const TCHAR* MaterialPath=MCRVoxel::GetBlockMaterialPath(Block);
        if (MaterialPath && MaterialPath[0]!=0)
        {
            if (UMaterialInterface* Material=LoadObject<UMaterialInterface>(nullptr,MaterialPath)) VoxelMesh->SetMaterial(SectionIndex,Material);
        }
    }
}
