#include "Config.hpp"
#include <Procpp/process.hpp>
#include <filesystem>
#include <iostream>
#include <string>


int main(int argc, const char* argv[]){

    Config cfg;



    if(argc <= 1 || argv[1] != std::string("init")){
        std::string configfile = (argc >= 2 ? argv[1] : cfg.default_config);
        if(!std::filesystem::exists(configfile)) {
            if(std::filesystem::exists(configfile + std::string(".cfg"))){
                configfile = configfile +std::string(".cfg");
            }
            else{
                std::cout << "configfile \"" << configfile << "\" does not exist!" << std::endl;
                std::cout << "Create a template with \"" << (argc >= 1? argv[0] : "cman") << "\" init [project / library name](.cfg)" << std::endl;
                return -1;
            }
        }
        std::cout << "using config: " << configfile << std::endl;

        cfg.load(configfile);
        std::cout << cfg << std::endl;
        {
            auto targets = cfg.scan().compile();
            std::cout << "compiling " << targets.size() << " targets..." << std::endl;
        }
        auto proc = cfg.combine();
        cfg.save_cache();
    }
    else if(argv[1] == std::string("init")){
        cfg.save_config((argc >= 3? argv[2] : cfg.default_config));
        std::cout << cfg << std::endl;
    }


    return 0;
}