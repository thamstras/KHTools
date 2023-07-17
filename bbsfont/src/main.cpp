#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "stb_image.h"
#include "stb_image_write.h"
#include "Texture.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"

#include <cstdint>
typedef uint16_t ushort;
typedef uint32_t uint;
typedef int8_t byte;
typedef uint8_t ubyte;

namespace fs = std::filesystem;

template<typename T1,
		typename std::enable_if<std::is_trivially_copyable_v<T1>
								&& std::is_standard_layout_v<T1>>::type* = nullptr>
T1 ReadStream(std::istream& stream)
{
	T1 var;
	stream.read((char*)(&var), sizeof(T1));
	return var;
}

// #### STRUCTS ####
struct MouseData
{
	double xPos;
	double yPos;
	float deltaX;
	float deltaY;
	bool firstData;
	bool rawMotionAvailible;
};

struct WindowData
{
	GLFWwindow* window;
	int width;
	int height;
	bool hasCursorLock;
};

// #### SETTINGS ####
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

const bool USE_ANTIALIASING = false;
const bool USE_ANISOTROPIC = false;

const float MAX_DELTA_TIME = 0.2f;

const bool DISABLE_MOUSELOCK = true;

// #### GLOBALS ####
WindowData g_window;
MouseData g_mouse = { 0.0, 0.0, 0.0f, 0.0f, true, false };
//Camera g_camera(glm::vec3(0.0f, 1.5f, -3.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;
double g_worldTime = 0.0f;

struct BMP_FONT_INFO {
	ushort strNum;
	ushort imageWidth;
	ushort imageHeight;
	byte width;
	byte height;
};

struct BBS_FILE_INF {
	BMP_FONT_INFO info;
};

struct BMP_FONT_PALETTE {
	uint RGBA[16];
};

struct BBS_FILE_CLU {
	BMP_FONT_PALETTE palettes[2][2];
};

struct STR_CODE {
	ushort code;
	ushort u;
	ushort v;
	byte clut;
	byte horizontalAdvance;
};

struct FONT
{
	BMP_FONT_PALETTE palette[2][2];
	BMP_FONT_INFO info;
	uint defTexImageHeight;
	byte fontSize;
	byte baseLine;
	byte JpToEnSizePix;
	STR_CODE* pStrCode;
	char* pTexture;
	uint pTexture_size;
};

struct Image
{
	uint width;			// Image width in pixels
	uint height;		// Image height in pixels
	ubyte pixelSize;	// Pixel size in bytes. Pretty much always 4 (RGBA8888) or 3 (RGB888)
	char* pixelData;	// width * height * pixelSize bytes of data
};

FONT* g_loadedFont;

void LoadFont(fs::path inf, fs::path clu, fs::path mtx, fs::path cod, bool isHelpFont)
{
	if (g_loadedFont != nullptr) delete g_loadedFont;
	g_loadedFont = new FONT();

	std::ifstream inf_stream = std::ifstream(inf);
	g_loadedFont->info = ReadStream<BMP_FONT_INFO>(inf_stream);
	inf_stream.close();

	std::ifstream clu_stream = std::ifstream(clu);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			g_loadedFont->palette[j][i] = ReadStream<BMP_FONT_PALETTE>(clu_stream);
	clu_stream.close();

	if (isHelpFont) g_loadedFont->info.strNum += 1;
	
	std::ifstream cod_stream = std::ifstream(cod);
	g_loadedFont->pStrCode = new STR_CODE[g_loadedFont->info.strNum];
	for (int i = 0; i < g_loadedFont->info.strNum; i++) g_loadedFont->pStrCode[i] = ReadStream<STR_CODE>(cod_stream);
	if (isHelpFont)
	{
		ushort count = g_loadedFont->info.strNum;
		g_loadedFont->pStrCode[count - 1].code = 0x9394;
		g_loadedFont->pStrCode[count - 1].u = 0x1c0;
		g_loadedFont->pStrCode[count - 1].v = 0x150;
		g_loadedFont->pStrCode[count - 1].clut = 0xc;
	}
	cod_stream.close();

	uint size = fs::file_size(mtx);
	std::ifstream mtx_stream = std::ifstream(mtx);
	g_loadedFont->pTexture = new char[size];
	mtx_stream.read(g_loadedFont->pTexture, size);
	g_loadedFont->pTexture_size = size;

	auto infoWidth = g_loadedFont->info.width;
	g_loadedFont->defTexImageHeight = 0x200;
	if ((0x200 / infoWidth) * infoWidth != 0x200)
	{
		g_loadedFont->defTexImageHeight = (short)(0x200 / infoWidth) * (ushort)infoWidth;
	}

	/*for (int t = 0; t < 2; t++)
	{
		auto imageWidth = g_loadedFont->info.imageWidth;
		auto texHeight = ((int)size / (int)imageWidth) << 1;
		g_loadedFont->pSurface[t] = MakeTexture(imageWidth, texHeight, imageWidth, texHeight, (void*)(g_loadedFont->pTexture + (0x20000 * t)), (void*)(g_loadedFont->palette));
		
	}*/
	mtx_stream.close();
}

