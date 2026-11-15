"""Create/import the water material and original normal maps inside Unreal Editor 5.8.

Run with:
  UnrealEditor-Cmd.exe Project.uproject -run=pythonscript -script=Scripts/create_water_material.py -unattended -nop4
or from the Editor: File > Execute Python Script.
"""
from pathlib import Path
import unreal

PROJECT_DIR = Path(unreal.Paths.project_dir())
SOURCE_DIR = PROJECT_DIR / "Content" / "Water" / "Source"
TEXTURE_DIR = "/Game/Water/Textures"
MATERIAL_DIR = "/Game/Water/Materials"
MATERIAL_ASSET = MATERIAL_DIR + "/M_Water_Realistic"


def ensure_directory(path: str) -> None:
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def import_normal_map(filename: str, asset_name: str):
    source_file = SOURCE_DIR / filename
    if not source_file.is_file():
        raise RuntimeError("Missing source water normal map: " + str(source_file))

    target_path = TEXTURE_DIR + "/" + asset_name
    if unreal.EditorAssetLibrary.does_asset_exist(target_path):
        texture = unreal.load_asset(target_path)
    else:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", str(source_file))
        task.set_editor_property("destination_path", TEXTURE_DIR)
        task.set_editor_property("destination_name", asset_name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        texture = unreal.load_asset(target_path)

    if not texture:
        raise RuntimeError("Unreal failed to import " + str(source_file))

    texture.set_editor_property("srgb", False)
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_NORMALMAP)
    texture.set_editor_property("filter", unreal.TextureFilter.TF_TRILINEAR)
    texture.set_editor_property("address_x", unreal.TextureAddress.TA_WRAP)
    texture.set_editor_property("address_y", unreal.TextureAddress.TA_WRAP)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture


def node(material, expression_type, x, y):
    result = unreal.MaterialEditingLibrary.create_material_expression(material, expression_type, x, y)
    if not result:
        raise RuntimeError("Could not create material expression: " + str(expression_type))
    return result


def link(source, output_name, target, input_name):
    ok = unreal.MaterialEditingLibrary.connect_material_expressions(source, output_name, target, input_name)
    if not ok:
        raise RuntimeError("Material connection failed: " + str(source) + " -> " + str(target) + " (" + input_name + ")")


def output(source, output_name, material_property):
    ok = unreal.MaterialEditingLibrary.connect_material_property(source, output_name, material_property)
    if not ok:
        raise RuntimeError("Could not connect material property: " + str(material_property))


def scalar(material, name, value, x, y):
    expr = node(material, unreal.MaterialExpressionScalarParameter, x, y)
    expr.set_editor_property("parameter_name", name)
    expr.set_editor_property("default_value", value)
    return expr


