# MCR Realistic — projeto-fonte Unreal Engine 5.8 para Android

**Estado honesto da entrega:** base de jogo voxel em C++ ampliada, com sistemas de mundo, interação, inventário, crafting, sobrevivência, ambiente e conteúdo original. **Não é uma recriação completa do Minecraft Bedrock e não foi compilada/testada na Unreal Engine neste ambiente.** O APK não está incluído.

Este projeto preserva a identidade de um sandbox de blocos, usando iluminação, materiais PBR, água animada e modelos low-poly/voxel originais. Os assets deste pacote foram gerados para o projeto; não há código, texturas, sons ou modelos extraídos do Minecraft.

## Conteúdo real incluído

### Código C++

- `AMCRWorldManager`: carregamento de chunks em torno do jogador, persistência de alterações, autosave e spawn inicial de fauna.
- `AMCRBlockChunk`: terreno procedural 16×16×64, face culling básico, malhas de colisão e uma seção de malha por tipo de bloco para permitir materiais distintos.
- `AMCRWaterSurface`: superfície independente, três componentes de onda com fase global, coloração por profundidade e laterais de água expostas.
- `AMCRPlayerCharacter`: primeira pessoa, movimento, pulo, agachamento, mineração, colocação, modo criativo e dano por queda.
- `UMCRInventoryComponent`: 36 espaços, hotbar de 9, empilhamento, movimentação/troca, dividir pilha e adicionar/remover blocos.
- `UMCRCraftingComponent`: receitas para tábuas, bancada, pedregulho, vidro, fornalha, baú e tochas.
- `UMCRSurvivalComponent`: vida, fome, regeneração e dano por fome.
- `AMCRDayNightWeather`: ciclo de dia/noite, iluminação direcional, sky light e estados de clima claro/nublado/chuva/tempestade.
- `AMCRCreatureActor`: animais geométricos com movimento errante simples, reação de afastamento e vocalização.
- `UMCRMobileHUD` e `UMCRInventoryWidget`: HUD touch, vida/fome, seleção de hotbar e painel de inventário com troca/divisão de pilhas e botões de receitas.
- `UMCRWorldSaveGame`: alterações de blocos, inventário, vida/fome, hora e clima.

### Conteúdo artístico original em formato-fonte

| Conteúdo | Quantidade | Formato |
|---|---:|---|
| Texturas PBR de blocos | 18 conjuntos × 3 mapas | PNG 512×512 |
| Mapas de água | 2 | PNG 512×512 |
| Modelos originais | 10 | OBJ + MTL |
| Áudio original sintetizado | 8 | WAV mono, 22.05 kHz, 16-bit |

Os mapas PBR são Base Color, Normal e Roughness para grama, terra, pedra, areia, madeira, folhas, minérios e outros materiais. Os modelos OBJ incluem árvores, arbusto, rocha, quatro animais e duas ferramentas de mineração/jardinagem. São malhas-fonte estáticas; não são rigs esqueléticos.

## Funcionalidades já presentes e limitações

| Sistema | Estado desta base |
|---|---|
| Chunks e terreno procedural | Implementado; geração e construção da malha ainda ocorrem na game thread |
| Biomas | Variações de planície, floresta, deserto e área fria; não há simulação completa de biomas |
| Cavernas e minérios | Ruído de caverna e distribuição procedural de minérios; ainda simples |
| Água | Ondas geométricas, normais em movimento no material, Fresnel, refração e cor por profundidade; precisa de teste visual em GPU mobile |
| Blocos | Quebrar/colocar, tipos de bloco, materiais PBR por seção e salvamento das edições |
| Inventário | 36 espaços, hotbar 9, empilhar, trocar/mover e dividir por botão; sem drag-and-drop por gesto |
| Crafting | Receitas básicas conectadas a botões na tela de inventário |
| Baú e fornalha | Itens/blocos podem ser fabricados, mas ainda não existem telas e lógica completas de armazenamento/cozimento |
| Sobrevivência | Vida, fome, regeneração, dano por fome e queda; ainda sem comida utilizável, armadura ou morte/respawn completo |
| Criativo | Alternância por tecla C e botão touch; voo inicial simplificado |
| Criaturas | Quatro tipos passivos com deslocamento errante; sem aldeões, hostis, reprodução, navegação avançada ou animações esqueléticas |
| Dia/noite | Ciclo e iluminação dinâmica presentes; sky capture não é atualizado continuamente |
| Clima | Estados claro/nublado/chuva/tempestade alteram luz e neblina; chuva visual, relâmpagos e partículas ainda faltam |
| Áudio | Ambiência, passos, quebra, colocação, interface e vocalização incluídos como WAV; mixagem e áudio espacial precisam de ajuste |
| UI | HUD touch e inventário/crafting básico; ainda faltam menu inicial completo, lista/criação de mundos, perfis e menu de configurações completo |
| Água avançada | Não há espuma, caústicas, correnteza física, respingos, reflexos de tela completos ou pós-processamento subaquático |
| Otimização | Culling de faces e streaming por raio; greedy meshing, jobs assíncronos, LOD, pooling e profiling ainda faltam |
| Android | Configuração ARM64 e pipeline UAT; nenhum APK foi produzido neste ambiente |

