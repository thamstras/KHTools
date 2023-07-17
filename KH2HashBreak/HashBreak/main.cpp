#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::string;

typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;

typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

struct TestDef
{
	string realName;
	uint32 hash32;
	uint16 hash16;
};

struct TargetDef
{
	uint32 hash32;
	uint16 hash16;
};

const char g_charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/";
const int g_charset_len = sizeof(g_charset) / sizeof(char);

uint32 hash32(uint8* data, size_t length)
{
	int32 c = -1;
	for (int i = 0; i < length; i++)
	{
		c ^= data[i] << 24;
		for (int j = 8; j > 0; j--)
		{
			if (c < 0)
				c = (c << 1) ^ 0x4C11DB7;
			else
				c <<= 1;
		}
	}

	return (uint32)~c;
}

uint16 hash16(uint8* data, size_t length)
{
	int32 s1 = -1;
	for (int i = length - 1; i >= 0; i--)
	{
		s1 = (s1 ^ (data[i] << 8)) & 0xFFFF;
		for (int j = 8; j > 0; j--)
		{
			if ((s1 & 0x8000) != 0)
				s1 = ((s1 << 1) ^ 0x1021) & 0xFFFF;
			else
				s1 <<= 1;
		}
	}
	return (uint16)~s1;
}

bool CheckGuess(const TargetDef& target, string& guess)
{
	uint8* data = (uint8*)guess.data();
	size_t len = guess.length();

	if (hash32(data, len) == target.hash32
		&& hash16(data, len) == target.hash16)
	{
		std::cout << "TARGET FOUND: " << guess << endl;
		return true;
	}
	return false;
}

bool Recurse(const char* charset, const int charset_len, string& prefix, int n, const string& suffix, const TargetDef& target)
{
	if (n == 0)
	{
		string guess = prefix + suffix;
		return CheckGuess(target, guess);
	}

	for (int i = 0; i < charset_len; i++)
	{
		string newPrefix = prefix + charset[i];
		if (Recurse(charset, charset_len, newPrefix, n - 1, suffix, target))
			return true;
	}

	return false;
}


bool MakeGuesses(int length, const string& prefix, const string& suffix, const TargetDef& target)
{
	string starting = prefix;
	return Recurse(g_charset, g_charset_len, starting, length, suffix, target);
}

std::atomic<int> nWorkersComplete;

struct WorkerThread
{
	// problem vars
	string prefix;
	string suffix;
	int remainingLength;
	TargetDef target;

	bool found;

	// thread management
	bool alive = true;
	std::condition_variable cvStart;
	std::mutex mux;
	std::thread thread;

	void Start(const string& prefix, const string& suffix, int remainingLength, TargetDef target)
	{
		this->prefix = prefix;
		this->suffix = suffix;
		this->remainingLength = remainingLength;
		this->target;
		this->found = false;

		std::unique_lock<std::mutex> lm(mux);
		cvStart.notify_one();
	}

	void DoWork()
	{
		while (alive)
		{
			// Wait for main thread to tell us to start
			std::unique_lock<std::mutex> lm(mux);
			cvStart.wait(lm);

			found = Recurse(g_charset, g_charset_len, prefix, remainingLength, suffix, target);

			nWorkersComplete++;
		}
	}

};

WorkerThread workers[64];

void InitThreadPool()
{
	for (int i = 0; i < 64; i++)
	{
		workers[i].alive = true;
		workers[i].thread = std::thread(&WorkerThread::DoWork, &workers[i]);
	}
}

void DestroyThreadPool()
{
	for (int i = 0; i < 64; i++)
	{
		workers[i].alive = false;
		std::unique_lock<std::mutex> lm(workers[i].mux);
		workers[i].cvStart.notify_one();
	}

	for (int i = 0; i < 64; i++)
		workers[i].thread.join();
}

void PrintProgress(int prog, int total)
{
	cout << "[";
	int p;
	for (p = 0; p < prog; p++) cout << "#";
	for (; p < total; p++) cout << ".";
	cout << "]" << "\r";
}

bool MultithreadGuess(int length, const string& prefix, const string& suffix, const TargetDef& target)
{
	nWorkersComplete = 0;
	
	for (int i = 0; i < 64; i++)
	{
		string tPrefix = prefix + g_charset[i];
		workers[i].Start(tPrefix, suffix, length - 1, target);
	}

	int lastProgress = -1;
	while (nWorkersComplete < 64)
	{
		if (nWorkersComplete > lastProgress)
		{
			lastProgress = nWorkersComplete;
			PrintProgress(nWorkersComplete, 64);
		}
	}

	for (int i = 0; i < 64; i++)
	{
		if (workers[i].found) return true;
	}
	return false;
}

int main(int argc, char** argv)
{


	/*string guess = "anm/GAMEOVER.anb";
	if (hash32((uint8*)guess.data(), guess.length()) == 0x0A11C560 && hash16((uint8*)guess.data(), guess.length()) == 0x9A01)
	{
		cout << "FOUND IT!" << std::endl;
	}
	else
	{
		cout << "GUESS AGAIN..." << std::endl;
	}*/

	//string prefix = "anm/GAME";
	string prefix = "anm/";
	string suffix = ".anb";
	TargetDef target = { 0x0A11C560 , 0x9A01 };
	int start = 2;

	for (int a = 0; a < argc; a++)
	{
		char* arg = argv[a];
		if (strcmp(arg, "-n") == 0)
		{
			a++;
			if (a < argc)
			{
				start = atoi(argv[a]);
			}
			else
			{
				cout << "No argument supplied for -n!" << std::endl;
				return -1;
			}
		}
		else if (strcmp(arg, "-p") == 0)
		{
			a++;
			if (a < argc)
			{
				prefix = string(argv[a]);
			}
			else
			{
				cout << "No argument supplied for -p!" << std::endl;
				return -1;
			}
		}
		else if (strcmp(arg, "-s") == 0)
		{
			a++;
			if (a < argc)
			{
				suffix = string(argv[a]);
			}
			else
			{
				cout << "No argument supplied for -s!" << std::endl;
				return -1;
			}
		}
		else if (strcmp(arg, "-h") == 0)
		{
			cout << "Not written help yet, sorry." << endl;
			return 0;
		}
	}

	InitThreadPool();

	cout << std::noshowbase << std::hex;
	cout << "Prefix: " << prefix << " Suffix: " << suffix << " Target: " << target.hash32 << "_" << target.hash16 << endl;
	cout << "Starting at length " << start << endl;

	for (int c = start; c <= 32; c++)
	{
		//bool res = MakeGuesses(c, prefix, suffix, target);
		bool res = MultithreadGuess(c, prefix, suffix, target);
		if (res) break;
		else std::cout << "Length " << c << " checked.                                        " << endl;
	}

	DestroyThreadPool();

	/*string prefix = "anm/ex/h_ex500/EEX0000";
	string suffix = ".anb";
	TargetDef target = { 0x0002568E, 0xEE61 };
	cout << std::noshowbase << std::hex;
	cout << "Prefix: " << prefix << " Suffix: " << suffix << " Target: " << target.hash32 << "_" << target.hash16 << endl;
	for (int c = 1; c <= 5; c++)
	{
		bool res = MakeGuesses(c, prefix, suffix, target);
		if (res) break;
		else std::cout << c << " checked." << endl;
	}*/

	return 0;
}