#include <string>
#include <iostream>
#include <chrono>
//#include "NSWFL_CRC32.H"

#include <stdint.h>
struct crc32
{
	static void generate_table(uint32_t(&table)[256])
	{
		uint32_t polynomial = 0xEDB88320;
		for (uint32_t i = 0; i < 256; i++)
		{
			uint32_t c = i;
			for (size_t j = 0; j < 8; j++)
			{
				if (c & 1) {
					c = polynomial ^ (c >> 1);
				}
				else {
					c >>= 1;
				}
			}
			table[i] = c;
		}
	}

	static uint32_t update(uint32_t(&table)[256], uint32_t initial, const void* buf, size_t len)
	{
		uint32_t c = initial ^ 0xFFFFFFFF;
		const uint8_t* u = static_cast<const uint8_t*>(buf);
		for (size_t i = 0; i < len; ++i)
		{
			c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
		}
		return c ^ 0xFFFFFFFF;
	}
};

const char* worlds[] = {
    "EX", "DP", "SW", "CD",
    "SB", "YT", "RG", "JB",
    "HE", "LS", "DI", "PP",
    "DC", "KG", "14", "VS",
    "BD", "WM", "WP", "19",
    "20", "21", "22", "JF"
};

/*int findDir()
{
    const char* baseDir = "MISSION";
    char stringBuffer[255] = { 0 };
    uint32_t target = 0xEAA87E91;
    //uint32_t target = 0x25CA4906;

    uint32_t table[256];
    crc32::generate_table(table);

    for (int w = 0; w < 24; w++)
    {
        for (int i = 0; i < 1000; i++)
        {
            sprintf_s(stringBuffer, "%s/%s/MS%d", baseDir, worlds[w], i);
            size_t pathlen = strlen(stringBuffer);
            //unsigned int hash = crc32.FullCRC((unsigned char*)stringBuffer, pathlen);
            uint32_t hash = crc32::update(table, 0, stringBuffer, pathlen);
            if (hash == target)
            {
                std::cout << "FOUND IT: " << stringBuffer << std::endl;
                return 0;
            }
        }
        std::cout << "IT'S NOT " << worlds[w] << std::endl;
    }
    std::cout << "IT'S NOT MISSION/<WORLD>/MS<NUM>" << std::endl;
    return -1;
}*/

struct target
{
    uint32_t value;
    bool found;
};

target targets[] = {
    {0x37E33627, false},
    {0x40E406B1, false},
    {0x88EEC142, true},    //C_HELP_T
    {0x308EF23E, false},
    {0x4789C2A8, false},
    {0xA987A384, false},
    {0xBDCE12E7, false},
    {0xC503560F, false},
    {0xD9ED570B, false},
    {0xDE809312, false},
    {0xD7E23C9B, false}, // Expected: N01CDPV
    {0x8C1B0652, false}, // Expected: CFACE_13
    {0x7CBD0EB3, false}, // Expected: N07DC02PV
};

const char* iPatterns[] = {
    "%02d_RACE",
    "%02d_ICE",
    "%02d_RESULT",
    "%02d_TUBO",
    "A_MENU_%02d",
    "CFACE_%02d",
    "CS_%02d",
    "D_LINK_%02d",
    "D_LINK_%02d_B",
    "D_LINK_%02d_F",
    "D_LINK_%02d_G",
    "INFWIND%d",
    "P%02dCAMP",
    "REPORT%02d",
    "REPORT%02dST",
    "REPORT%02d2D",
    "STORYAQU%02d",
    "STORYTER%02d",
    "STORYVEN%02d",
    "GAGE_%02d",
    "P%02dBDPV",
    "VOLTAGE_%02d",
    "TITLE_%02d",
    "MISSION_%02d",
    "A_COCKPIT_%02d",
    "SLOT_%02d",
    "FB_F%2d",
    "FB_F%02d",
    "FB_%02d",
    "FB_%02dF",
    "FBF%2d",
    "FBF%02d",
    "FB%02d",
    "FB%02dF",
};

const int n_iPatterns = 35;

