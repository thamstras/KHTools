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
#include <cstring>
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

class MemoryStream
{
private:
	uint8_t* data;
	uint32_t data_size;
	uint32_t read_ptr;
public:
	MemoryStream(uint8_t* data, uint32_t size) : data(data), data_size(size), read_ptr(0) {}
	~MemoryStream() {}

	uint32_t Length() { return data_size; }

	uint32_t GetPos() { return read_ptr; }
	void SeekAbs(uint32_t pos) { read_ptr = pos; if (read_ptr < 0) read_ptr = 0; else if (read_ptr > data_size) read_ptr = data_size; }
	void SeekRel(int32_t off) { read_ptr += off; if (read_ptr < 0) read_ptr = 0; else if (read_ptr > data_size) read_ptr = data_size; }
	bool EoF() { return (read_ptr >= data_size); }

	uint8_t* Raw() { return data; }

	uint32_t Read(uint8_t* out, uint32_t offset, uint32_t size)
	{
		if (read_ptr >= data_size) return 0;

		uint32_t writePtr = 0;
		for (writePtr; writePtr < size; writePtr++)
		{
			out[writePtr] = data[read_ptr];
			read_ptr++; writePtr++;
			if (read_ptr >= data_size) break;
		}
		return writePtr;
	}
};

uint32_t LEu8tou32(uint8_t* in)
{
	uint32_t out = 0;
	out |= in[0] << 0;
	out |= in[1] << 8;
	out |= in[2] << 16;
	out |= in[3] << 24;
	return out;
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

struct Image
{
	uint width;			// Image width in pixels
	uint height;		// Image height in pixels
	ubyte pixelSize;	// Pixel size in bytes. Pretty much always 4 (RGBA8888) or 3 (RGB888)
	char* pixelData;	// width * height * pixelSize bytes of data
};

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

ImVec4 ConvertColor(uint col32)
{
	// Implicit conversion
	return ImColor(col32);
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

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	
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

}

bool Init()
{
	// ## INIT GLFW ##

	glfwInit();
	std::cout << "[GS] Compiled with GLFW " << glfwGetVersionString() << std::endl;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	g_window.width = SCR_WIDTH;
	g_window.height = SCR_HEIGHT;

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

bool Decompress(uint8_t* inBuf, uint32_t inBuf_size, uint8_t** out_data, uint32_t* out_size)
{
	if (inBuf == nullptr || inBuf_size < 8) return false;

	MemoryStream data = MemoryStream(inBuf, inBuf_size);

	// Size of compressed data is appended to end
	uint8_t buffer[4]{};
	data.SeekAbs(data.Length() - 4);
	data.Read(buffer, 0, 4);
	uint32_t extraSize = LEu8tou32(buffer);
	if (extraSize == 0)
	{
		// No compression, copy in to out, minus last 4 bytes
		data.SeekAbs(0);
		*out_size = inBuf_size - 4;
		*out_data = new uint8_t[*out_size];
		data.Read(*out_data, 0, *out_size);
		return true;
	}
	else
	{
		std::vector<uint8_t> output = std::vector<uint8_t>();

		data.SeekRel(-5);
		uint8_t headerSize;
		data.Read(&headerSize, 0, 1);
		data.SeekRel(-4);
		data.Read(buffer, 0, 4);
		uint32_t compressedSize = LEu8tou32(buffer);
		
		// reset stream position
		data.SeekAbs(0);

		// read uncompressed data from start
		uint32_t uncompressedSize = inBuf_size - compressedSize;
		if (uncompressedSize)
		{
			output.reserve(output.capacity() + uncompressedSize);
			data.Read(output.data(), 0, uncompressedSize);
		}

		compressedSize -= headerSize;

		// buffer compressed data so we're not seeking all the time
		uint8_t* compressionBuffer = new uint8_t[compressedSize];
		data.Read(compressionBuffer, 0, compressedSize);

		data.SeekRel(headerSize);

		uint8_t* outBuf = new uint8_t[compressedSize + headerSize + extraSize];
		uint32_t outBuf_length = compressedSize + headerSize + extraSize;

		int currentOutSize = 0;
		int decompressedLength = outBuf_length;
		int readBytes = 0;
		uint8_t flags = 0, mask = 1;
		while (currentOutSize < decompressedLength)
		{
			if (mask == 1)
			{
				if (readBytes >= compressedSize)
				{
					// TODO: Error
				}
				flags = compressionBuffer[compressedSize - 1 - readBytes];
				readBytes++;
				mask = 0x80;
			}
			else
			{
				mask >>= 1;
			}

			if ((flags & mask) > 0)
			{
				if (readBytes + 1 >= compressedSize)
				{
					// TODO: Error
				}

				uint8_t byte1 = compressionBuffer[compressedSize - 1 - readBytes];
				readBytes++;
				uint8_t byte2 = compressionBuffer[compressedSize - 1 - readBytes];
				readBytes++;

				int length = byte1 >> 4;
				length += 3;

				int disp = ((byte1 & 0x0F) << 8) | byte2;
				disp += 3;

				if (disp > currentOutSize)
				{
					if (currentOutSize < 2)
					{
						// TODO: Error
					}

					disp = 2;
				}

				int bufIdx = currentOutSize - disp;
				for (int i = 0; i < length; i++)
				{
					uint8_t next = outBuf[outBuf_length - 1 - bufIdx];
					bufIdx++;
					outBuf[outBuf_length - 1 - currentOutSize] = next;
					currentOutSize++;
				}
			}
			else
			{
				if (readBytes >= compressedSize)
				{
					// TODO: Error
				}

				uint8_t next = compressionBuffer[compressedSize - 1 - readBytes];
				readBytes++;
				outBuf[outBuf_length - 1 - currentOutSize] = next;
				currentOutSize++;
			}
		}

		for (int i = 0; i < outBuf_length; i++) output.push_back(outBuf[i]);

		delete[] outBuf;
		delete[] compressionBuffer;

		*out_data = new uint8_t[output.size()];
		for (int i = 0; i < output.size(); i++) (*out_data)[i] = output[i];

		*out_size = decompressedLength + (inBuf_size - headerSize - compressedSize);

		return true;
	}

	return false;
}

/*Image* DecodeImage(uint texWidth, uint texHeight, uint defHeight, char* pix, uint pix_size, char* clut, uint clut_size)
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
}*/

void DumpTex(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cerr << "No file provided" << std::endl;
		return;
	}
	FILE* inFile;
	if (fopen_s(&inFile, argv[2], "rb") != 0)
	{
		std::cerr << "Couldn't open file: " << strerror(errno) << std::endl;
		return;
	}
	if (inFile == 0) return;	// This can't happen but this line needs to be here to stop VS complaining.
	fseek(inFile, 0, SEEK_END);
	long fsize = ftell(inFile);
	rewind(inFile);
	uint8_t* filebuffer = new uint8_t[fsize];
	if (filebuffer == nullptr)
	{
		std::cerr << "Error allocating map memory" << std::endl;
		fclose(inFile);
		return;
	}
	size_t readSize = fread(filebuffer, 1, fsize, inFile);
	if (readSize != fsize)
	{
		std::cerr << "Read error" << std::endl;
		delete[] filebuffer;
		fclose(inFile);
		return;
	}

	fclose(inFile);

	uint8_t* output = nullptr;
	uint32_t output_size = 0;

	if (Decompress((uint8_t*)filebuffer, fsize, &output, &output_size))
	{
		FILE* outfile;
		if (fopen_s(&outfile, ".\output.bin", "wb") != 0)
		{
			std::cerr << "Couldn't open output file: " << strerror(errno) << std::endl;
			delete[] filebuffer;
			delete[] output;
			return;
		}
		if (outfile == 0) { delete[] filebuffer; delete[] output; return; }	// Again, can't happen, VS moaning.
		fwrite(output, 1, output_size, outfile);
		fclose(outfile);
		delete[] output;
	}

	delete[] filebuffer;

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