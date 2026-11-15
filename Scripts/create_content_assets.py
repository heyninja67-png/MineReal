"""Import original generated block textures, OBJ props and WAV effects into UE 5.8.

Run through UnrealEditor-Cmd.exe -run=pythonscript after source assets are unpacked.
The original PNG/OBJ/WAV source files remain in Content/*/Source for reproducibility.
"""
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.project_dir())
BLOCK_SOURCE = ROOT / "Content" / "Blocks" / "Source"
MODEL_SOURCE = ROOT / "Content" / "Models" / "Source"
AUDIO_SOURCE = ROOT / "Content" / "Audio" / "Source"
BLOCK_TEXTURES = "/Game/Blocks/Textures"
BLOCK_MATERIALS = "/Game/Blocks/Materials"
MODEL_DEST = "/Game/Models"
AUDIO_DEST = "/Game/Audio"

MATERIALS = [
    "Grass", "Dirt", "Stone", "Sand", "Wood", "Leaves", "CoalOre", "IronOre",
    "Cobblestone", "Planks", "Gravel", "Sandstone", "Snow", "Ice", "Clay",
    "CopperOre", "GoldOre", "DiamondOre",
]


def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_file(source, destination, name):
    asset_path = destination + "/" + name
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", str(source))
    task.set_editor_property("destination_path", destination)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    asset = unreal.load_asset(asset_path)
    if not asset:
        raise RuntimeError("Unreal import failed: " + str(source))
    return asset


def node(material, expression_type, x, y):
    result = unreal.MaterialEditingLibrary.create_material_expression(material, expression_type, x, y)
    if not result:
        raise RuntimeError("Could not create material expression " + str(expression_type))
    return result


def link(source, output_name, target, input_name):
    if not unreal.MaterialEditingLibrary.connect_material_expressions(source, output_name, target, input_name):
        raise RuntimeError("Material graph connection failed: " + input_name)


def material_output(source, output_name, property_name):
    if not unreal.MaterialEditingLibrary.connect_material_property(source, output_name, property_name):
        raise RuntimeError("Could not connect material property: " + str(property_name))


def make_block_material(name, albedo, normal, roughness):
    asset_path = BLOCK_MATERIALS + "/M_" + name
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.EditorAssetLibrary.delete_asset(asset_path)
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_" + name, BLOCK_MATERIALS, unreal.Material, unreal.MaterialFactoryNew()
    )
    if not mat:
        raise RuntimeError("Could not create block material " + name)
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", True)

    albedo_node = node(mat, unreal.MaterialExpressionTextureSampleParameter2D, -600, -250)
    albedo_node.set_editor_property("parameter_name", "BaseColorTex")
    albedo_node.set_editor_property("texture", albedo)
    material_output(albedo_node, "", unreal.MaterialProperty.MP_BASE_COLOR)

    normal_node = node(mat, unreal.MaterialExpressionTextureSampleParameter2D, -600, 80)
    normal_node.set_editor_property("parameter_name", "NormalTex")
    normal_node.set_editor_property("texture", normal)
    normal_node.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
    material_output(normal_node, "", unreal.MaterialProperty.MP_NORMAL)

    rough_node = node(mat, unreal.MaterialExpressionTextureSampleParameter2D, -600, 420)
    rough_node.set_editor_property("parameter_name", "RoughnessTex")
    rough_node.set_editor_property("texture", roughness)
    mask = node(mat, unreal.MaterialExpressionComponentMask, -300, 420)
    mask.set_editor_property("r", True)
    mask.set_editor_property("g", False)
    mask.set_editor_property("b", False)
    mask.set_editor_property("a", False)
    link(rough_node, "", mask, "Input")
    material_output(mask, "", unreal.MaterialProperty.MP_ROUGHNESS)

    spec = node(mat, unreal.MaterialExpressionScalarParameter, -300, 650)
    spec.set_editor_property("parameter_name", "Specular")
    spec.set_editor_property("default_value", 0.28 if name not in ("Ice", "Leaves") else 0.45)
    material_output(spec, "", unreal.MaterialProperty.MP_SPECULAR)
    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    return mat


ensure_directory(BLOCK_TEXTURES)
ensure_directory(BLOCK_MATERIALS)
ensure_directory(MODEL_DEST)
ensure_directory(AUDIO_DEST)

for name in MATERIALS:
    tex_name = name
    albedo = import_file(BLOCK_SOURCE / f"T_{tex_name}_BC.png", BLOCK_TEXTURES, f"T_{tex_name}_BC")
    normal = import_file(BLOCK_SOURCE / f"T_{tex_name}_N.png", BLOCK_TEXTURES, f"T_{tex_name}_N")
    rough = import_file(BLOCK_SOURCE / f"T_{tex_name}_R.png", BLOCK_TEXTURES, f"T_{tex_name}_R")
    albedo.set_editor_property("srgb", True)
    albedo.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DEFAULT)
    normal.set_editor_property("srgb", False)
    normal.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
    rough.set_editor_property("srgb", False)
    rough.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_MASKS)
    for texture in (albedo, normal, rough):
        texture.set_editor_property("address_x", unreal.TextureAddress.TA_WRAP)
        texture.set_editor_property("address_y", unreal.TextureAddress.TA_WRAP)
        unreal.EditorAssetLibrary.save_loaded_asset(texture)
    make_block_material(name, albedo, normal, rough)

# Import 10 original low-poly / voxel-style source meshes.
for source in sorted(MODEL_SOURCE.glob("*.obj")):
    import_file(source, MODEL_DEST, source.stem)

# Import synthesized one-shots and ambience beds.
for source in sorted(AUDIO_SOURCE.glob("*.wav")):
    import_file(source, AUDIO_DEST, source.stem)

print("Imported 18 PBR block sets, 10 source models and 8 original WAV assets.")
print("Review import warnings in the Editor log before packaging.")

# Simple parameterized tint for imported low-poly creatures (uses original vertex-free OBJ meshes).
CREATURE_MAT_DIR = "/Game/Creatures/Materials"
ensure_directory(CREATURE_MAT_DIR)
creature_path = CREATURE_MAT_DIR + "/M_CreatureTint"
if unreal.EditorAssetLibrary.does_asset_exist(creature_path):
    unreal.EditorAssetLibrary.delete_asset(creature_path)
creature_mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    "M_CreatureTint", CREATURE_MAT_DIR, unreal.Material, unreal.MaterialFactoryNew()
)
if not creature_mat:
    raise RuntimeError("Could not create creature tint material")
tint = node(creature_mat, unreal.MaterialExpressionVectorParameter, -300, -100)
tint.set_editor_property("parameter_name", "Tint")
tint.set_editor_property("default_value", unreal.LinearColor(.65, .61, .53, 1.0))
material_output(tint, "", unreal.MaterialProperty.MP_BASE_COLOR)
creature_rough = node(creature_mat, unreal.MaterialExpressionConstant, -300, 150)
creature_rough.set_editor_property("r", .86)
material_output(creature_rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
unreal.MaterialEditingLibrary.recompile_material(creature_mat)
unreal.EditorAssetLibrary.save_loaded_asset(creature_mat)
