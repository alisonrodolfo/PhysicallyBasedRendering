# Real Time Physically Based Rendering


# Objetivo

Neste projeto vai ser uma implementação de PBR (Physically Based Rendering) em tempo real. O PBR é um visualizador para modelo 3D, utilizando mapas de ambiente HDRi e texturas de 4k.
O motor é desenvolvido é baseado em C ++ com OpenGL.

. O pipeline da PBR é escrito diretamente usando OpenGL e o visualizador usa GLFW / GLAD para gerenciamento de janelas, entradas e desenho de interface.

Este projeto utiliza High Dynamic Range (HDR) para carregar e criar um ambiente: cubemap, mapa de reflexão do pré-filtro, mapa de irradiância (iluminação global) e mapa de brdf. Por outro lado, materiais de renderização com base física são criados para armazenar texturas de modelos: albedo, tangente às normais do espaço, metálicas, rugosidade, oclusão do ambiente.



# Introdução

Uma nova e melhor forma de renderização. No uso inicial de produção de imagens geradas por computador, a renderização era baseada em rasterização, já que era a única opção viável.
A renderização com PBR é uma área ativa de pesquisa e há um grande número de equações, algoritmos e implementação para cada característica que precisa ser implementada.

O usuário vai poder deslocar pelo sistema, mover os objetos e criar uma pequena animação no cenário, escolhendo os objetos e texturas.
A Interface vai conter dados de utilização, informações de execução e as opções de definição para criação do cenário.

# Projeto:

