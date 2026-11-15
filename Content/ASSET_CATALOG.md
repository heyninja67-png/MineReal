# Catálogo de assets-fonte incluídos

Conteúdo original gerado para este projeto, sem extração de jogos comerciais.

## Texturas PBR (512×512)

Cada conjunto tem `_BC` (Base Color), `_N` (Tangent-space Normal) e `_R` (Roughness):

Grass, Dirt, Stone, Sand, Wood, Leaves, CoalOre, IronOre, Cobblestone, Planks, Gravel, Sandstone, Snow, Ice, Clay, CopperOre, GoldOre, DiamondOre.

Os mapas PNG ficam em `Content/Blocks/Source/`. A script `Scripts/create_content_assets.py` importa os arquivos e cria um material por conjunto em `/Game/Blocks/Materials/`.

## Modelos OBJ

| Arquivo | Uso previsto |
|---|---|
| SM_OakTree.obj | árvore de copa larga |
| SM_PineTree.obj | árvore cônica |
| SM_Boulder.obj | rocha ambiental |
| SM_Bush.obj | arbusto |
| SM_Creature_Sheep.obj | criatura passiva tipo ovino |
| SM_Creature_Cow.obj | criatura passiva bovina |
| SM_Creature_Chicken.obj | criatura passiva pequena |
| SM_Creature_Boar.obj | criatura passiva robusta |
| SM_Tool_Pickaxe.obj | ferramenta de mineração |
| SM_Tool_Shovel.obj | ferramenta de terreno |

São malhas estáticas de origem procedural, sem rig esquelético ou animação exportada. Os quatro modelos de criaturas podem ser importados no Editor e carregados pelo ator `AMCRCreatureActor`.

## WAV

- `AMB_Wind_Forest.wav`
- `AMB_Water_Shore.wav`
- `SFX_BlockBreak_Stone.wav`
- `SFX_BlockBreak_Wood.wav`
- `SFX_BlockPlace.wav`
- `SFX_Footstep_Grass.wav`
- `SFX_UI_Click.wav`
- `SFX_Animal_Soft.wav`

Todos foram sintetizados para o projeto, mono, 22.05 kHz, PCM 16-bit. A mixagem final e a repetição perfeita dos loops devem ser ajustadas no Unreal Audio Editor.

## Água

`Content/Water/Source/T_WaterNormal_A.png` e `T_WaterNormal_B.png` fornecem detalhe normal móvel. O material `/Game/Water/Materials/M_Water_Realistic` é gerado pelo script `Scripts/create_water_material.py`.
