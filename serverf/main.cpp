//
// Created by Pamula File on 5/5/21.
//
#include "../header.h"
#include "config.hpp"
#include <semaphore.h>

void print_usage() {
    std::cerr << std::endl << "usage: webserv [file]" <<
        std::endl << std::endl << "arguments:" << std::endl <<
        "  file          Configuration file for webserver." <<
        " If no file was provided," << std::endl <<
        "                default config will be used." <<
        std::endl;
}

void	deleteAllManagmentNCookies(bool depth)
{
	DIR *dir = opendir(".");
	struct dirent *dr;
	std::string file;

	while ((dr = readdir(dir)))
	{
		file = std::string(dr->d_name);

		if (file == "." || file == "..")
			continue ;
		if (dr->d_type & DT_DIR)
		{
			chdir(dr->d_name);
			deleteAllManagmentNCookies(true);
			continue ;
		}
		if (file.find(".cookie") != std::string::npos || file.find(".management") != std::string::npos)
			remove(dr->d_name);
	}
	if (depth)
		chdir("..");
	closedir(dir);
}

int main(int ac, char *av[], char *env[])
{
    config              *config_class;

    if (ac == 2) {
        config_class = parse(av[1]);
    }
    else if (ac == 1) {
        config_class = parse("cfg/ConfigExample.ws");
    }
    else {
        std::cerr << "Wrong number of arguments. Expected 0 or 1, got " <<
            (ac - 1) << "." << std::endl;
        print_usage();
        return 1;
    }
#ifdef BONUS
	deleteAllManagmentNCookies(false);
#endif
    init_serv(*config_class);
    set_env(env, *config_class);
    loop(*config_class);
    delete config_class;
    return (0);
}