A aplicação vai ser implementada usando Visual Studio, e bibliotecas C++ para funcionar:

   *  [glfw3.h](https://www.glfw.org/)     - biblioteca OpenGL para gerenciamento de janelas e entradas e muitas coisas.
   *  [glad.h](https://glad.dav1d.de/)     - biblioteca de carregamento do OpenGL.
   *  [glm.hpp](https://glm.g-truc.net/0.9.9/index.html)       - OpenGL Mathematics Funções de operações matemáticas 
   *  [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h)  - Carregamento de imagens [Sean Barret].
   *  [assimp](http://www.assimp.org/)       -   biblioteca de importação de modelos 3D .
   *  [imgui.h](https://github.com/ocornut/imgui)       -  biblioteca de interface gráfica.



As funções primárias são:
Ctrl+z (desfaz)
Ctrl+y (refaz)
Ctrl+d (abre o menu de objetos)


<p align="center">
	<br>
	<img src="https://lh6.googleusercontent.com/AaI3qYYdgZ0YyCbZrdDjIo8STVHmp_lAmdz5WzAx7kb23PWQLdRNzXC1x8fLYV8BtXOsnqhKpH1SlWjliVPVKkm1ExxS-KAa501Gd_nDFWj7WXDBj3YRZ1EKSCNgA-R9Lx56jyBg"/ >
	<br>
</p>


# A teoria 


Os modelos modernos de sombreamento são geralmente chamados de Baseados em Física. Eles apresentam um modelo de iluminação mais complicadp, que se separa em várias equações com três fatores intercambiáveis especiais - toda a equação forma:<br>
um B idirecional <br>
R refletância <br>
D distruibuição<br>
F função<br>
https://en.wikipedia.org/wiki/Bidirectional_reflectance_distribution_function

https://en.wikipedia.org/wiki/Specular_highlight#Cook.E2.80.93Torrance_model
<br>
(BRDF) conhecido como o modelo de Cook-Torrance . Um BRDF é essencialmente é uma função de quatro variáveis reais que definem como a luz é refletida em uma superfície opaca.<br>
Para que um modelo de iluminação PBR seja considerado, ele deve atender às três condições a seguir :<br>
Seja baseado no modelo de superfície de microfacet.<br>
Economize energia.<br>
Use um BRDF com base física.<br><br>

<p align="center">
	<br>
	<img src="prints/ndf.png"/ >
	<br>
</p>
<p>
	
Observando a imagem acima, vemos a área de reflexão especular aumentar, mas também seu brilho diminui com o aumento dos níveis de rugosidade.
<br>
Para que a conservação de energia se mantenha, precisamos fazer uma distinção clara entre luz difusa e luz especular.
A parte de reflexão que é refletida diretamente e não entra na superfície; é iluminação especular.<br>
A parte de refração restante que entra na superfície e é absorvida; é isso que conhecemos como iluminação difusa.<br>
Na parte da física, temos;  As partículas absorvem parte ou toda a energia da luz em cada colisão, que é convertida em calor.
<br>Geralmente, nem toda a energia é absorvida e a luz continuará a dispersão, espalharem uma direção (principalmente) aleatória na qual colide com outras partículas até que sua energia se esgote.
Algo importante: As superfícies metálicas reagem de maneira diferente à luz em comparação com as superfícies não metálicas.
<br>As superfícies metálicas seguem os mesmos princípios de reflexão e refração, mas toda a luz refratada é absorvida diretamente sem dispersão, deixando apenas luz refletida ou especular; superfícies metálicas não mostram cores difusas.
<br>
# A equação de refletância

Lo(p,ωo)=∫Ωfr(p,ωi,ωo)Li(p,ωi)n⋅ωidωi

A renderização baseada física segue fortemente uma versão mais especializada da equação de renderização conhecida comoequação de refletância. 

# BRDF
BRDF ou função de distribuição reflexiva bidirecional, escala o brilho recebido com base nas propriedades do material da superfície.<br>
BRDF aproxima as propriedades refletivas e refrativas do material com base na teoria de microfacetos.<br>

Existe várias formas dessa equação, podem ser encotradas em: http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
<br>Vamos escolher as mesmas funções usadas pelo Unreal Engine 4: 

<p align="center">
	<br>
	<img src="prints/brdf.PNG"/ >
	<br>
</p>
<p>
	
# Função de distribuição normal

No código GLSL, a função de distribuição normal Trowbridge-Reitz GGX se pareceria com isso:

```GLSL
float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}
```
# Função de geometria

A função geometria aproxima-se estatisticamente da área relativa da superfície, onde os detalhes da sua micro superfície se sobrepõem, causando a obstrução dos raios de luz.
<p align="center">
	<br>
	<img src="prints/geometry_shadowing.png"/ >
	<br>
</p>
<p align="center">
	<br>
	<img src="prints/geometry.png"/ >
	<br>
</p>
A função de geometria é um multiplicador entre [0,0, 1,0] com branco ou 1,0 que não mede sombreamento de microfacete e preto ou sombreamento de microfaceto completo preto ou 0,0.
<br>
No GLSL, a função geometria é convertida no seguinte código:<br>
```GLSL
float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}
```

# Equação de Fresnel

A equação de Fresnel (pronunciada como Freh-nel) descreve a proporção de luz que é refletida sobre a luz que é refratada, que varia conforme o ângulo que estamos olhando para uma superfície. No momento em que a luz atinge uma superfície, com base na superfície para visualizar o ângulo, a equação de Fresnel indica a porcentagem de luz que é refletida. A partir dessa proporção de reflexão e do princípio de conservação de energia, podemos obter diretamente a parte refratada da luz a partir de sua energia restante.
<br>
O fenômeno <br>
Olhando para a sua mesa a partir de um ângulo de quase 90 graus, você verá as reflexões se tornarem muito mais aparente. Todas as superfícies teoricamente refletem totalmente a luz se vistas de ângulos perfeitos de 90 graus.<br>
Existem algumas sutilezas envolvidas com a equação de Fresnel. Uma é que a aproximação de Fresnel-Schlick é definida para superfícies não metálicas. 
<br>
Podemos consultar diversos dados de refração em: https://refractiveindex.info/
```GLSL
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
```

# Equação de refletância de Cook-Torrance

# Criação de materiais PBR

Cada um dos parâmetros de superfície necessários para um pipeline de PBR pode ser definido ou modelado por texturas. O uso de texturas nos dá controle por fragmento sobre como cada ponto específico da superfície deve reagir à luz: se esse ponto é metálico, áspero ou liso ou como a superfície responde a diferentes comprimentos de onda da luz.

Albedo : textura especifica para cada texel da cor da superfície ou a refletividade de base, se esse texel for metálico. Isso é amplamente semelhante ao com textura difusa, mas todas as informações de iluminação são extraídas da textura. Texturas difusas geralmente têm sombras leves ou fendas escuras dentro da imagem, o que é algo que você não deseja em uma textura albedo; deve conter apenas a cor (ou coeficientes de absorção refratados) da superfície.

Normal : O mapa normal nos permite especificar por fragmento um vetor único para dar a ilusão de que uma superfície é mais irregular.

Metálico : o mapa metálico especifica por texel se um texel é metálico ou não. 

Rugosidade : o mapa de rugosidade especifica a rugosidade de uma superfície por texel. O valor amostrado da rugosidade influencia as orientações estatísticas dos microfacetes da superfície. Uma superfície mais áspera obtém reflexos mais amplos e borrados, enquanto uma superfície lisa obtém reflexos focados e claros.

AO : oclusão ambiental, especifica um fator de sombreamento extra da superfície e a geometria potencialmente circundante. Se temos uma superfície de tijolo, por exemplo, a textura albedo não deve ter informações de sombra nas fendas do tijolo. O mapa AO, no entanto, especifica essas bordas escuras, pois é mais difícil a luz escapar. A consideração da oclusão ambiental no final do estágio de iluminação pode aumentar significativamente a qualidade visual da sua cena. O mapa de oclusão ambiental de uma malha / superfície é gerado manualmente ou pré-calculado em programas de modelagem 3D.

Os artistas definem e ajustam esses valores de entrada com base em física por texel e podem basear seus valores de textura nas propriedades da superfície físicas dos materiais do mundo real. Essa é uma das maiores vantagens de um pipeline de renderização de PBR, pois essas propriedades físicas de uma superfície permanecem as mesmas, independentemente do ambiente ou da configuração de iluminação, facilitando a vida dos artistas para obter resultados fisicamente plausíveis. As superfícies criadas em um pipeline de PBR podem ser facilmente compartilhadas entre diferentes mecanismos de renderização de PBR, parecerão corretas independentemente do ambiente em que estão e, como resultado, parecerão muito mais naturais.

# A Iluminação

Agora que temos a equação de refletância final, podemos calcular nossa iluminação.<br>
Podemos pegar diretamente o número (conhecido) de fontes de luz e calcular sua irradiância total, uma vez que cada fonte de luz possui apenas uma única direção de luz que influencia o brilho da superfície. Isso torna as fontes de luz em PBR, relativamente simples, pois, efetivamente, precisamos apenas fazer um loop sobre as fontes de luz que contribuem.

## Um modelo de superfície PBR
Primeiro, precisamos pegar as entradas relevantes de PBR necessárias para proteger a superfície:
```GLSL
#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
  
uniform vec3 camPos;
  
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
```
Então, no início do shader de fragmento, fazemos os cálculos habituais necessários para qualquer algoritmo de iluminação:
<br>
```GLSL
void main()
{
    vec3 N = normalize(Normal); 
    vec3 V = normalize(camPos - WorldPos);
    [...]
}
```

# PBR texturizado
```GLSL
[...]
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
  
void main()
{
    vec3 albedo     = pow(texture(albedoMap, TexCoords).rgb, 2.2);
    vec3 normal     = getNormalFromNormalMap();
    float metallic  = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao        = texture(aoMap, TexCoords).r;
    [...]
}
```

# A Iluminação Baseada em Imagem

## Trata o ambiente ao redor como uma grande fonte de luz.<br>
## Cada pixel agora é um emissor de luz

Isso geralmente é conseguido através da manipulação de um mapa do ambiente cubemap (tirado do mundo real ou gerado a partir de uma cena 3D), para que possamos usá-lo diretamente em nossas equações de iluminação: tratando cada pixel do cubemap como um emissor de luz. Dessa forma, podemos capturar efetivamente a iluminação global e a sensação geral de um ambiente, dando aos objetos uma melhor sensação de pertencer ao ambiente.
<br>
## Precisamos de alguma maneira recuperar o brilho da cena, dado qualquer vetor de direção W.
## A resolução da integral precisa ser rápida e em tempo real.
<br>
Dado um cubemap, podemos visualizar cada texel do cubemap como uma única fonte de luz emissora. Amostrando este cubemap com qualquer vetor de direção W, recuperamos o brilho da cena nessa direção.
<br>
```GLSL
vec3 radiance = texture(_cubemapEnvironment, w_i).rgb;
```
<br>
Para resolver a integral de uma maneira mais eficiente, queremos pré-processar ou pré-cálcular a maioria de seus cálculos.
<br>
Isso nos dá uma integral que depende apenas de W (assumindo que está no centro do mapa do ambiente). Com esse conhecimento, podemos calcular ou pré-calcular um novo mapa do cubo que armazena em cada direção da amostra (ou texel), resultando em integral difusa por convolução.

## Geramos uma mapa de irradiância
<p align="center">
	<br>
	<img src="prints/ibl_irradiance.png"/ >
	<br>
</p>

Armazenando o resultado complicado em cada texel do cubemap (na direção de W) o mapa de irradiância é semelhante a uma exibição média de cores ou iluminação do ambiente. A amostragem de qualquer direção desse mapa do ambiente nos dará a irradiância da cena nessa direção específica.

https://www.indiedb.com/features/using-image-based-lighting-ibl

# PBR e HDR

A diferença entre uma simples lâmpada ou o sol são seus equivalentes físicos.<br>
Sem trabalhar em um ambiente de renderização HDR , é impossível especificar corretamente a intensidade relativa de cada luz.

# O formato do arquivo radiância HDR

O formato de arquivo brilho (com extensão .hdr) armazena uma Cubemap completa com todas as 6 faces como dados de ponto flutuante permitindo qualquer pessoa especificar valores de cores fora do 0.0 para 1.0, para suas intensidades de cores corretas.<br>
HDR gratuitos: http://www.hdrlabs.com/sibl/archive.html
<br>
Esse mapa do ambiente é projetado de uma esfera para um plano, para que possamos armazenar mais facilmente o ambiente em uma única imagem conhecida como mapa equirectangular.
<br>
# HDR e stb_image.h
A popular biblioteca de cabeçalho stb_image.h suporta o carregamento de imagens HDR

```C
#include "stb_image.h"
[...]

stbi_set_flip_vertically_on_load(true);
int width, height, nrComponents;
float *data = stbi_loadf("imagem.hdr", &width, &height, &nrComponents, 0);
unsigned int hdrTexture;
if (data)
{
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
}
else
{
    std::cout << "Failed to load HDR image." << std::endl;
}  
```
stb_image.h mapeia automaticamente os valores HDR para uma lista de valores de ponto flutuante. Isso é tudo o que precisamos para armazenar o mapa do ambiente HDR equirectangular em uma textura de ponto flutuante 2D.

<br>
# Do equirectangular ao Cubemap

Para converter uma imagem equirectangular em um cubemap, precisamos renderizar um cubo e projetar o mapa equirectangular em todas as faces do cubo por dentro e tirar 6 imagens de cada lado do cubo como uma face de mapa de cubos. O shader de vértice desse cubo simplesmente renderiza o cubo como está e passa sua posição local para o shader de fragmento como um vetor de amostra 3D:
```GLSL
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 localPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    localPos = aPos;  
    gl_Position =  projection * view * vec4(localPos, 1.0);
}
```

Para o shader de fragmento, colorimos cada parte do cubo como se dobrássemos o mapa equirectangular em cada lado do cubo. Para fazer isso, tomamos a direção da amostra do fragmento como interpolada da posição local do cubo e usamos esse vetor de direção e alguma mágica da trigonometria para amostrar o mapa equiretangular como se fosse um cubemap. Armazenamos diretamente o resultado no fragmento da face do cubo, que deve ser tudo o que precisamos fazer:

```GLSL
#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(localPos)); // make sure to normalize localPos
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}
```
Obviamente, também geramos o cubemap correspondente, pré-alocando memória para cada uma de suas 6 faces<br>
Pegamos o anexo colorido do buffer de estrutura e alternamos seu destino de textura para todas as faces do mapa do cubo, renderizando diretamente a cena em uma das faces do cubemap. Depois que essa rotina terminar (o que precisamos fazer apenas uma vez), o cubemap  envCubemap deve ser a versão do ambiente cubemap da nossa imagem HDR original.<br>
E passamos isso tudo para um shader skybox:
```GLSL
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 localPos;

void main()
{
    localPos = aPos;

    mat4 rotView = mat4(mat3(view)); // remove translation from the view matrix
    vec4 clipPos = projection * rotView * vec4(localPos, 1.0);

    gl_Position = clipPos.xyww;
}
```
```GLSL
#version 330 core
out vec4 FragColor;

in vec3 localPos;
  
uniform samplerCube environmentMap;
  
void main()
{
    vec3 envColor = texture(environmentMap, localPos).rgb;
    
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
  
    FragColor = vec4(envColor, 1.0);
}
```

Amostramos o mapa do ambiente usando suas posições de cubo de vértice interpoladas que correspondem diretamente ao vetor de direção correto a ser amostrado.
<br>
Como os componentes da câmera são ignorados, a renderização desse shader sobre um cubo deve fornecer o mapa do ambiente como um fundo sem movimento. Além disso, quase todos os mapas HDR estão no espaço linear de cores por padrão, portanto, precisamos aplicar a correção gama antes de gravar no buffer de moldura padrão.
<br>
# Convolução do Cubemap

Nosso principal objetivo é resolver a integral de toda a iluminação indireta difusa, dada a irradiância da cena na forma de um mapa de ambiente de cubemap.<br>

No entanto, é computacionalmente impossível amostrar a iluminação do ambiente de todas as direções possíveis Ω, o número de direções possíveis é teoricamente infinito. No entanto, podemos aproximar o número de direções tomando um número finito de direções ou amostras, espaçadas uniformemente ou tomadas aleatoriamente dentro do hemisfério para obter uma aproximação bastante precisa da irradiância, resolvendo efetivamente a integral ∫ discretamente.
<br>
Exite diversas formas de fazer isso, o deial é pesquisar a computacionalmente possível<br>
## pré-calculado environmentMap
Agora, após essa rotina, devemos ter um mapa de irradiância pré-calculado que possamos usar diretamente para nossa iluminação difusa baseada em imagem. Para verificar se convolutamos com sucesso o mapa do ambiente, vamos substituir o mapa do ambiente pelo mapa de irradiância como o amostrador de ambiente do skybox<br>
O resultado é uma versão muito borrada do mapa do ambiente. <br>

# PBR e iluminação indireta por irradiância

O mapa de irradiância representa a parte difusa da integral de refletância acumulada de toda a luz indireta ao redor. Visto que a luz não provém de nenhuma fonte de luz direta, mas do ambiente ao redor tratamos tanto a iluminação indireta difusa quanto a especular como a iluminação ambiente, substituindo nosso termo constante previamente definido.
<br>
Como a luz ambiente vem de todas as direções dentro do hemisfério orientada em torno do N normal não existe um único vetor intermediário para determinar a resposta de Fresnel. 
<br>
Para simular ainda o Fresnel, calculamos o Fresnel a partir do ângulo entre o vetor normal e o vetor de visualização. <br>
<br>
Como você pode ver, o cálculo real da iluminação baseada em imagem é bastante simples e requer apenas uma única pesquisa de textura de cubemap; a maior parte do trabalho consiste em pré-calcular ou envolver o mapa do ambiente em um mapa de irradiância.
<br>

# Iluminação Baseada em Imagem - especular

* Verificar equação de refletância

Tentar resolver a integral para todas as direções de luz recebidas, incluindo todas as direções de vista possíveis, é uma sobrecarga combinatória e é muito caro para calcular em tempo real. <br>
A Epic Games propôs uma solução em que eles foram capazes de pré-convocar a parte especular para fins em tempo real, considerando alguns compromissos, conhecidos como aproximação de soma dividida.
<br>
A aproximação da soma dividida divide a parte especular da equação de refletância em duas partes separadas, que podemos convoluir individualmente e depois combinar no shader PBR para iluminação indireta especular baseada em imagem. De maneira semelhante à pré-convolução do mapa de irradiância, a aproximação da soma dividida requer um mapa do ambiente HDR como entrada de convolução.
<br>
Para cada nível de rugosidade que envolvemos, armazenamos os resultados sequencialmente mais borrados nos níveis do mapa mipmap pré-filtrado. Por exemplo, um mapa de ambiente pré-filtrado que armazena o resultado pré-complicado de 5 valores diferentes de rugosidade em seus 5 níveis de mipmap é o seguinte:
<p align="center">
	<br>
	<img src="prints/ibl_prefilter_map.png"/ >
	<br>
</p>

# Pré-filtrando um mapa do ambiente HDR

A pré-filtragem de um mapa do ambiente é bastante semelhante ao mapa de irradiância. A diferença é que agora consideramos a rugosidade e armazenamos reflexões sequencialmente mais ásperas nos níveis de mip do mapa pré-filtrado.
<br>

## glGenerateMipmap

```C
unsigned int prefilterMap;
glGenTextures(1, &prefilterMap);
glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
for (unsigned int i = 0; i < 6; ++i)
{
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
}
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
```

omo planejamos fazer uma amostra do prefilterMap de seus mipmaps, será necessário garantir que o filtro de minificação esteja definido como GL_LINEAR_MIPMAP_LINEAR para ativar a filtragem trilinear. <br>

O que resta a fazer é permitir que o OpenGL pré-filtre o mapa do ambiente com diferentes valores de rugosidade em vários níveis do mipmap.

Existe diversos calculos a serem feitos agora, consultar mais sobre o assunto.


# Concluindo a refletância do IBL

Para colocar a parte especular indireta da equação de refletância em funcionamento, precisamos costurar as duas partes da aproximação da soma dividida. Vamos começar adicionando os dados de iluminação pré-calculados à parte superior do nosso shader PBR:
<br>
```GLSML
vec3 F = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

vec3 kS = F;
vec3 kD = 1.0 - kS;
kD *= 1.0 - metallic;	  
  
vec3 irradiance = texture(irradianceMap, N).rgb;
vec3 diffuse    = irradiance * albedo;
  
const float MAX_REFLECTION_LOD = 4.0;
vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;   
vec2 envBRDF  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
vec3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);
  
vec3 ambient = (kD * diffuse + specular) * ao; 
```
Agora, executando esse código exato nas séries de esferas que diferem por sua rugosidade e propriedades metálicas, finalmente conseguimos ver suas cores verdadeiras no renderizador PBR final.

Primeiro, o pré-cálculo realmente precisa ser feito apenas uma vez, e não em toda inicialização. E segundo, no momento em que você usa vários mapas de ambiente, você terá que pré-calcular cada um deles em cada inicialização que tende a se acumular.

Por esse motivo, você geralmente pré-calcula um mapa do ambiente em um mapa de irradiância e pré-filtro apenas uma vez e depois o armazena em disco (observe que o mapa de integração do BRDF não depende de um mapa do ambiente, portanto, você só precisa calcule ou carregue uma vez). Isso significa que você precisará criar um formato de imagem personalizado para armazenar mapas de cubo HDR, incluindo seus níveis mip. Ou você o armazena (e carrega) como um dos formatos disponíveis (como .dds, que suporta o armazenamento de níveis mip).

# Obs:

Por algum motivo cubemap ficou de cabeça para baixo:

Soluação: no shader de fragmentos PBR na linha R = reflect(-V,N) - flip o sinal de V.


</p>
