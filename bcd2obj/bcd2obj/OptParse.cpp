#include "OptParse.h"

#include <string>
#include <iostream>

void ParseSingleOpt(CLI_OPT* opt, char* arg)
{
    switch (opt->type)
    {
    case OPT_INT:
    {
        int* dest = (int*)opt->opt_dest;
        *dest = std::stoi(std::string(arg));
        std::cout << "Parsed opt " << opt->long_opt << " as " << *dest << std::endl;
    }
    break;
    case OPT_CHAR:
    {
        char* dest = (char*)opt->opt_dest;
        *dest = *arg;
        std::cout << "Parsed opt " << opt->long_opt << " as " << *dest << std::endl;
    }
    break;
    case OPT_STRING:
    {
        char** dest = (char**)opt->opt_dest;
        *dest = arg;
        std::cout << "Parsed opt " << opt->long_opt << " as " << *dest << std::endl;
    }
    break;
    default:
        break;
    }
}

void ParseOpts(CLI_OPT* optDefs, int count_optDefs, int argc, char** argv)
{
    for (int a = 0; a < argc; a++)
    {
        char* arg = argv[a];
        if (arg[0] == '-')
        {
            if (arg[1] == '-')  // Long opt
            {
                arg += 2;
                for (int o = 0; o < count_optDefs; o++)
                {
                    CLI_OPT opt = optDefs[o];
                    if (strcmp(arg, opt.long_opt) == 0)
                    {
                        ParseSingleOpt(&opt, argv[++a]);
                    }
                }
            }
            else    // Short opts
            {
                arg += 1;
                for (int o = 0; o < count_optDefs; o++)
                {
                    CLI_OPT opt = optDefs[o];
                    if (*arg == *opt.short_opt)
                    {
                        ParseSingleOpt(&opt, argv[++a]);
                    }
                }
            }
        }
    }
}