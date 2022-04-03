#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <projetoCG/filesystem.h>
#include <projetoCG/shader.h>
#include <projetoCG/camera.h>
#include <projetoCG/model.h>

#include <iostream>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
void renderSphere();
void renderCube();
void renderQuad();
void renderlight();

// configurações
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 15.0f));
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;

// variaveis globais
bool firstMouse = true;
int tipo = 0;
int tipoTextura = 0;
int tipoModelo = 0;
bool displayIrradiance = false;
bool displaypPrefilter = false;
bool displayEnvCubemap = true;
bool moverObjeto = false;

bool funcaoSenX = false;
bool funcaoSenY = false;
bool funcaoSenZ = false;
bool funcaoSenLoop = false;

float fatorFuncaoSen = 5.0f;
float fatorIntensidade = 1.0f;

string meuHDR = "resources/textures/hdr/Arches_E_PineTree_3k.hdr";

float my_color_albedo[3] = { 0.0f, 0.0f, 1.0f };


// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: INICIALIZA SETUP
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Real Time Physically Based Rendering (CG 2019.1)", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	//  GLFW para capturar o mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	

	// positions of the point lights
	glm::vec3 pointLightPositions[] = {
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	// global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	// defina a função de profundidade 
	glDepthFunc(GL_LEQUAL);
	// habilite a amostragem perfeita do cubemap para níveis mais baixos de mip no mapa de pré-filtro
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// build and compile shaders
	// -------------------------

	Shader shader("resources/shaders/iluminacaopbr.vs", "resources/shaders/iluminacaopbr.fs");

	Shader lampShader("resources/shaders/lampada.vs", "resources/shaders/lampada.fs");

	Shader ourShader("resources/shaders/parte2/4.1.texture.vs", "resources/shaders/parte2/4.1.texture.fs");


	// build and compile shaders
// -------------------------
	Shader shaderTex("resources/shaders/1.2.pbr.vs", "resources/shaders/1.2.pbr.fs");

	shaderTex.use();
	shaderTex.setInt("albedoMap", 0);
	shaderTex.setInt("normalMap", 1);
	shaderTex.setInt("metallicMap", 2);
	shaderTex.setInt("roughnessMap", 3);
	shaderTex.setInt("aoMap", 4);

	Shader pbrShaderEspc("resources/shaders/parte2/pbr.vs", "resources/shaders/parte2/pbr.fs");


	Shader pbrShader("resources/shaders/pbr.vs", "resources/shaders/pbr.fs");
	Shader pbrShaderTuti("resources/shaders/pbrtuti.vs", "resources/shaders/pbr.fs");

	Shader equirectangularToCubemapShader("resources/shaders/cubemap.vs", "resources/shaders/equirectangular_to_cubemap.fs");
	Shader irradianceShader("resources/shaders/cubemap.vs", "resources/shaders/irradiance_convolution.fs");
	Shader prefilterShader("resources/shaders/cubemap.vs", "resources/shaders/prefilter.fs");
	Shader brdfShader("resources/shaders/brdf.vs", "resources/shaders/brdf.fs");
	Shader backgroundShader("resources/shaders/background.vs", "resources/shaders/background.fs");

	Shader modelShader("resources/shaders/model_loading.vs", "resources/shaders/model_loading.fs");

	// build and compile shaders
	// -------------------------
	Shader pbrShaderParte2("resources/shaders/parte2/pbr.vs", "resources/shaders/parte2/pbr.fs");
	//Shader equirectangularToCubemapShader("2.1.1.cubemap.vs", "2.1.1.equirectangular_to_cubemap.fs");
	//Shader backgroundShader("2.1.1.background.vs", "2.1.1.background.fs");


	pbrShader.use();
	pbrShader.setInt("irradianceMap", 0);
	pbrShader.setInt("prefilterMap", 1);
	pbrShader.setInt("brdfLUT", 2);
	pbrShader.setInt("albedoMap", 3);
	pbrShader.setInt("normalMap", 4);
	pbrShader.setInt("metallicMap", 5);
	pbrShader.setInt("roughnessMap", 6);
	pbrShader.setInt("aoMap", 7);

	pbrShaderTuti.use();
	pbrShaderTuti.setInt("irradianceMap", 0);
	pbrShaderTuti.setInt("prefilterMap", 1);
	pbrShaderTuti.setInt("brdfLUT", 2);
	pbrShaderTuti.setInt("albedoMap", 3);
	pbrShaderTuti.setInt("normalMap", 4);
	pbrShaderTuti.setInt("metallicMap", 5);
	pbrShaderTuti.setInt("roughnessMap", 6);
	pbrShaderTuti.setInt("aoMap", 7);

	backgroundShader.use();
	backgroundShader.setInt("environmentMap", 0);

	// load models
// -----------
	Model trooperModel(FileSystem::getPath("resources/objects/trooper/trooper.obj"));
	Model cerberosModel(FileSystem::getPath("resources/objects/cerberus/cerberus.obj"));
	Model podracerModel(FileSystem::getPath("resources/objects/podracer/podracer.obj"));
	Model robotModel(FileSystem::getPath("resources/objects/robot/robot.obj"));



	
	// load PBR material textures
	// --------------------------
	// load PBR material textures

			// military
	unsigned int militaryAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/military/albedo.png").c_str());
	unsigned int militaryNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/military/normal.png").c_str());
	unsigned int militaryMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/military/metallic.png").c_str());
	unsigned int militaryRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/military/roughness.png").c_str());
	unsigned int militaryAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/military/ao.png").c_str());
	// rusted iron
	unsigned int metal1AlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/metal1/albedo.png").c_str());
	unsigned int metal1NormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/metal1/normal.png").c_str());
	unsigned int metal1MetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/metal1/metallic.png").c_str());
	unsigned int metal1RoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/metal1/roughness.png").c_str());
	unsigned int metal1AOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/metal1/ao.png").c_str());
		// rusted iron
	unsigned int tidalAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/tidal/albedo.png").c_str());
	unsigned int tidalNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/tidal/normal.png").c_str());
	unsigned int tidalMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/tidal/metallic.png").c_str());
	unsigned int tidalRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/tidal/roughness.png").c_str());
	unsigned int tidalAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/tidal/ao.png").c_str());
	// rusted iron
	unsigned int ironAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/rusted_iron/albedo.png").c_str());
	unsigned int ironNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/rusted_iron/normal.png").c_str());
	unsigned int ironMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/rusted_iron/metallic.png").c_str());
	unsigned int ironRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/rusted_iron/roughness.png").c_str());
	unsigned int ironAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/rusted_iron/ao.png").c_str());

	// gold
	unsigned int goldAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/gold/albedo.png").c_str());
	unsigned int goldNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/gold/normal.png").c_str());
	unsigned int goldMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/gold/metallic.png").c_str());
	unsigned int goldRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/gold/roughness.png").c_str());
	unsigned int goldAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/gold/ao.png").c_str());

	// grass
	unsigned int grassAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/grass/albedo.png").c_str());
	unsigned int grassNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/grass/normal.png").c_str());
	unsigned int grassMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/grass/metallic.png").c_str());
	unsigned int grassRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/grass/roughness.png").c_str());
	unsigned int grassAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/grass/ao.png").c_str());

	// plastic
	unsigned int plasticAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/plastic/albedo.png").c_str());
	unsigned int plasticNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/plastic/normal.png").c_str());
	unsigned int plasticMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/plastic/metallic.png").c_str());
	unsigned int plasticRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/plastic/roughness.png").c_str());
	unsigned int plasticAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/plastic/ao.png").c_str());

	// wall
	unsigned int wallAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/wall/albedo.png").c_str());
	unsigned int wallNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/wall/normal.png").c_str());
	unsigned int wallMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/wall/metallic.png").c_str());
	unsigned int wallRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/wall/roughness.png").c_str());
	unsigned int wallAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/wall/ao.png").c_str());

	// trooper
	unsigned int trooperAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/trooper/albedo.png").c_str());
	unsigned int trooperNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/trooper/normal.png").c_str());
	unsigned int trooperMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/trooper/metallic.png").c_str());
	unsigned int trooperRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/trooper/roughness.png").c_str());
	unsigned int trooperAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/trooper/ao.png").c_str());

	// trooper
	unsigned int cerberosAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/cerberos/albedo.png").c_str());
	unsigned int cerberosNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/cerberos/normal.png").c_str());
	unsigned int cerberosMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/cerberos/metallic.png").c_str());
	unsigned int cerberosRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/cerberos/roughness.png").c_str());
	unsigned int cerberosAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/cerberos/ao.png").c_str());

	// podracer
	/*
	unsigned int podracerAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/podracer/albedo.png").c_str());
	unsigned int podracerNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/podracer/normal.png").c_str());
	unsigned int podracerMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/podracer/metallic.png").c_str());
	unsigned int podracerRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/podracer/roughness.png").c_str());
	unsigned int podracerAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/podracer/ao.png").c_str());
	*/
	// robot
	unsigned int robotAlbedoMap = loadTexture(FileSystem::getPath("resources/textures/pbr/robot/albedo.png").c_str());
	unsigned int robotNormalMap = loadTexture(FileSystem::getPath("resources/textures/pbr/robot/normal.png").c_str());
	unsigned int robotMetallicMap = loadTexture(FileSystem::getPath("resources/textures/pbr/robot/metallic.png").c_str());
	unsigned int robotRoughnessMap = loadTexture(FileSystem::getPath("resources/textures/pbr/robot/roughness.png").c_str());
	unsigned int robotAOMap = loadTexture(FileSystem::getPath("resources/textures/pbr/robot/ao.png").c_str());

	// lights
	// ------
	glm::vec3 lightPositions[] = {
		glm::vec3(-10.0f,  10.0f, 10.0f),
		glm::vec3(10.0f,  10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};

	// Intensidade da luz
	glm::vec3 lightColors[] = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f)
	};
	int nLinhas = 9;
	int nColunas = 7;
	float espacoEntreElas = 2.5;

	// pbr: setup framebuffer
	// ----------------------
	unsigned int captureFBO;
	unsigned int captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	

	// pbr: load HDR environment map
	// ---------------------------------
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float *data = stbi_loadf(FileSystem::getPath(meuHDR).c_str(), &width, &height, &nrComponents, 0);
	unsigned int hdrTexture;
	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // observe como especificamos o valor dos dados da textura como flutuante

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

	// pbr: setup cubemap para renderizar e anexar ao framebuffer
	// ---------------------------------------------------------
	unsigned int envCubemap;
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // ativar a amostragem mipmap pré-filtro (combater artefato de pontos visíveis)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// pbr: configurar projeção e visualizar matrizes para capturar dados nas 6 direções da face do mapa do cubo
	// ----------------------------------------------------------------------------------------------
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// pbr: converter mapa de ambiente HDR equiretangular para equivalente de cubemap
	// ----------------------------------------------------------------------
	equirectangularToCubemapShader.use();
	equirectangularToCubemapShader.setInt("equirectangularMap", 0);
	equirectangularToCubemapShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512); // não esqueça de configurar a viewport para as dimensões de captura.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularToCubemapShader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// então deixe o OpenGL gerar mipmaps a partir da primeira face do mip (combatendo o artefato de pontos visíveis)
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: crie um cubemap de irradiância e redimensione a captura do FBO para a escala de irradiância.
	// --------------------------------------------------------------------------------
	unsigned int irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// pbr: resolver integral difusa por convolução para criar uma irradiância (cube)map.
	// -----------------------------------------------------------------------------
	irradianceShader.use();
	irradianceShader.setInt("environmentMap", 0);
	irradianceShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // não esqueça de configurar a viewport para as dimensões de captura.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader.setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// pbr: crie um pré-filtro cubemap e redimensione a captura do FBO para pré-filtrar a escala.
	// --------------------------------------------------------------------------------
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
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Definir o filtro de minificação para mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// gere mipmaps para o mapa do cubo para que o OpenGL aloque automaticamente a memória necessária.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: execute uma simulação quasi monte - carlo na iluminação do ambiente para criar um pré-filtro (cube)map.
	// ----------------------------------------------------------------------------------------------------
	prefilterShader.use();
	prefilterShader.setInt("environmentMap", 0);
	prefilterShader.setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// redimensione o buffer de moldura de acordo com o tamanho do nível do mip.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.setFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader.setMat4("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// pbr: gerar uma textura de pesquisa  2D a partir das equações de BRDF usadas.
	// ----------------------------------------------------
	unsigned int brdfLUTTexture;
	glGenTextures(1, &brdfLUTTexture);

	// pré-aloque memória suficiente para a textura textura de pesquisa .
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	// definir o modo de quebra automática como GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// configura o buffer do objeto do quadro de captura e renderize o espaço no na tela com BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	brdfShader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);




	// inicialize chamadas de shader estáticos antes de renderizar
	// --------------------------------------------------
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	pbrShader.use();
	pbrShader.setMat4("projection", projection);
	backgroundShader.use();
	backgroundShader.setMat4("projection", projection);
	pbrShaderTuti.use();
	pbrShaderTuti.setMat4("projection", projection);
	pbrShaderEspc.use();
	pbrShaderEspc.setMat4("projection", projection);

	// onfigure a viewport para as dimensões da tela do buffer de estrutura original
	int scrWidth, scrHeight;
	glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
	glViewport(0, 0, scrWidth, scrHeight);


	/********************************** DEAR  ImGUI - INICIO *****************************************************/

	// Setup Dear ImGui context --------------------------------------------------------
	ImGui::CreateContext();

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	// Our state
	bool show_demo_window = true;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	/********************************** DEAR  ImGUI - FIM *****************************************************/
	//glm::vec3 luzGira[] = { glm::vec3(-10.0f,  10.0f, 10.0f) ,glm::vec3(-10.0f,  10.0f, 10.0f) ,glm::vec3(-10.0f,  10.0f, 10.0f) , glm::vec3(-10.0f,  10.0f, 10.0f) } ;

	
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{

		switch (tipo)
		{
		case 0: {
			shader.use();
			shader.setVec3("albedo", my_color_albedo[0], my_color_albedo[1], my_color_albedo[2]);
			shader.setFloat("ao", 1.0f);
			shader.setMat4("projection", projection);
			// per-frame time logic
		// --------------------
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			// input
			// -----
			processInput(window);

			// render
			// ------
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			shader.use();
			glm::mat4 view = camera.GetViewMatrix();
			shader.setMat4("view", view);
			shader.setVec3("camPos", camera.Position);

			// processar linhas * número da coluna de esferas com valores variáveis de metal / rugosidade dimensionados por linhas e colunas, respectivamente
			glm::mat4 model = glm::mat4(1.0f);
			for (int row = 0; row < nLinhas; ++row)
			{
				shader.setFloat("metallic", (float)row / (float)nLinhas);
				for (int col = 0; col < nColunas; ++col)
				{
					// Fixamos a rugosidade em 0,025 - 1,0, pois superfícies perfeitamente lisas (rugosidade de 0,0) tendem a parecer pouco
					// 
					shader.setFloat("roughness", glm::clamp((float)col / (float)nLinhas, 0.05f, 1.0f));

					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(
						(col - (nLinhas / 2)) * espacoEntreElas,
						(row - (nLinhas / 2)) * espacoEntreElas,
						0.0f
					));
					shader.setMat4("model", model);
					renderSphere();
				}
			}




			// render light source (simply re-render sphere at light positions)
			// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
			// keeps the codeprint small.
			shader.setVec3("albedo", 1.0f, 1.0f, 1.0f);
			shader.setFloat("metallic", 0.0f);
			shader.setFloat("roughness", 0.1f);
			shader.setFloat("ao", 0.0f);

			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

				if (funcaoSenX) {
					newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				}else if (funcaoSenY) {
					newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
				}else if (funcaoSenZ) {
					newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
				}
				else if (funcaoSenLoop) {
					newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));
	
				}
				else {
					newPos = lightPositions[i];
				}

				shader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
				shader.setVec3("lightColors[" + std::to_string(i) + "]", newintensidade);

				model = glm::mat4(1.0f);
				model = glm::translate(model, newPos);
				model = glm::scale(model, glm::vec3(0.5f));
				shader.setMat4("model", model);
	

			}
			// also draw the lamp object(s)
			lampShader.use();
			lampShader.setMat4("projection", projection);
			lampShader.setMat4("view", view);
			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

				if (funcaoSenX) {
					newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				}
				else if (funcaoSenY) {
					newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
				}
				else if (funcaoSenZ) {
					newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
				}
				else if (funcaoSenLoop) {
					newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

				}
				else {
					newPos = lightPositions[i];
				}

				model = glm::mat4(1.0f);
				model = glm::translate(model, newPos);
				model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
				lampShader.setMat4("model", model);
				renderlight();

			}
		
	

			/********************************** DEAR  ImGUI - INICIO *****************************************************/
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();



			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
			{

				// Create a window called "My First Tool", with a menu bar.
				ImGui::Begin("Pipeline PBR", &show_demo_window, ImGuiWindowFlags_MenuBar);

				// Edit a color (stored as ~3 floats)
				ImGui::ColorEdit3("Cor do Albedo", my_color_albedo);
				//my_color_albedo[0] = my_color[0] * (1.f / 255.f);
				//my_color_albedo[1] = my_color[1] * (1.f / 255.f);
				//my_color_albedo[2] = my_color[2] * (1.f / 255.f);
				ImGui::Text("Funcao de seno variando entre -1 e 1.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Funcao Seno X", &funcaoSenX);
				ImGui::Checkbox("Funcao Seno Y", &funcaoSenY);
				ImGui::Checkbox("Funcao Seno Z", &funcaoSenZ);
				ImGui::Checkbox("Luz Faz um loop ", &funcaoSenLoop);
				ImGui::SliderFloat("Fator Funcao Seno", &fatorFuncaoSen, 0.0f, 20.0f);
				ImGui::SliderFloat("Fator Intensidade", &fatorIntensidade, 0.0f, 100.0f);

				ImGui::SliderInt("nLinhas", &nLinhas, 0, 10);
				ImGui::SliderInt("nColunas", &nColunas, 0, 10);
				ImGui::SliderFloat("Distancia", &espacoEntreElas, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
				//ImGui::SliderFloat("Rode", &rode, 0.0f, 1000.0f);
				//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				//	counter++;
				//ImGui::SameLine();
				//ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				// Plot some values
				const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
				ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

				// Display contents in a scrolling region
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Projeto CG 2019.1 - Alison Barreiro");
	
				ImGui::End();
			}


			// Rendering
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			/********************************** DEAR  ImGUI - FIM *****************************************************/

			break;
		}
		case 1: {

			shaderTex.use();
			shaderTex.setMat4("projection", projection);
			// per-frame time logic
	   // --------------------
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			// input
			// -----
			processInput(window);

			// render
			// ------
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);




			shaderTex.use();
			glm::mat4 view = camera.GetViewMatrix();
			shaderTex.setMat4("view", view);
			shaderTex.setVec3("camPos", camera.Position);

			
		

			switch (tipoTextura) {
			case 0: {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, metal1AlbedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, metal1NormalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, metal1MetallicMap);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, metal1RoughnessMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, metal1AOMap);
				break;
			}
			case 1: {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, goldNormalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, goldAOMap);
				break;
			}
			case 2: {
				// grass
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, grassAlbedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, grassNormalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, grassMetallicMap);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, grassRoughnessMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, grassAOMap);
				break;
			}case 3: {
				// rusted iron
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, ironAlbedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, ironNormalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, ironMetallicMap);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, ironRoughnessMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, ironAOMap);
				break;
			}case 4: {
				// rusted iron
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, wallNormalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, wallAOMap);
				break;
			}case 5: {
				// plasticAlbedoMap
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, plasticAOMap);
				break;
			}case 6: {
				// military
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, militaryAlbedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, militaryNormalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, militaryMetallicMap);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, militaryRoughnessMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, militaryAOMap);
				break;
			}case 7: {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, tidalAlbedoMap);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, tidalNormalMap);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, tidalMetallicMap);
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, tidalRoughnessMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, tidalAOMap);
				break;
				
			}
			

			default:
				break;
			}

			// render rows*column number of spheres with material properties defined by textures (they all have the same material properties)
			glm::mat4 model = glm::mat4(1.0f);
			for (int row = 0; row < nLinhas; ++row)
			{
				for (int col = 0; col < nColunas; ++col)
				{
					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(
						(float)(col - (nColunas / 2)) * espacoEntreElas,
						(float)(row - (nLinhas / 2)) * espacoEntreElas,
						0.0f
					));
					shaderTex.setMat4("model", model);
					renderSphere();
				}
			}

		

			// render light source (simply re-render sphere at light positions)
			// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
			// keeps the codeprint small.


			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

				if (funcaoSenX) {
					newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				}
				else if (funcaoSenY) {
					newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
				}
				else if (funcaoSenZ) {
					newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
				}
				else if (funcaoSenLoop) {
					newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

				}
				else {
					newPos = lightPositions[i];
				}

				shaderTex.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
				shaderTex.setVec3("lightColors[" + std::to_string(i) + "]", newintensidade);

				model = glm::mat4(1.0f);
				model = glm::translate(model, newPos);
				model = glm::scale(model, glm::vec3(0.5f));
				shaderTex.setMat4("model", model);


			}
			// also draw the lamp object(s)
			lampShader.use();
			lampShader.setMat4("projection", projection);
			lampShader.setMat4("view", view);
			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

				if (funcaoSenX) {
					newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				}
				else if (funcaoSenY) {
					newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
				}
				else if (funcaoSenZ) {
					newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
				}
				else if (funcaoSenLoop) {
					newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

				}
				else {
					newPos = lightPositions[i];
				}

				model = glm::mat4(1.0f);
				model = glm::translate(model, newPos);
				model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
				lampShader.setMat4("model", model);
				renderlight();

			}



			/********************************** DEAR  ImGUI - INICIO *****************************************************/
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();



			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
			{

				// Create a window called "My First Tool", with a menu bar.
				ImGui::Begin("Textura PBR", &show_demo_window, ImGuiWindowFlags_MenuBar);
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::MenuItem("Abrir..", "Ctrl+O")) { /* Do stuff */ }
						if (ImGui::MenuItem("Salvar", "Ctrl+S")) { /* Do stuff */ }
						if (ImGui::MenuItem("Fechar", "Ctrl+W")) {}
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}
				// Edit a color (stored as ~3 floats)
				//my_color_albedo[0] = my_color[0] * (1.f / 255.f);
				//my_color_albedo[1] = my_color[1] * (1.f / 255.f);
				//my_color_albedo[2] = my_color[2] * (1.f / 255.f);
				ImGui::Text("Funcao de seno variando entre -1 e 1.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Funcao Seno X", &funcaoSenX);
				ImGui::Checkbox("Funcao Seno Y", &funcaoSenY);
				ImGui::Checkbox("Funcao Seno Z", &funcaoSenZ);
				ImGui::Checkbox("Luz Faz um loop ", &funcaoSenLoop);
				ImGui::SliderFloat("Fator Funcao Seno", &fatorFuncaoSen, 0.0f, 20.0f);
				ImGui::SliderFloat("Fator Intensidade", &fatorIntensidade, 0.0f, 100.0f);

				ImGui::SliderInt("nLinhas", &nLinhas, 0, 10);
				ImGui::SliderInt("nColunas", &nColunas, 0, 10);
				ImGui::SliderFloat("Distancia", &espacoEntreElas, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
				//ImGui::SliderFloat("Rode", &rode, 0.0f, 1000.0f);
				//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				//	counter++;
				//ImGui::SameLine();
				//ImGui::Text("counter = %d", counter);
				ImGui::SameLine();
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				// Plot some values
				const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
				ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));
				ImGui::SameLine();
				// Display contents in a scrolling region
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Projeto CG 2019.1 - Alison Barreiro");
				ImGui::SameLine();
				// Display contents in a scrolling region
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Opcoes de texturas");
				ImGui::BeginChild("Scrolling");
				//for (int n = 0; n < 10; n++)
					//ImGui::Text("%04d: Objeto X", n);
				if (ImGui::Button("Metal")) {
					tipoTextura=0;
				}else if (ImGui::Button("Ouro ")) {
					tipoTextura = 1;
				}
				else if (ImGui::Button("Grama")) {
					tipoTextura = 2;
				}
				else if (ImGui::Button("Ferro")) {
					tipoTextura = 3;
				}
				else if (ImGui::Button("Tijolo")) {
					tipoTextura = 4;
				}
				else if (ImGui::Button("Plastico")) {
					tipoTextura = 5;
				}
				else if (ImGui::Button("Military")) {
					tipoTextura = 6;
				}
				else if (ImGui::Button("Tidal Poor")) {
					tipoTextura = 7;
				}
				ImGui::EndChild();

				ImGui::End();
			}


			// Rendering
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			break;
		}
		case 2: {
			pbrShaderEspc.use();
			pbrShaderEspc.setInt("irradianceMap", 0);
			pbrShaderEspc.setInt("prefilterMap", 1);
			pbrShaderEspc.setInt("brdfLUT", 2);
			shader.setVec3("albedo", my_color_albedo[0], my_color_albedo[1], my_color_albedo[2]);
			pbrShaderEspc.setFloat("ao", 1.0f);


			// per-frame time logic
			// --------------------
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			// input
			// -----
			processInput(window);

			// render
			// ------
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// render scene, supplying the convoluted irradiance map to the final shader.
		// ------------------------------------------------------------------------------------------
			pbrShaderEspc.use();
			glm::mat4 model = glm::mat4(1.0f);
			glm::mat4 view = camera.GetViewMatrix();
			pbrShaderEspc.setMat4("view", view);
			pbrShaderEspc.setVec3("camPos", camera.Position);


			// bind pre-computed IBL data
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

			// processar linhas * número da coluna de esferas com valores variáveis de metal / rugosidade dimensionados por linhas e colunas, respectivamente
		
			for (int row = 0; row < nLinhas; ++row)
			{
				pbrShaderEspc.setFloat("metallic", (float)row / (float)nLinhas);
				for (int col = 0; col < nColunas; ++col)
				{
					// Fixamos a rugosidade em 0,025 - 1,0, pois superfícies perfeitamente lisas (rugosidade de 0,0) tendem a parecer pouco
					// 
					pbrShaderEspc.setFloat("roughness", glm::clamp((float)col / (float)nLinhas, 0.05f, 1.0f));

					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(
						(col - (nLinhas / 2)) * espacoEntreElas,
						(row - (nLinhas / 2)) * espacoEntreElas,
						0.0f
					));
					pbrShaderEspc.setMat4("model", model);
					renderSphere();
				}
			}



			// render light source (simply re-render sphere at light positions)
			// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
			// keeps the codeprint small.
			

			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

				if (funcaoSenX) {
					newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				}
				else if (funcaoSenY) {
					newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
				}
				else if (funcaoSenZ) {
					newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
				}
				else if (funcaoSenLoop) {
					newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

				}
				else {
					newPos = lightPositions[i];
				}

				pbrShaderEspc.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
				pbrShaderEspc.setVec3("lightColors[" + std::to_string(i) + "]", newintensidade);

				model = glm::mat4(1.0f);
				model = glm::translate(model, newPos);
				model = glm::scale(model, glm::vec3(0.5f));
				pbrShaderEspc.setMat4("model", model);


			}
			// also draw the lamp object(s)
			lampShader.use();
			lampShader.setMat4("projection", projection);
			lampShader.setMat4("view", view);
			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

				if (funcaoSenX) {
					newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				}
				else if (funcaoSenY) {
					newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
				}
				else if (funcaoSenZ) {
					newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
				}
				else if (funcaoSenLoop) {
					newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

				}
				else {
					newPos = lightPositions[i];
				}

				model = glm::mat4(1.0f);
				model = glm::translate(model, newPos);
				model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
				lampShader.setMat4("model", model);
				renderlight();

			}



			/********************************** DEAR  ImGUI - INICIO *****************************************************/
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();



			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
			{

				// Create a window called "My First Tool", with a menu bar.
				ImGui::Begin("Pipeline PBR", &show_demo_window, ImGuiWindowFlags_MenuBar);

				// Edit a color (stored as ~3 floats)
				ImGui::ColorEdit3("Cor do Albedo", my_color_albedo);
				//my_color_albedo[0] = my_color[0] * (1.f / 255.f);
				//my_color_albedo[1] = my_color[1] * (1.f / 255.f);
				//my_color_albedo[2] = my_color[2] * (1.f / 255.f);
				ImGui::Text("Funcao de seno variando entre -1 e 1.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Funcao Seno X", &funcaoSenX);
				ImGui::Checkbox("Funcao Seno Y", &funcaoSenY);
				ImGui::Checkbox("Funcao Seno Z", &funcaoSenZ);
				ImGui::Checkbox("Luz Faz um loop ", &funcaoSenLoop);
				ImGui::SliderFloat("Fator Funcao Seno", &fatorFuncaoSen, 0.0f, 20.0f);
				ImGui::SliderFloat("Fator Intensidade", &fatorIntensidade, 0.0f, 100.0f);

				ImGui::SliderInt("nLinhas", &nLinhas, 0, 10);
				ImGui::SliderInt("nColunas", &nColunas, 0, 10);
				ImGui::SliderFloat("Distancia", &espacoEntreElas, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
				//ImGui::SliderFloat("Rode", &rode, 0.0f, 1000.0f);
				//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				//	counter++;
				//ImGui::SameLine();
				//ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				// Plot some values
				const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
				ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));

				// Display contents in a scrolling region
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Projeto CG 2019.1 - Alison Barreiro");

				ImGui::End();
			}


			

			// render skybox (enderizar como último para evitar excesso)
			backgroundShader.use();

			backgroundShader.setMat4("view", view);
			glActiveTexture(GL_TEXTURE0);
			if (displayEnvCubemap) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
			}
			if (displayIrradiance) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
			}
			if (displaypPrefilter) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
			}

			renderCube();



			// Rendering
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			/********************************** DEAR  ImGUI - FIM *****************************************************/

	
			break;
		}
		case 3: {

			// per-frame time logic
			// --------------------
			float currentFrame = glfwGetTime();
			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;

			// input
			// -----
			processInput(window);

			// render
			// ------
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// render scene, supplying the convoluted irradiance map to the final shader.
		// ------------------------------------------------------------------------------------------
			pbrShaderTuti.use();
			glm::mat4 model = glm::mat4(1.0f);
			glm::mat4 view = camera.GetViewMatrix();
			pbrShaderTuti.setMat4("view", view);
			pbrShaderTuti.setVec3("camPos", camera.Position);


			// bind pre-computed IBL data
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

			pbrShaderTuti.use();
			pbrShaderTuti.setMat4("view", view);
			pbrShaderTuti.setVec3("camPos", camera.Position);


			switch (tipoTextura) {
			case 0: {
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, metal1AlbedoMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, metal1NormalMap);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, metal1MetallicMap);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, metal1RoughnessMap);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, metal1AOMap);
				break;
			}
			case 1: {
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, goldAlbedoMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, goldNormalMap);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, goldMetallicMap);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, goldRoughnessMap);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, goldAOMap);
				break;
			}
			case 2: {
				// grass
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, grassAlbedoMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, grassNormalMap);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, grassMetallicMap);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, grassRoughnessMap);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, grassAOMap);
				break;
			}case 3: {
				// rusted iron
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, ironAlbedoMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, ironNormalMap);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, ironMetallicMap);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, ironRoughnessMap);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, ironAOMap);
				break;
			}case 4: {
				// rusted iron
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, wallAlbedoMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, wallNormalMap);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, wallMetallicMap);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, wallRoughnessMap);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, wallAOMap);
				break;
			}case 5: {
				// plasticAlbedoMap
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, plasticAlbedoMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, plasticNormalMap);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, plasticMetallicMap);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, plasticRoughnessMap);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, plasticAOMap);
				break;
			}case 6: {
				// military
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, militaryAlbedoMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, militaryNormalMap);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, militaryMetallicMap);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, militaryRoughnessMap);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, militaryAOMap);
				break;
			}case 7: {
				glActiveTexture(GL_TEXTURE3);
				glBindTexture(GL_TEXTURE_2D, tidalAlbedoMap);
				glActiveTexture(GL_TEXTURE4);
				glBindTexture(GL_TEXTURE_2D, tidalNormalMap);
				glActiveTexture(GL_TEXTURE5);
				glBindTexture(GL_TEXTURE_2D, tidalMetallicMap);
				glActiveTexture(GL_TEXTURE6);
				glBindTexture(GL_TEXTURE_2D, tidalRoughnessMap);
				glActiveTexture(GL_TEXTURE7);
				glBindTexture(GL_TEXTURE_2D, tidalAOMap);
				break;

			}


			default:
				break;
			}




			// render rows*column number of spheres with material properties defined by textures (they all have the same material properties)
	
			for (int row = 0; row < nLinhas; ++row)
			{
				for (int col = 0; col < nColunas; ++col)
				{
					model = glm::mat4(1.0f);
					model = glm::translate(model, glm::vec3(
						(float)(col - (nColunas / 2)) * espacoEntreElas,
						(float)(row - (nLinhas / 2)) * espacoEntreElas,
						0.0f
					));
					pbrShaderTuti.setMat4("model", model);
					renderSphere();
				}
			}

			// render light source (simply re-render sphere at light positions)
			// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
			// keeps the codeprint small.

		

			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

				if (funcaoSenX) {
					newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				}
				else if (funcaoSenY) {
					newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
				}
				else if (funcaoSenZ) {
					newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
				}
				else if (funcaoSenLoop) {
					newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

				}
				else {
					newPos = lightPositions[i];
				}

				pbrShaderTuti.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
				pbrShaderTuti.setVec3("lightColors[" + std::to_string(i) + "]", newintensidade);

				model = glm::mat4(1.0f);
				model = glm::translate(model, newPos);
				model = glm::scale(model, glm::vec3(0.5f));
				pbrShaderTuti.setMat4("model", model);


			}
			// also draw the lamp object(s)
			lampShader.use();
			lampShader.setMat4("projection", projection);
			lampShader.setMat4("view", view);
			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

				if (funcaoSenX) {
					newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
				}
				else if (funcaoSenY) {
					newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
				}
				else if (funcaoSenZ) {
					newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
				}
				else if (funcaoSenLoop) {
					newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

				}
				else {
					newPos = lightPositions[i];
				}

				model = glm::mat4(1.0f);
				model = glm::translate(model, newPos);
				model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
				lampShader.setMat4("model", model);
				renderlight();

			}



			/********************************** DEAR  ImGUI - INICIO *****************************************************/
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();



			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
			{

				// Create a window called "My First Tool", with a menu bar.
				ImGui::Begin("Textura PBR", &show_demo_window, ImGuiWindowFlags_MenuBar);
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::BeginMenu("File"))
					{
						if (ImGui::MenuItem("Abrir..", "Ctrl+O")) { /* Do stuff */ }
						if (ImGui::MenuItem("Salvar", "Ctrl+S")) { /* Do stuff */ }
						if (ImGui::MenuItem("Fechar", "Ctrl+W")) {}
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}
				// Edit a color (stored as ~3 floats)
				//my_color_albedo[0] = my_color[0] * (1.f / 255.f);
				//my_color_albedo[1] = my_color[1] * (1.f / 255.f);
				//my_color_albedo[2] = my_color[2] * (1.f / 255.f);
				ImGui::Text("Funcao de seno variando entre -1 e 1.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Funcao Seno X", &funcaoSenX);
				ImGui::Checkbox("Funcao Seno Y", &funcaoSenY);
				ImGui::Checkbox("Funcao Seno Z", &funcaoSenZ);
				ImGui::Checkbox("Luz Faz um loop ", &funcaoSenLoop);
				ImGui::SliderFloat("Fator Funcao Seno", &fatorFuncaoSen, 0.0f, 20.0f);
				ImGui::SliderFloat("Fator Intensidade", &fatorIntensidade, 0.0f, 100.0f);

				ImGui::SliderInt("nLinhas", &nLinhas, 0, 10);
				ImGui::SliderInt("nColunas", &nColunas, 0, 10);
				ImGui::SliderFloat("Distancia", &espacoEntreElas, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
				//ImGui::SliderFloat("Rode", &rode, 0.0f, 1000.0f);
				//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				//	counter++;
				//ImGui::SameLine();
				//ImGui::Text("counter = %d", counter);
				ImGui::SameLine();
				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				// Plot some values
				const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
				ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));
			
				// Display contents in a scrolling region
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "Projeto CG 2019.1 - Alison Barreiro");
				
				ImGui::BeginChild("Scrolling");
				//for (int n = 0; n < 10; n++)
					//ImGui::Text("%04d: Objeto X", n);
				if (ImGui::Button("Metal")) {
					tipoTextura = 0;
				}
				else if (ImGui::Button("Ouro ")) {
					tipoTextura = 1;
				}
				else if (ImGui::Button("Grama")) {
					tipoTextura = 2;
				}
				else if (ImGui::Button("Ferro")) {
					tipoTextura = 3;
				}
				else if (ImGui::Button("Tijolo")) {
					tipoTextura = 4;
				}
				else if (ImGui::Button("Plastico")) {
					tipoTextura = 5;
				}
				else if (ImGui::Button("Military")) {
					tipoTextura = 6;
				}
				else if (ImGui::Button("Tidal Poor")) {
					tipoTextura = 7;
				}
				ImGui::EndChild();

				ImGui::End();
			}

			// render skybox (enderizar como último para evitar excesso)
			backgroundShader.use();

			backgroundShader.setMat4("view", view);
			glActiveTexture(GL_TEXTURE0);
			if (displayEnvCubemap) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
			}
			if (displayIrradiance) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
			}
			if (displaypPrefilter) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
			}

			renderCube();





			// Rendering
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			/********************************** DEAR  ImGUI - FIM *****************************************************/

			break;
		}case 4: {

		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render scene, supplying the convoluted irradiance map to the final shader.
	// ------------------------------------------------------------------------------------------
		pbrShader.use();
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = camera.GetViewMatrix();
		pbrShader.setMat4("view", view);
		pbrShader.setVec3("camPos", camera.Position);


		// bind pre-computed IBL data
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

		

		switch (tipoModelo)
		{
		case 0: {
			// cerberos
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, cerberosAlbedoMap);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, cerberosNormalMap);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, cerberosMetallicMap);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, cerberosRoughnessMap);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, cerberosAOMap);

			model = glm::mat4(3.0f);
			model = glm::translate(model, glm::vec3(0.0, -1.0, -0.5));
			model = glm::rotate(model, 68.0f, glm::vec3(0.0, 1.0, 0.0));
			pbrShader.setMat4("model", model);
			cerberosModel.Draw(pbrShader);
			break;
		}
		case 1: {
			// trooper
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, trooperAlbedoMap);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, trooperNormalMap);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, trooperMetallicMap);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, trooperRoughnessMap);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, trooperAOMap);

			model = glm::mat4(3.0f);
			model = glm::translate(model, glm::vec3(0.0, -1.0, -0.5));
			//model = glm::rotate(model, 68.0f, glm::vec3(0.0, 1.0, 0.0));
			pbrShader.setMat4("model", model);
			trooperModel.Draw(pbrShader);
			break;
		}case 2: {
	
			break;
		}case 3: {
			// podracer
		

			model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(3.0, 0.0, 2.0));
			pbrShader.setMat4("model", model);
			podracerModel.Draw(pbrShader);
			break;
		}case 4: {
			// robot
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, robotAlbedoMap);
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_2D, robotNormalMap);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_2D, robotMetallicMap);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, robotRoughnessMap);
			glActiveTexture(GL_TEXTURE7);
			glBindTexture(GL_TEXTURE_2D, robotAOMap);

			model = glm::mat4(3.0f);
			model = glm::translate(model, glm::vec3(0.0, -1.0, -0.5));
			pbrShader.setMat4("model", model);
			robotModel.Draw(pbrShader);
			break;
		}
		default:
			break;
		}




		// render light source (simply re-render sphere at light positions)
		// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
		// keeps the codeprint small.

		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
			glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

			if (funcaoSenX) {
				newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
			}
			else if (funcaoSenY) {
				newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
			}
			else if (funcaoSenZ) {
				newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
			}
			else if (funcaoSenLoop) {
				newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

			}
			else {
				newPos = lightPositions[i];
			}

			pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
			pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", newintensidade);

			model = glm::mat4(1.0f);
			model = glm::translate(model, newPos);
			model = glm::scale(model, glm::vec3(0.5f));
			pbrShader.setMat4("model", model);


		}
		// also draw the lamp object(s)
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		{
			glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
			glm::vec3 newintensidade = glm::vec3(lightColors[i].x * fatorIntensidade, lightColors[i].y * fatorIntensidade, lightColors[i].z * fatorIntensidade);

			if (funcaoSenX) {
				newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0, 0.0);
			}
			else if (funcaoSenY) {
				newPos = lightPositions[i] + glm::vec3(0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen, 0.0);
			}
			else if (funcaoSenZ) {
				newPos = lightPositions[i] + glm::vec3(0.0, 0.0, sin(glfwGetTime() * fatorFuncaoSen) * fatorFuncaoSen);
			}
			else if (funcaoSenLoop) {
				newPos = lightPositions[i] + glm::vec3(1.0f + sin(glfwGetTime()) * fatorFuncaoSen, 0.0, -(sin(glfwGetTime() / fatorFuncaoSen) * 20.0f));

			}
			else {
				newPos = lightPositions[i];
			}

			model = glm::mat4(1.0f);
			model = glm::translate(model, newPos);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			lampShader.setMat4("model", model);
			renderlight();

		}



		/********************************** DEAR  ImGUI - INICIO *****************************************************/
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();



		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{

			// Create a window called "My First Tool", with a menu bar.
			ImGui::Begin("Ferramentas PBR", &show_demo_window, ImGuiWindowFlags_MenuBar);
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Abrir..", "Ctrl+O")) { /* Do stuff */ }
					if (ImGui::MenuItem("Salvar", "Ctrl+S")) { /* Do stuff */ }
					if (ImGui::MenuItem("Fechar", "Ctrl+W")) {}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			// Edit a color (stored as ~3 floats)
			//my_color_albedo[0] = my_color[0] * (1.f / 255.f);
			//my_color_albedo[1] = my_color[1] * (1.f / 255.f);
			//my_color_albedo[2] = my_color[2] * (1.f / 255.f);
			ImGui::Text("Funcao de seno variando entre -1 e 1.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Funcao Seno X", &funcaoSenX);
			ImGui::Checkbox("Funcao Seno Y", &funcaoSenY);
			ImGui::Checkbox("Funcao Seno Z", &funcaoSenZ);
			ImGui::Checkbox("Luz Faz um loop ", &funcaoSenLoop);
			ImGui::SliderFloat("Fator Funcao Seno", &fatorFuncaoSen, 0.0f, 20.0f);
			ImGui::SliderFloat("Fator Intensidade", &fatorIntensidade, 0.0f, 100.0f);

			ImGui::SliderInt("nLinhas", &nLinhas, 0, 10);
			ImGui::SliderInt("nColunas", &nColunas, 0, 10);
			ImGui::SliderFloat("Distancia", &espacoEntreElas, 0.0f, 10.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
			//ImGui::SliderFloat("Rode", &rode, 0.0f, 1000.0f);
			//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			//	counter++;
			//ImGui::SameLine();
			//ImGui::Text("counter = %d", counter);
			ImGui::SameLine();
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			// Plot some values
			const float my_values[] = { 0.2f, 0.1f, 1.0f, 0.5f, 0.9f, 2.2f };
			ImGui::PlotLines("Frame Times", my_values, IM_ARRAYSIZE(my_values));
			
			// Display contents in a scrolling region
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Projeto CG 2019.1 - Alison Barreiro");
			
			// Display contents in a scrolling region
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "Opcoes de cubemap");
			ImGui::Checkbox("Mostrar cubemap", &displayEnvCubemap);
			ImGui::Checkbox("Irradiância difusa", &displayIrradiance);
			ImGui::Checkbox("Funcao Seno Z", &displaypPrefilter);

			ImGui::BeginChild("Scrolling");
			//for (int n = 0; n < 10; n++)
				//ImGui::Text("%04d: Objeto X", n);
			if (ImGui::Button("Cerberos")) {
				tipoModelo = 0;
			}
			else if (ImGui::Button("Trooper ")) {
				tipoModelo = 1;
			}
			else if (ImGui::Button("Robot")) {
				tipoModelo = 4;
			}
			ImGui::EndChild();
			ImGui::End();
		}
		// render skybox (enderizar como último para evitar excesso)
		backgroundShader.use();

		backgroundShader.setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		if (displayEnvCubemap) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		}
		if (displayIrradiance) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
		}
		if (displaypPrefilter) {
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap); // display prefilter map
		}

		renderCube();





		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		/********************************** DEAR  ImGUI - FIM *****************************************************/

		break;
		}

		default:
			break;
		}




		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_MOD_CONTROL) == GLFW_PRESS &&
		glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	
		float cameraSpeed = deltaTime;
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			cameraSpeed = 2.5 * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, cameraSpeed);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, cameraSpeed);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, cameraSpeed);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, cameraSpeed);
	
	

	
	if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
		tipo = 0;
	}
	else if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
		tipo = 1;
	}
	else if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
		tipo = 2;
	}
	else if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS) {
		tipo = 3;
	}
	else if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
		tipo = 4;
	}









}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3)) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		camera.ProcessMouseMovement(xoffset, yoffset);
	}
	else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359;
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y       * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y       * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		indexCount = indices.size();

		std::vector<float> data;
		for (int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
		}
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		float stride = (3 + 2 + 3) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
	}

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}
unsigned int lightVAO = 0;
unsigned int lightVBO = 0;
void renderlight()
{
	// initialize (if necessary)
	if (lightVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &lightVAO);
		glGenBuffers(1, &lightVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(lightVAO);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//glEnableVertexAttribArray(1);
		//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		//glEnableVertexAttribArray(2);
		//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(lightVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);


}
// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}