const char* iwPatterns[] = {
    "B%02d%sPV",
    "M%02d%sPV",
    "N%02d%sPV",
};

const int n_iwPatterns = 3;

const char* iwjPatterns[] = {
    "B%02d%s%02dPV",
    "M%02d%s%02dPV",
    "N%02d%s%02dPV",
};

const int n_iwjPatterns = 3;

const char* ijPatterns[] = {
    "HELP%02d%02d"
};

const int n_ijPatterns = 1;

void check(uint32_t hash, char * stringBuffer)
{
    for each (target & t in targets)
    {
        if (t.found) continue;
        if (hash == t.value)
        {
            std::cout << "FOUND IT: " << stringBuffer << std::endl;
            t.found = true;
            break;
        }
    }
}

int findFile()
{
    char stringBuffer[255] = { 0 };
    //uint32_t target = 0xAFCCAB6F;

    uint32_t table[256];
    crc32::generate_table(table);
    std::cout << "STRAT 1" << std::endl;
    
    for (int i = 0; i < 100; i++)
    {
        for (int p = 0; p < n_iPatterns; p++)
        {
            sprintf_s(stringBuffer, iPatterns[p], i);
            size_t namelen = strlen(stringBuffer);
            uint32_t hash = crc32::update(table, 0, stringBuffer, namelen);
            check(hash, stringBuffer);
        }

        for (int w = 0; w < 24; w++)
        {
            for (int p = 0; p < n_iwPatterns; p++)
            {
                sprintf_s(stringBuffer, iwPatterns[p], i, worlds[w]);
                size_t namelen = strlen(stringBuffer);
                uint32_t hash = crc32::update(table, 0, stringBuffer, namelen);
                check(hash, stringBuffer);
            }

            for (int j = 0; j < 100; j++)
            {
                for (int p = 0; p < n_iwjPatterns; p++)
                {
                    sprintf_s(stringBuffer, iwjPatterns[p], i, worlds[w], j);
                    size_t namelen = strlen(stringBuffer);
                    uint32_t hash = crc32::update(table, 0, stringBuffer, namelen);
                    check(hash, stringBuffer);
                }
            }
        }
        std::cout << i << "%" << std::endl;
    }

    std::cout << "NAME'S NOT NICE!" << std::endl;
    return -1;
}

struct SoundTask
{
    char* worldCode;
    uint32_t targetHash;
};

SoundTask tasks[] = {
    //{"DP", 0x1174F201},

    {"PP", 0x08EBCA66},
    {"CD", 0x14289B42},
    {"CD", 0x1CA25B48},
    {"DP", 0x1DCC3BA5},
    {"SB", 0x22FB4C08},
    {"PP", 0x2DEA0975},
    {"CD", 0x320BF8E5},
    {"PP", 0x63B2ACD5},
    {"KG", 0x82EC817A},
    {"PP", 0x84C1614A},
    {"PP", 0x8A9B3E1F},
    {"DP", 0x9133EA0E},
    {"HE", 0x94A85E7A},
    {"PP", 0x953CF2DF},
    {"SB", 0xAAC07A9E},
    {"DP", 0xAE423E84},
    {"CD", 0xB2BB72AA},
    {"PP", 0xC0899711},
    {"JB", 0xDFE50C61},
    {"KG", 0xE00F8A70},
    {"DP", 0xEF0999EE}
};

int taskCount = 21;

char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void DoTask(SoundTask task)
{
    uint32_t table[256];
    crc32::generate_table(table);
    char stringBuffer[255] = { 0 };

    for (int i = 0; i < 100000; i++)
    {
        for (int j = 0; j < (26 * 26); j++)
        {
            char ca = alphabet[j / 26];
            char cb = alphabet[j % 26];
            for (int k = 0; k < 10; k++)
            {
                sprintf_s(stringBuffer, "1%s%05d%c%c%01d", task.worldCode, i, ca, cb, k);
                size_t nameLen = strlen(stringBuffer);
                uint32_t hash = crc32::update(table, 0, stringBuffer, nameLen);
                if (hash == task.targetHash)
                {
                    std::cout << std::hex << "FOUND " << task.targetHash << " " << stringBuffer << std::endl;
                    return;
                }
            }
        }
        if (i % 1000 == 0) std::cout << ".";
    }
    std::cout << "DIDN'T FIND " << std::hex << task.targetHash << std::endl;
}