A base não usa botões decorativos para declarar funcionalidades que não existem. Os sistemas acima são implementações iniciais e precisam de balanceamento, correção e teste no Editor.

## Abrir e preparar no Unreal Editor

1. Instale uma versão autorizada da **Unreal Engine 5.8** com suporte C++.
2. Configure o Android SDK/NDK/JDK para a versão exata da engine. A documentação atual da Epic lista Android SDK target 35 (mínimo 34 para compilar), NDK r27c, Build-tools 35.0.1 e OpenJDK 21.0.3.
3. Extraia o ZIP e abra `MinecraftBedrockRealistic.uproject`.
4. Gere os project files e compile o módulo C++ quando o Editor solicitar.
5. Antes de executar o jogo, gere os assets importados. No Windows PowerShell, defina `UE_ROOT` e execute:

```powershell
$env:UE_ROOT = "D:\Epic Games\UE_5.8"
.\Build\GenerateGameAssets.ps1
```

Esse processo importa as texturas PNG, OBJ e WAV, cria os materiais PBR de blocos, o material de tintura das criaturas e o material de água. Os `.uasset` são produzidos pela instalação local da Unreal; os arquivos-fonte permanecem no ZIP.

6. Abra o mapa padrão da Engine (Entry) e pressione Play.

### Controles

- WASD: mover; mouse: olhar; Espaço: pular; Ctrl: agachar.
- Clique esquerdo: quebrar bloco; clique direito: colocar bloco.
- E/roda do mouse: próximo espaço da hotbar; I: inventário.
- C: alternar modo criativo.
- Android: joystick virtual da Unreal, botões Minar/Colocar/Pular/Item+/Mochila/Criativo e hotbar por toque.

No inventário, toque em um espaço e depois em outro para mover/trocar. O botão Dividir Pilha divide o espaço selecionado. Os botões de receita tentam fabricar o item e só concluem se houver ingredientes e espaço livre.

## Compilar Android localmente

```powershell
$env:UE_ROOT = "D:\Epic Games\UE_5.8"
.\Build\BuildAndroid.ps1
```

O script executa a geração/importação dos assets e depois chama `RunUAT BuildCookRun` para Android Development ARM64. O pacote sai em `Build/AndroidPackage`. O tamanho final dependerá da versão da Engine, configuração de símbolos, texturas, shaders, plugins e opções de empacotamento; o tamanho do ZIP de fontes **não** prevê o tamanho do APK.

## GitHub Actions

O workflow `.github/workflows/android-build.yml` usa um **runner Windows self-hosted** com labels `self-hosted`, `Windows`, `X64`, `ue-5-8`, `android`. Configure a variável de repositório `UE_ROOT` apontando para a instalação autorizada da Unreal Engine no runner.

O runner também precisa ter Visual Studio C++ Build Tools compatíveis com a engine, SDK/NDK/JDK Android e espaço em disco/RAM suficientes para compilar e cozinhar o projeto. Um runner hospedado padrão do GitHub não fornece uma instalação licenciada da Unreal Engine automaticamente.

## Regerar os assets originais

Os assets-fonte já estão incluídos. Para reproduzi-los, com Python 3 e NumPy/Pillow instalados:

```bash
python Scripts/generate_original_assets.py
```

Depois, execute `Build/GenerateGameAssets.ps1` dentro do Windows com UE 5.8 para importar os arquivos ao Content Browser e gerar os materiais `.uasset`.

## Validação disponível

Execute:

```bash
python Scripts/validate_project.py
```

A validação verifica estrutura, JSON do `.uproject`, presença e integridade estrutural de PNG/WAV/OBJ, quantidade de assets e configurações Android. **Não** compila C++, não executa UnrealHeaderTool, não importa os assets no Editor, não faz cook, não gera APK e não testa em aparelho.

## Licenças e origem

Os PNGs, OBJ e WAV desta entrega foram sintetizados/gerados para este projeto. O código-fonte é fornecido para estudo e modificação. Não há assets de Minecraft nem downloads de bibliotecas de terceiros incluídos. Caso assets externos sejam adicionados depois, verifique a licença individual e mantenha atribuições exigidas.

## Referências oficiais

- Unreal Engine 5.8 Android Quick Start: https://dev.epicgames.com/documentation/unreal-engine/android-quick-start
- Requisitos Android UE 5.8: https://dev.epicgames.com/documentation/unreal-engine/android-development-requirements-for-unreal-engine
- Importação de conteúdo via Interchange/Python: https://dev.epicgames.com/documentation/unreal-engine/importing-assets-using-interchange-in-unreal-engine
- Empacotamento com UAT: https://dev.epicgames.com/documentation/unreal-engine/build-operations-cooking-packaging-deploying-and-running-projects-in-unreal-engine
