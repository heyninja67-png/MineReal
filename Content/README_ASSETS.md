# Content assets status

This source package intentionally contains no third-party models, textures, sounds, or copied Minecraft assets. The voxel world is generated at runtime from C++ and the mesh supplies per-vertex block colours. A temporary Unreal engine vertex-colour debug material is attempted at runtime; if it is unavailable, the mesh falls back to the engine default surface material.

For a finished visual release, create a project-owned PBR material that reads Vertex Color, then replace the fallback assignment in `MCRBlockChunk.cpp`. Add only assets whose licences permit your intended use, and retain each asset's licence/attribution in this folder. No marketplace/Sketchfab/Poly Haven downloads are bundled or represented as included.
