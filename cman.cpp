#include "Config.hpp"
#include <iostream>


int main(int argc, const char* argv[]){

    Config cfg;

    cfg.load("cman.cfg");

    std::cout << cfg << std::endl;

    if(argc <= 1 || argv[1] == std::string("update")){
        {
            auto targets = cfg.scan().compile();
            std::cout << "compiling " << targets.size() << " targets..." << std::endl;
        }
        cfg.save_cache();
    }
    else if(argv[1] == std::string("init")){
        cfg.save_config((argc >= 3? argv[2] : cfg.default_config));
    }


    return 0;
}