def create_material(normal_a, normal_b):
    if unreal.EditorAssetLibrary.does_asset_exist(MATERIAL_ASSET):
        unreal.EditorAssetLibrary.delete_asset(MATERIAL_ASSET)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(
        "M_Water_Realistic", MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew()
    )
    if not material:
        raise RuntimeError("Could not create /Game/Water/Materials/M_Water_Realistic")

    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    material.set_editor_property("two_sided", True)
    material.set_editor_property("translucency_lighting_mode", unreal.TranslucencyLightingMode.TLM_SURFACE)
    material.set_editor_property("translucency_directional_lighting_intensity", 1.6)

    # Two independently panned normal maps produce crossing wave directions.
    pan_a = node(material, unreal.MaterialExpressionPanner, -1000, -250)
    pan_a.set_editor_property("speed_x", 0.018)
    pan_a.set_editor_property("speed_y", 0.011)
    pan_b = node(material, unreal.MaterialExpressionPanner, -1000, 100)
    pan_b.set_editor_property("speed_x", -0.013)
    pan_b.set_editor_property("speed_y", 0.021)

    sample_a = node(material, unreal.MaterialExpressionTextureSampleParameter2D, -720, -250)
    sample_a.set_editor_property("parameter_name", "WaterNormal_A")
    sample_a.set_editor_property("texture", normal_a)
    sample_a.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
    sample_b = node(material, unreal.MaterialExpressionTextureSampleParameter2D, -720, 100)
    sample_b.set_editor_property("parameter_name", "WaterNormal_B")
    sample_b.set_editor_property("texture", normal_b)
    sample_b.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)

    link(pan_a, "", sample_a, "Coordinates")
    link(pan_b, "", sample_b, "Coordinates")

    # A panned tangent normal drives the moving surface ripples. The second map is
    # used as a subtle micro-roughness mask rather than incorrectly adding encoded normals.
    output(sample_a, "", unreal.MaterialProperty.MP_NORMAL)

    # Color and opacity are vertex-painted per water column to encode depth.
    vertex = node(material, unreal.MaterialExpressionVertexColor, -720, 470)
    color_mask = node(material, unreal.MaterialExpressionComponentMask, -470, 420)
    color_mask.set_editor_property("r", True)
    color_mask.set_editor_property("g", True)
    color_mask.set_editor_property("b", True)
    color_mask.set_editor_property("a", False)
    link(vertex, "", color_mask, "Input")

    fresnel = node(material, unreal.MaterialExpressionFresnel, -470, 650)
    fresnel.set_editor_property("exponent", 5.0)
    fresnel.set_editor_property("base_reflect_fraction", 0.04)

    reflection_tint = node(material, unreal.MaterialExpressionVectorParameter, -470, 850)
    reflection_tint.set_editor_property("parameter_name", "ReflectionTint")
    reflection_tint.set_editor_property("default_value", unreal.LinearColor(0.66, 0.83, 0.93, 1.0))

    color_blend = node(material, unreal.MaterialExpressionLinearInterpolate, -150, 500)
    link(color_mask, "", color_blend, "A")
    link(reflection_tint, "", color_blend, "B")
    link(fresnel, "", color_blend, "Alpha")
    output(color_blend, "", unreal.MaterialProperty.MP_BASE_COLOR)

    alpha_mask = node(material, unreal.MaterialExpressionComponentMask, -470, 1120)
    alpha_mask.set_editor_property("r", False)
    alpha_mask.set_editor_property("g", False)
    alpha_mask.set_editor_property("b", False)
    alpha_mask.set_editor_property("a", True)
    link(vertex, "", alpha_mask, "Input")

    opacity_parameter = scalar(material, "WaterOpacity", 0.92, -470, 1320)
    opacity = node(material, unreal.MaterialExpressionMultiply, -150, 1120)
    link(alpha_mask, "", opacity, "A")
    link(opacity_parameter, "", opacity, "B")
    output(opacity, "", unreal.MaterialProperty.MP_OPACITY)

    rough_mask = node(material, unreal.MaterialExpressionComponentMask, -470, 1570)
    rough_mask.set_editor_property("r", True)
    rough_mask.set_editor_property("g", False)
    rough_mask.set_editor_property("b", False)
    rough_mask.set_editor_property("a", False)
    link(sample_b, "", rough_mask, "Input")
    rough_scale = node(material, unreal.MaterialExpressionMultiply, -220, 1570)
    rough_scale_constant = node(material, unreal.MaterialExpressionConstant, -470, 1780)
    rough_scale_constant.set_editor_property("r", 0.035)
    link(rough_mask, "", rough_scale, "A")
    link(rough_scale_constant, "", rough_scale, "B")
    rough_base = scalar(material, "WaterRoughness", 0.055, -470, 1950)
    roughness = node(material, unreal.MaterialExpressionAdd, 50, 1570)
    link(rough_scale, "", roughness, "A")
    link(rough_base, "", roughness, "B")
    output(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)

    specular = scalar(material, "WaterSpecular", 0.92, 50, 1780)
    output(specular, "", unreal.MaterialProperty.MP_SPECULAR)

    refraction = scalar(material, "WaterIOR", 1.333, -150, 1710)
    output(refraction, "", unreal.MaterialProperty.MP_REFRACTION)

    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    print("Created water material: " + MATERIAL_ASSET)
    return material


ensure_directory(TEXTURE_DIR)
ensure_directory(MATERIAL_DIR)
normal_a = import_normal_map("T_WaterNormal_A.png", "T_WaterNormal_A")
normal_b = import_normal_map("T_WaterNormal_B.png", "T_WaterNormal_B")
create_material(normal_a, normal_b)
print("Water material generation completed. Review shader warnings in the Output Log.")