void DoTasks()
{
    auto start = std::chrono::high_resolution_clock::now();
    
    // These times are *very* rough guesses.
    std::cout << "Stating " << taskCount << " tasks. Estimated Time: " << (120 * taskCount) / 60 << " minutes. Worst case: " << (250 * taskCount) / 60 << " minutes." << std::endl;

    uint32_t table[256];
    crc32::generate_table(table);
    char stringBuffer[255] = { 0 };

    for (int t = 0; t < taskCount; t++)
    {
        SoundTask task = tasks[t];
        DoTask(task);
    }

    auto end = std::chrono::high_resolution_clock::now();

    auto dur = end - start;

    std::cout << "All tasks finished in " << std::dec << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << " milliseconds" << std::endl;
}

/*const char* bgmNames[] = {
    "001SINDE_F",
    "002SINDE_B",
    "003NEMURE_F",
    "004NEMURE_B",
    "005SYUGYO_F",
    "006SYUGYO_B",
    "007SHIRA_F",
    "008SHIRA_B",
    "009RAYDI_F",
    "010RAYDI_B",
    "011DISTOW_F",
    "012DISTOW_B",
    "013NEVER_F",
    "014NEVER_B",
    "015HERC_F",
    "016HERC_B",
    "017RIRO_F",
    "018RIRO_B",
    "019IENSID_F",
    "020IENSID_B",
    "021KOUYA_F",
    "022DISICE_F",
    "023TSUSIN_F",
    "030YOUKI",
    "031ISAMASHI",
    "032ODAYAKA",
    "033FUON",
    "034KANASII",
    "035KINPAKU",
    "036SEISIN",
    "037YAMI",
    "038RIA_DEAI",
    "040ANBA_B1",
    "041ANBA_B2",
    "042DIS_B1",
    "043DIS_B2",
    "044VANITA_B",
    "045ANTHEM_B",
    "046LAST_B1",
    "047LAST_B2",
    "048HANYO_B1",
    "049HANYO_B2",
    "050TITLE",
    "051WORLDMAP",
    "060TERA",
    "061AQUA",
    "062VEN",
    "063KAIRI1",
    "064KAIRI3",
    "065RIKU",
    "066PEET",
    "067DISVILL",
    "068ZEA",
    "069BRAIG",
    "070KEY_L",
    "071KEY_D",
    "072KEY_L_D",
    "073KIZUNA",
    "074ZACK",
    "100ICE1_128",
    "101ICE1_132",
    "102ICE_2",
    "103FRUIT",
    "104DICE",
    "105POOMINI",
    "106CARTRACE",
    "107SYUGYO",
    "108RIRO",
    "109TRAINING",
    "110HAN_BT1",
    "111HAN_BT2",
    "112RAGE_BT",
    "113KH1TIT",
    "114RACEVIEW",
    "115BOSS",
    "116ICON",
    "117SHORT_L2",
    "118GUMI",
    "119DESTI",
    "120HAND",
    "121PICTH",
    "122NAZONO",
    "123REV",
    "124DP_AMB",
    "200SE_MORI",
};

const int bgm_count = 85;

void BGMTask()
{
    //char stringBuffer[255] = { 0 };
    uint32_t target = 0xD69C56EE;

    uint32_t table[256];
    crc32::generate_table(table);

    for (int i = 0; i < bgm_count; i++)
    {
        const char* name = bgmNames[i];
        size_t namelen = strlen(name);
        uint32_t hash = crc32::update(table, 0, name, namelen);
        if (hash == target)
        {
            std::cout << std::hex << "FOUND " << target << " " << name << std::endl;
            return;
        }
    }
}*/

int main(int argc, char** argv)
{
    //findDir();
    //findFile();
    DoTasks();
    //BGMTask();
}