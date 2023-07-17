#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <set>

#include "BTypes.h"
#include "FileTypes/BbsPmo.h"

namespace fs = std::filesystem;

std::set<uint8> g_headerUnk04_0;
std::set<uint8> g_headerUnk04_1;
std::set<uint8> g_headerUnk04_2;
std::set<uint8> g_headerUnk04_3;
std::set<uint16> g_headerUnk0A;
std::set<uint32> g_meshFormatunk;
std::set<uint8> g_meshUnk8;
std::set<uint16> g_meshUnkA;

void CheckPmoHeader(fs::path path, PmoHeader& header)
{
	//if (header.unk_04 != 0)
		g_headerUnk04_0.insert(header.unk_04[0]);
		g_headerUnk04_1.insert(header.unk_04[1]);
		g_headerUnk04_2.insert(header.unk_04[2]);
		g_headerUnk04_3.insert(header.unk_04[3]);

	//if (!header.unk_0A != 0)
		//std::cout << "PMO " << path << " header.unk0A is " << std::hex << header.unk_0A << std::endl;
		g_headerUnk0A.insert(header.unk_0A);
}

void CheckPmoMesh(fs::path path, int meshId, int secId, PmoMesh& mesh)
{
	if (mesh.header.vertexFormat.unused_b13 != 0)
		std::cout << "PMO " << path << " mesh" << std::dec << meshId << ":" << secId << " vertexFormat.unused_b13 is USED!" << std::endl;

	if (mesh.header.vertexFormat.unused_b17 != 0)
		std::cout << "PMO " << path << " mesh" << std::dec << meshId << ":" << secId << " vertexFormat.unused_b17 is USED!" << std::endl;

	if (mesh.header.vertexFormat.unused_b21_22 != 0)
		std::cout << "PMO " << path << " mesh" << std::dec << meshId << ":" << secId << " vertexFormat.unused_b21_22 is USED!" << std::endl;

	//if (mesh.header.vertexFormat.unk != 0)
		//std::cout << "PMO " << path << " mesh" << std::dec << meshId << ":" << secId << " vertexFormat.unk HAS VALUE " << mesh.header.vertexFormat.unk << std::endl;
		g_meshFormatunk.insert(mesh.header.vertexFormat.unk);

	//if (mesh.header.unk_08 != 0)
		g_meshUnk8.insert(mesh.header.unk_08);

	//if (mesh.header.unknown0A != 0)
		g_meshUnkA.insert(mesh.header.unknown0A);
}

template <typename T1>
void OutputSet(std::set<T1> set, const char* name)
{
	if (set.size() == 0) return;
	
	int col = 0;
	
	std::cout << name << " took the following values : " << std::hex << std::endl;
	for (auto val : set)
	{
		std::cout << "\t" << val;
		if (++col > 8)
		{
			col = 0;
			std::cout << std::endl;
		}
	}

	if (col != 0)
	{
		std::cout << std::endl;
		col = 0;
	}

	std::cout << std::endl;
}

template <>
void OutputSet<uint8_t>(std::set<uint8_t> set, const char* name)
{
	if (set.size() == 0) return;

	int col = 0;

	std::cout << name << " took the following values : " << std::hex << std::endl;
	for (auto val : set)
	{
		std::cout << "\t" << (uint16_t)val;
		if (++col > 8)
		{
			col = 0;
			std::cout << std::endl;
		}
	}

	if (col != 0)
	{
		std::cout << std::endl;
		col = 0;
	}

	std::cout << std::endl;
}

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		std::cerr << "Incorrect argument count." << std::endl;
		return -1;
	}

	fs::path basePath = fs::path(argv[1]);

	if (!fs::is_directory(basePath))
	{
		std::cerr << "Input is not directory." << std::endl;
		return -1;
	}

	std::cout << std::showbase << std::uppercase;

	g_headerUnk04_0 = std::set<uint8>();
	g_headerUnk04_1 = std::set<uint8>();
	g_headerUnk04_2 = std::set<uint8>();
	g_headerUnk04_3 = std::set<uint8>();
	g_headerUnk0A = std::set<uint16>();
	g_meshFormatunk = std::set<uint32>();
	g_meshUnk8 = std::set<uint8>();
	g_meshUnkA = std::set<uint16>();

	std::cout << "Iteration tree root: " << basePath << std::endl;
	
	for (auto& p : fs::recursive_directory_iterator(basePath))
	{
		if (fs::is_directory(p)) continue;
		std::wstring ext = std::wstring(p.path().extension().c_str());
		if (ext.compare(L".pmo") == 0)
		{
			std::ifstream stream = std::ifstream();
			stream.open(p.path().c_str(), std::ios_base::binary|std::ios_base::in);
			if (!stream.is_open())
			{
				std::cerr << "Failed to open file " << strerror(errno) << std::endl;
				continue;
			}
			
			try
			{
				PmoFile pmo = PmoFile::ReadPmoFile(stream);
			
				CheckPmoHeader(p.path(), pmo.header);

				if (pmo.hasMesh0())
				{
					int secId = 0;
					for (auto& mesh : pmo.mesh0)
					{
						CheckPmoMesh(p.path(), 0, secId++, mesh);
					}
				}

				if (pmo.hasMesh1())
				{
					int secId = 0;
					for (auto& mesh : pmo.mesh1)
					{
						CheckPmoMesh(p.path(), 1, secId++, mesh);
					}
				}
			}
			catch (std::runtime_error ex)
			{
				std::cerr << "Runtime exception reading " << p.path() << ": " << ex.what() << std::endl;
				continue;
			}
			catch (std::invalid_argument ex)
			{
				std::cerr << "DUMDUM ALERT: invalid_argument " << ex.what() << std::endl;
				continue;
			}
			catch (std::out_of_range ex)
			{
				std::cerr << "DUMDUM ALERT: out_of_range" << ex.what() << std::endl;
				continue;
			}
			catch (std::exception ex)
			{
				std::cerr << "Unknown exception handling " << p.path() << ex.what() << std::endl;
				continue;
			}

		}
	}

	std::cout << std::endl << "-------- SEARCH DONE --------" << std::endl << std::endl;
	
	int col = 0;

	OutputSet(g_headerUnk04_0, "PMO header.unknown_04[0] (uint8) took the following values:");
	OutputSet(g_headerUnk04_1, "PMO header.unknown_04[1] (uint8) took the following values:");
	OutputSet(g_headerUnk04_2, "PMO header.unknown_04[2] (uint8) took the following values:");
	OutputSet(g_headerUnk04_3, "PMO header.unknown_04[3] (uint8) took the following values:");

	OutputSet(g_headerUnk0A, "PMO header.unknown_0A (uint16) took the following values:");
	
	OutputSet(g_meshFormatunk, "PMO mesh.vertexFormat.unk (uint32:3) took the following values:");

	OutputSet(g_meshUnk8, "PMO mesh.unknown08 (uint8) took the following values:");

	OutputSet(g_meshUnkA, "PMO mesh.unknown0A (uint16) took the following values:");

	std::cout << "---------- END OF REPORT ----------" << std::endl;
}