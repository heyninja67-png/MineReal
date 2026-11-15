# Água realista — MinecraftBedrockRealistic UE5

## O que foi criado nesta etapa

A água deixou de ser desenhada como cubos azuis opacos no mesmo mesh da terra e da pedra. Agora cada chunk carregado tem uma superfície de água independente (`AMCRWaterSurface`). Isso permite usar translucidez, refração e detalhes de iluminação sem misturar a água com o material opaco do terreno.

A superfície tem:

- Malha subdividida em cada bloco de água exposto.
- Três ondas geométricas direcionais combinadas, com movimento suave.
- Fase das ondas calculada pela posição global, para evitar reinício visível nas bordas entre chunks.
- Faces verticais nas margens abertas, formando a borda do volume de água.
- Cor de água rasa mais turquesa e água profunda mais azul-escura.
- Opacidade aproximada baseada na profundidade da coluna de água.
- Dois mapas de normais originais e contínuos, movimentados em velocidades diferentes no material.
- Fresnel para intensificar a tonalidade/reflexo visual nos ângulos rasantes.
- Specular alto, roughness baixo com microvariação e entrada de refração/IOR.

## Gerar o material Unreal

Os PNGs de origem estão em `Content/Water/Source/`. O material `.uasset` é criado pelo script `Scripts/create_water_material.py` dentro da Unreal Engine, porque arquivos `.uasset` precisam ser serializados pelo Editor da versão compatível da engine.

O caminho gerado é:

`/Game/Water/Materials/M_Water_Realistic`

### No Editor

1. Abra `MinecraftBedrockRealistic.uproject` usando Unreal Engine 5.8.
2. No menu **File**, escolha **Execute Python Script**.
3. Selecione `Scripts/create_water_material.py` na pasta do projeto.
4. Aguarde a importação de `T_WaterNormal_A.png` e `T_WaterNormal_B.png` e a criação de `M_Water_Realistic`.
5. Veja o Output Log e corrija qualquer aviso/erro do Material Editor antes de testar o mapa.

### Antes de compilar para Android

O script `Build/BuildAndroid.ps1` já executa `Build/GenerateWaterAssets.ps1` antes de chamar Unreal Automation Tool. Portanto, o workflow Android também gera o material antes de cozinhar o projeto.

O computador/runner precisa ter uma instalação autorizada da Unreal Engine 5.8, com Python Script Plugin e Editor Scripting Utilities disponíveis. Defina `UE_ROOT` para a pasta raiz dessa instalação.

## Limitações conhecidas — não confundir com acabamento final

- O código C++ da superfície e as texturas-fonte estão incluídos, mas o `.uasset` é gerado ao executar o script no Editor/runner.
- A água ainda não tem natação, física de flutuação, correnteza, espuma procedural, respingos, caústicas, pós-processamento subaquático ou sons próprios.
- O Fresnel adiciona uma cor de reflexão estilizada; não equivale a reflexos completos do cenário em tempo real.
- O Unreal pode reduzir ou substituir recursos avançados de água em caminhos móveis de baixo custo. Reflexos de tela (SSR), refração e iluminação devem ser conferidos no aparelho Android de destino; não presuma qualidade de desktop em todos os telefones.
- A superfície tem ondas de geometria e normais animadas, mas a intensidade, transparência, ordenação translucente e custo de GPU ainda precisam de calibração visual em aparelho real.
- Nenhuma compilação C++, cook, APK ou teste Android foi executado neste ambiente.

## Verificação disponível aqui

`python Scripts/validate_project.py` valida a estrutura, presença dos arquivos de água, JSON do projeto e símbolos principais. `python -m py_compile Scripts/create_water_material.py Scripts/generate_water_normals.py` verifica apenas a sintaxe Python. Esses testes estáticos não substituem executar o script no Unreal Editor nem compilar o jogo.
