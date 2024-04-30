#pragma once



#include <Procpp/process.hpp>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>
#include <istream>
#include <map>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


//heavy usage for span where implementation was lazy
class Config{
private:


public:
    static constexpr const char* default_config = "cman.cfg";
    static constexpr const char* default_cache = ".cman_cache";

    using Target = std::pair<std::filesystem::path, size_t>;
    using Targets = std::map<std::filesystem::path, size_t>;


    
    Targets targets;

    size_t hash(std::istream& file){
        return 0;
    }
    size_t hash(std::istream&& file){
        return 0;
    }
    size_t hash(const std::filesystem::path& file){
        return hash(std::ifstream(file));
    }


    std::vector<std::string> extensions()const{
        std::string exts;
        try {
        exts = config.at("EXT");
        } catch (const std::out_of_range&) {
            return {};
        }
        std::vector<std::string> out;
        std::size_t pos = 0;
        for(std::size_t i = 0; i < exts.size(); i++){
            if(exts[i] == ' '){
                out.push_back(exts.substr(pos, i-pos));
                pos = i+1;
            }
        }
        if(pos+1 < exts.size()){
            out.push_back(exts.substr(pos));
        }
        return out;
    }

    bool in_extensions(const std::filesystem::path& p){
        auto ext = p.extension().string();
        for(auto& e : extensions()){
            //std::cout << ext << " == " << e << "  " << (ext == e? "True" : "False") << std::endl;
            if(ext == e) return true;            
        }
        return false;
    }


    std::map<std::string, std::string> config = {
        std::pair("CC", "g++"),
        std::pair("CFLAGS", "-Wall -std=c++20"),
        std::pair("SCANDIR", "test/src"),
        std::pair("OUTDIR", "test/bin"),
        std::pair("OUTFILE", "out.a"),
        std::pair("EXT", ".c++ .cpp .c"),
        std::pair("CACHE", default_cache),
    };

    Config& load(const std::filesystem::path p = default_config){
        if(!std::filesystem::exists(p))return *this;
        std::ifstream f(p);
        std::string str;
        while (f) try {
            std::getline(f, str);
            if(str.empty() || str.front()=='#') continue;
            auto pos = str.find_first_of('=');
            if(pos == str.npos) continue;
            auto name = str.substr(0,pos);
            auto value = str.substr(pos+1);
            if(name.empty() || value.empty()) continue;
            config[name] = value;
        }catch(...){continue;}
        return *this;
    }
    //what do i want this to do

    Config& scan(){
        auto p = config["SCANDIR"];
        if(!std::filesystem::exists(p))return *this;

        for(const auto& p : std::filesystem::recursive_directory_iterator(p)) {
            if(p.is_directory() || !in_extensions(p) || targets.contains(p.path())) continue;
            targets[p] = -1;
        };

        return *this;
    }


    Process combine(){
        auto of = config["OUTFILE"];
        auto od = config["OUTDIR"];
        if(of.empty()) of = "out.a";

        auto out = (std::filesystem::path(od) /= of).string();
        auto command = "ar rcs " + out + " $(find " + od + " -name \"*.o\")" ;
        std::cout << command << std::endl;
        return Process::spawn(command.c_str());
    }


    Config& ReadCache(std::istream&& f){
        std::string str;
        while (f) try {
            std::getline(f, str);
            if(str.empty() || str.front() == '#') continue;
            auto pos = str.find_first_of(':');
            if(pos == str.npos) continue;

            std::filesystem::path path = str.substr(0,pos);
            size_t hash = std::stoul(str.substr(pos+1), 0 , 16);
            if(!std::filesystem::exists(path)) continue;
            targets[path] = hash;
        }catch(...){continue;}
        return *this;
    }

    Process compile_target(const std::filesystem::path& path){
        auto p = path.parent_path().string().substr(config["SCANDIR"].size());
        while (!p.empty() && p.front() == '/') p.erase(p.begin());
        auto out = (std::filesystem::path(config["OUTDIR"])/=p)/=path.filename().replace_extension(".o");

        if(!std::filesystem::exists(out.parent_path())) Process::spawn(("mkdir -p " + out.parent_path().string()).c_str());

        auto command = config["CC"]+" "+config["CFLAGS"]+" -c "+path.string()+" -o "+out.string();
        std::clog << "command: " << command << std::endl;
        return Process::spawn((command).c_str());
    }


    std::vector<Process> compile(){
        std::vector<Process> procs;
        ReadCache(std::ifstream(config["CACHE"]));
        for(auto& e : targets){
            auto h = hash(e.first);
            if(h != e.second){procs.emplace_back(compile_target(e.first));}
            e.second = h;
        }

        return procs;
    }

    Config& save_cache(){
        std::filesystem::path p = config["CACHE"];
        if(p.empty()) p = default_cache;
        std::ofstream f(p);
        for(auto& e : targets){
            f << e.first.c_str() << ':' << std::hex << std::uppercase << e.second << '\n';
        }
        return *this;
    }

    Config& save_config(std::filesystem::path p = default_config){
        std::ofstream f(p);

        for(auto& e : config){
            f << e.first << "=" << e.second << '\n';    
        }

        return *this;
    }





    friend std::ostream& operator<<(std::ostream& os, const Config cfg){
        os << "{ ";
        auto it = cfg.config.begin();
        auto end = cfg.config.end();
        while (true) {
            const auto& e = *it;
            os << e.first << ": " << e.second;
            it++;
            if(it == end)
                break;
            os << ", ";
        }
        os << " }";
        return os;
    }


    Config(){}
    ~Config(){}
};



