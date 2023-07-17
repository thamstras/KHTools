#pragma once

enum OPT_TYPE
{
    OPT_INT,
    OPT_CHAR,
    OPT_STRING
};

struct CLI_OPT
{
    const char* short_opt;
    const char* long_opt;
    OPT_TYPE type;
    void* opt_dest;
};

void ParseSingleOpt(CLI_OPT* opt, char* arg);
void ParseOpts(CLI_OPT* optDefs, int count_optDefs, int argc, char** argv);