uint g_texWidth;
uint g_texHeight;
uint g_imgWidth;
uint g_imgHeight;
uint g_imgCount;

bool gui_InputByte(const char* label, byte* v, byte step = 1, byte fast_step = 10, ImGuiInputTextFlags flags = 0)
{
	return ImGui::InputScalar(label, ImGuiDataType_S8, (void*)v, &step, &fast_step, "%d", flags);
}

bool gui_InputUShort(const char* label, ushort* v, ushort step = 1, ushort fast_step = 10, ImGuiInputTextFlags flags = 0)
{
	return ImGui::InputScalar(label, ImGuiDataType_U16, (void*)v, &step, &fast_step, "%u", flags);
}

bool gui_InputUInt(const char* label, uint* v, uint step = 1, uint fast_step = 10, ImGuiInputTextFlags flags = 0)
{
	return ImGui::InputScalar(label, ImGuiDataType_U32, v, &step, &fast_step, "%u", flags);
}

void gui_FontInfo()
{
	if (ImGui::Begin("Font Info"))
	{
		gui_InputUShort("StrNum", &(g_loadedFont->info.strNum));
		gui_InputUShort("ImageWidth", &(g_loadedFont->info.imageWidth));
		gui_InputUShort("ImageHeight", &(g_loadedFont->info.imageHeight));
		gui_InputByte("Width", &(g_loadedFont->info.width));
		gui_InputByte("Height", &(g_loadedFont->info.height));
		ImGui::Separator();
		gui_InputUInt("DefTexImageHeight", &(g_loadedFont->defTexImageHeight));
		gui_InputByte("FontSize", &(g_loadedFont->fontSize));
		gui_InputByte("BaseLine", &(g_loadedFont->baseLine));
		gui_InputByte("JpToEnSizePix", &(g_loadedFont->JpToEnSizePix));
		ImGui::Separator();
		ImGui::Text("pTexture_size: %u", g_loadedFont->pTexture_size);
	}
	ImGui::End();
}

void gui_CodePoints()
{
	if (ImGui::Begin("Code Points"))
	{
		for (int c = 0; c < g_loadedFont->info.strNum; c++)
		{
			if (ImGui::TreeNode((void*)(c), "Code point %d", c))
			{
				gui_InputUShort("Code", &(g_loadedFont->pStrCode[c].code));
				gui_InputUShort("Code", &(g_loadedFont->pStrCode[c].u));
				gui_InputUShort("Code", &(g_loadedFont->pStrCode[c].v));
				gui_InputByte("Code", &(g_loadedFont->pStrCode[c].clut));
				gui_InputByte("Code", &(g_loadedFont->pStrCode[c].horizontalAdvance));
				ImGui::TreePop();
			}
		}
	}
	ImGui::End();
}

ImVec4 ConvertColor(uint col32)
{
	// Implicit conversion
	return ImColor(col32);
}

void gui_WidgetPalette(BMP_FONT_PALETTE* pal)
{
	for (int r = 0; r < 2; r++)
	{
		for (int c = 0; c < 8; c++)
		{
			ImGui::ColorButton("pal", ConvertColor(pal->RGBA[8 * r + c]));
		}
	}
}

void gui_Palettes()
{
	if (ImGui::Begin("Palettes"))
	{
		if (ImGui::TreeNode("Palette A1"))
			gui_WidgetPalette(&g_loadedFont->palette[0][0]);
		if (ImGui::TreeNode("Palette A2"))
			gui_WidgetPalette(&g_loadedFont->palette[0][1]);
		if (ImGui::TreeNode("Palette B1"))
			gui_WidgetPalette(&g_loadedFont->palette[1][0]);
		if (ImGui::TreeNode("Palette B2"))
			gui_WidgetPalette(&g_loadedFont->palette[1][1]);
	}
	ImGui::End();
}

// #### CALLBACKS ####
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	g_window.width = width;
	g_window.height = height;
}

void lockCursor()
{
	glfwGetCursorPos(g_window.window, &g_mouse.xPos, &g_mouse.yPos);
	glfwSetInputMode(g_window.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (g_mouse.rawMotionAvailible)
		glfwSetInputMode(g_window.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	g_window.hasCursorLock = true;
}

void unlockCursor()
{
	glfwSetInputMode(g_window.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	if (g_mouse.rawMotionAvailible)
		glfwSetInputMode(g_window.window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	g_window.hasCursorLock = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (g_mouse.firstData)
	{
		g_mouse.xPos = xpos;
		g_mouse.yPos = ypos;
		g_mouse.deltaX = 0.0f;
		g_mouse.deltaY = 0.0f;
		g_mouse.firstData = false;
	}

	g_mouse.deltaX = xpos - g_mouse.xPos;
	g_mouse.deltaY = g_mouse.yPos - ypos; // reversed since y-coordinates go from bottom to top

	g_mouse.xPos = xpos;
	g_mouse.yPos = ypos;

	if (ImGui::GetIO().WantCaptureMouse)
	{
		if (g_window.hasCursorLock) unlockCursor();
		return;
	}

	//if (g_window.hasCursorLock)
	//	g_camera.ProcessMouseMovement(g_mouse.deltaX, g_mouse.deltaY);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	/*if (g_window.hasCursorLock) g_camera.ProcessMouseScroll(yoffset);*/
}

void processInput(GLFWwindow* window)
{
	if (ImGui::GetIO().WantCaptureKeyboard)
	{
		if (g_window.hasCursorLock) unlockCursor();
		return;
	}

	if (!ImGui::GetIO().WantCaptureMouse
		&& glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
	{
		if (!g_window.hasCursorLock) lockCursor();
	}
	else if (DISABLE_MOUSELOCK && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
	{
		if (g_window.hasCursorLock) unlockCursor();
	}
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
	{
		if (g_window.hasCursorLock) unlockCursor();
	}

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	/*if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		g_camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		g_camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		g_camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		g_camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		g_camera.MovementMultiplier = 2.0f;
	else
		g_camera.MovementMultiplier = 1.0f;*/
}

bool Init()
{
	// ## INIT GLFW ##

	glfwInit();
	std::cout << "[GS] Compiled with GLFW " << glfwGetVersionString() << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//if (USE_ANTIALIASING) glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "KHBBSMap", NULL, NULL);
	g_window.window = window;
	if (window == NULL)
	{
		std::cout << "[GS] Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (glfwRawMouseMotionSupported()) g_mouse.rawMotionAvailible = true;

	// ## INIT OPENGL ##

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "[GS] Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return false;
	}

	/*if (GLAD_GL_EXT_texture_filter_anisotropic)
	{
		std::cout << "[GS] Loaded extention GL_EXT_texture_filter_anisotropic!" << std::endl;
		float max_anis;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anis);
		std::cout << "[GS] Max anisotropic samples: " << max_anis << std::endl;
	}*/

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	g_window.width = SCR_WIDTH;
	g_window.height = SCR_HEIGHT;

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "[GS] Renderer: " << renderer << std::endl;
	std::cout << "[GS] OpenGL version: " << version << std::endl;

	// uncomment this call to draw in wireframe polygons.
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (USE_ANTIALIASING) glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glDepthFunc(GL_LEQUAL);

	// ## INIT IMGUI ##

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(g_window.window, true);
	ImGui_ImplOpenGL3_Init();

	//std::string fontPath = fileManager.GetFontPath("Roboto-Medium.ttf");
	ImFont* font = io.Fonts->AddFontFromFileTTF("./Roboto-Medium.ttf", 13.0f);
	assert(font != nullptr);


	return true;
}

void Shutdown()
{
	std::cout << "[GS] Shutting down..." << std::endl;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_window.window);
	glfwTerminate();

	std::cout << "[GS] Goodbye!" << std::endl;
}

const char* fontTypes[6] = {
	"cmdfont",
	"helpfont",
	"menufont",
	"mesfont",
	"numeral",
	"fonticon"
};

const char* fontExts[4] = {
	"inf", "clu", "mtx", "cod"
};

Image* DecodeImage(uint texWidth, uint texHeight, uint defHeight, char* pix, uint pix_size, char* clut, uint clut_size)
{
	// This is wrong, pixels are stored as 4 bit values, not 8!
	
	uint imgWidth = texWidth * 2;
	uint imgHeight = texHeight;
	
	uint blockw = imgWidth / 32;
	uint blockh = defHeight / 16;

	assert(clut_size >= 16 * 4);
	assert(pix_size >= blockw * blockh * 256);
	
	Image* result = new Image;
	result->height = imgHeight;
	result->width = imgWidth;
	result->pixelSize = 4;
	result->pixelData = new char[imgWidth * imgHeight * 4];

	for (int bh = 0; bh < blockh; bh++)
	{
		for (int bw = 0; bw < blockw; bw++)
		{
			int blockstart = (bh * blockw + bw) * 256;
			char* blockBuffer = &(pix[blockstart]);
			int bpx = bw * 32;
			int bpy = bh * 16;
			for (int py = 0; py < 16; py++)
			{
				for (int px = 0; px < 32; px+=2)
				{
					int pp = ((bpy + py) * imgWidth + (bpx + px)) * 4;
					char index = blockBuffer[py * 32 + px];
					char idxA = index & 0b00001111;
					char idxB = (index & 0b11110000) >> 4;
					result->pixelData[pp + 0] = clut[idxA * 4 + 0];
					result->pixelData[pp + 1] = clut[idxA * 4 + 1];
					result->pixelData[pp + 2] = clut[idxA * 4 + 2];
					result->pixelData[pp + 3] = clut[idxA * 4 + 3];
					result->pixelData[pp + 4] = clut[idxB * 4 + 0];
					result->pixelData[pp + 5] = clut[idxB * 4 + 1];
					result->pixelData[pp + 6] = clut[idxB * 4 + 2];
					result->pixelData[pp + 7] = clut[idxB * 4 + 3];
				}
			}
		}
	}

	return result;
}

void DumpTex(int argc, char** argv)
{
	int fontIdx = -1;
	for (int id = 0; id < 6; id++)
	{
		if (strcmp(argv[2], fontTypes[id]) == 0)
		{
			fontIdx = id;
			break;
		}
	}

	if (fontIdx < 0)
	{
		std::cerr << "Unrecognised font!" << std::endl;
		return;
	}

	if (fontIdx == 5)
	{
		std::cerr << "fonticon not yet supported." << std::endl;
		return;
	}

	const char* base = fontTypes[fontIdx];
	std::string infPath = std::string(".\\").append(base).append(".inf");
	std::string cluPath = std::string(".\\").append(base).append(".clu");
	std::string mtxPath = std::string(".\\").append(base).append(".mtx");
	std::string codPath = std::string(".\\").append(base).append(".cod");

	LoadFont(fs::path(infPath), fs::path(cluPath), fs::path(mtxPath), fs::path(codPath), false);
	std::cout << "imgWidth " << g_loadedFont->info.imageWidth << " imgHeight " << g_loadedFont->info.imageHeight << " defHeight" << g_loadedFont->defTexImageHeight << std::endl;
	
	//Image* img = DecodeImage(g_loadedFont->info.imageWidth, g_loadedFont->info.imageHeight, g_loadedFont->defTexImageHeight, g_loadedFont->pTexture, g_loadedFont->pTexture_size, (char*)(g_loadedFont->palette[0][0].RGBA), 16 * 4);
	Image* img = DecodeImage(256, 32, 32, g_loadedFont->pTexture, g_loadedFont->pTexture_size, (char*)(g_loadedFont->palette[0][0].RGBA), 16 * 4);
	stbi_write_png("out0.png", img->width, img->height, 4, img->pixelData, 0);
	delete[] img->pixelData;

	//img = DecodeImage(g_loadedFont->info.imageWidth, g_loadedFont->info.imageHeight, g_loadedFont->defTexImageHeight, g_loadedFont->pTexture, g_loadedFont->pTexture_size, (char*)(g_loadedFont->palette[0][1].RGBA), 16 * 4);
	img = DecodeImage(256, 128, 128, g_loadedFont->pTexture, g_loadedFont->pTexture_size, (char*)(g_loadedFont->palette[0][1].RGBA), 16 * 4);
	stbi_write_png("out1.png", img->width, img->height, 4, img->pixelData, 0);
	delete[] img->pixelData;

	/*img = DecodeImage(g_loadedFont->info.imageWidth, g_loadedFont->info.imageHeight, g_loadedFont->defTexImageHeight, g_loadedFont->pTexture, g_loadedFont->pTexture_size, (char*)(g_loadedFont->palette[1][0].RGBA), 16 * 4);
	stbi_write_png("out2.png", img->width, img->height, 4, img->pixelData, 0);
	delete[] img->pixelData;

	img = DecodeImage(g_loadedFont->info.imageWidth, g_loadedFont->info.imageHeight, g_loadedFont->defTexImageHeight, g_loadedFont->pTexture, g_loadedFont->pTexture_size, (char*)(g_loadedFont->palette[1][1].RGBA), 16 * 4);
	stbi_write_png("out3.png", img->width, img->height, 4, img->pixelData, 0);
	delete[] img->pixelData;*/

	delete img;

}

int main(int argc, char** argv)
{
	if (strcmp(argv[1], "dumptex") == 0)
	{
		DumpTex(argc, argv);
		return 0;
	}

	/*if (!Init())
		return -1;
	atexit(Shutdown);

	lastFrame = glfwGetTime();

	while (!glfwWindowShouldClose(g_window.window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		if (deltaTime > MAX_DELTA_TIME) deltaTime = MAX_DELTA_TIME;
		lastFrame = currentFrame;
		g_worldTime += deltaTime;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		gui_FontInfo();
		gui_CodePoints();
		gui_Palettes();
		gui_TextureSettings();
		gui_TextureView();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_window.window);
		glfwPollEvents();
	}*/
}