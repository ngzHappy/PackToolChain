#include <cplusplus_stream.hpp>
#include <cplusplus_string.hpp>
#include <cplusplus_containers.hpp>
#include <lua/lua.hpp>

static void print_help() {
    std::cout<<"you should input full path of pack_name.lua"<<std::endl;
}

static std::vector<std::string> find_all_pack_hpp(const zns::filesystem::path&arg) {
    std::vector<std::string> varAns;
    zns::filesystem::directory_iterator varI{ arg };
    const zns::filesystem::directory_iterator varEnd;
    for (; varI!=varEnd; ++varI) {
        if (zns::ends_with(varI->path().filename().string(),".pack.hpp")) {
            varAns.push_back(varI->path().string());
        }
    }
    return std::move(varAns);
}
static std::vector<std::string> find_all_pack_name_hpp(const zns::filesystem::path&arg) {
    std::vector<std::string> varAns;
    zns::filesystem::directory_iterator varI{ arg };
    const zns::filesystem::directory_iterator varEnd;
    for (; varI!=varEnd; ++varI) {
        if (zns::ends_with(varI->path().filename().string(),".pack.name.hpp")) {
            varAns.push_back(varI->path().string());
        }
    }
    return std::move(varAns);
}

static const zns::string & bom() {
    const static zns::string varBom{ char(0x00ef),char(0x00bb),char(0x00bf) };
    return varBom;
}

static zns::string read_lua_file(const zns::string & arg) {
    zns::ifstream ifs{ arg };
    zns::string varAns;
    if (ifs.is_open()) {
        zns::string varLine;

        /*remove utf8 bom*/
        std::getline(ifs,varLine);
        if (zns::starts_with(varLine,bom())) {
            varAns=zns::string(varLine.begin()+3,varLine.end())+"\n";
        }
        else {
            varAns=std::move(varLine)+"\n";
        }

        while (std::getline(ifs,varLine)) {
            varAns+=std::move(varLine)+"\n";
        }
        return varAns;
    }
    return "";
}

static void make_pack_hpp_concept(
    const zns::string &,
    zns::ofstream &
) {

}

static void make_pack_cpp_concept(
    const zns::string &,
    zns::ofstream &ofs
) {
    ofs<<bom()<<std::endl;
}

static void make_pack_hpp(
    const zns::string &argUsingNameSpace,
    const zns::string &argPackName,
    const zns::filesystem::path&argFilePath
) {
    zns::filesystem::ofstream ofs(argFilePath,std::ios::binary);
    ofs<<bom()<<std::endl;
    zns::string MacroPackName=zns::replace_all_copy(argPackName,"::"s,"___"s);
    zns::to_upper(MacroPackName);

    ofs<<"#ifndef PRAGMA_ONCE_PACK_"<<MacroPackName<<"__"<<std::endl;
    ofs<<"#define PRAGMA_ONCE_PACK_"<<MacroPackName<<"__"<<std::endl;

    zns::vector<zns::string> varNamespaces;
    zns::split(varNamespaces,argPackName,zns::is_any_of("::"s));

    {
        auto varLast=std::remove_if(varNamespaces.begin(),varNamespaces.end(),
            [](const zns::string&arg) {
            if (arg.empty()) { return true; }
            if (zns::trim_copy(arg).empty()) { return true; }
            return false;
        });
        varNamespaces.erase(varLast,varNamespaces.end());
    }

    ofs<<std::endl;
    ofs<<"#include <memory>"<<std::endl;
    ofs<<"#include <cstddef>"<<std::endl;
    ofs<<"#include <cstdint>"<<std::endl;
    ofs<<"#include <type_traits>"<<std::endl;
    ofs<<std::endl;

    ofs<<"#ifndef PACK_CLASS_NAME"<<std::endl;
    ofs<<"#define PACK_CLASS_NAME(...) __VA_ARGS__"<<std::endl;
    ofs<<"#endif"<<std::endl;
    ofs<<std::endl;

    ofs<<"#ifndef PACK_PUBLIC_SUPER_NAME"<<std::endl;
    ofs<<"#define PACK_PUBLIC_SUPER_NAME(...) public __VA_ARGS__"<<std::endl;
    ofs<<"#endif"<<std::endl;
    ofs<<std::endl;
    
    ofs<<"#ifndef PACK_PRIVATE_SUPER_NAME"<<std::endl;
    ofs<<"#define PACK_PRIVATE_SUPER_NAME(...) private __VA_ARGS__"<<std::endl;
    ofs<<"#endif"<<std::endl;
    ofs<<std::endl;
    
    ofs<<"#ifndef PACK_PROTECTED_SUPER_NAME"<<std::endl;
    ofs<<"#define PACK_PROTECTED_SUPER_NAME(...) protected __VA_ARGS__"<<std::endl;
    ofs<<"#endif"<<std::endl;
    ofs<<std::endl;

    ofs<<std::endl;

    if (varNamespaces.empty()==false) {
        for (const auto & i:varNamespaces) {
            ofs<<"namespace "<<i<<" { ";
        }
        ofs<<std::endl;
    }

    ofs<<std::endl;
    make_pack_hpp_concept(argPackName,ofs);
    ofs<<std::endl;

    if (varNamespaces.empty()==false) {
        for (const auto & i:varNamespaces) {
            ofs<<"} /*"<<i<<"*/";
        }
        ofs<<std::endl;
    }

    ofs<<std::endl;

    if (argUsingNameSpace.empty()==false) {
        ofs<<std::endl;
        ofs<<argUsingNameSpace<<std::endl;
    }

    ofs<<std::endl;

    ofs<<"#endif"<<std::endl;
    ofs<<std::endl;
    ofs<<std::endl;
}

static void update_hpp_files(
    zns::filesystem::path&argDir,
    const zns::string &argPackName
) {

    std::vector<std::string> allNeedUpdataHpp=find_all_pack_hpp(argDir);
    std::map<std::string,std::int32_t> type_index;

    if (allNeedUpdataHpp.empty()) { return; }

    /*make class names*/
    for (const auto & i:allNeedUpdataHpp) {

        auto pos_=i.find_last_of(u8R"(/\)");
        if (pos_==i.npos) { continue; }

        std::string varClassName{ i.begin()+pos_+1,
        i.end()-9 };

        if (varClassName.empty()) { continue; }

        {
            zns::filesystem::ofstream ofs{argDir/(varClassName+".pack.name.hpp"),std::ios::binary};
            if (ofs.is_open()) {
                
                ofs<<"/*packname:"<<argPackName<<"*/"<<std::endl;
                
                ofs<<u8R"(constexpr static const char * classNameConcept(){ return ")";
                ofs<<varClassName<<u8R"(" ;})"<<std::endl;

                ofs<<u8R"(constexpr static unsigned int classNameLengthConcept(){ return )";
                ofs<<varClassName.size()<<" ;}"<<std::endl;

                ofs<<u8R"(constexpr static const char * packNameConcept(){ return ")";
                ofs<<argPackName<<u8R"(" ;})"<<std::endl;

                ofs<<u8R"(constexpr static unsigned int packNameLengthConcept(){ return )";
                ofs<<argPackName.size()<<" ;}"<<std::endl;

                auto this_index=type_index.size()+1;
                type_index[argPackName]=this_index;

                ofs<<u8R"(constexpr static unsigned int classOffsetOfPackConcept(){ return )";
                ofs<<this_index<<" ;}"<<std::endl;

                ofs<<"/**/"<<std::endl<<std::endl;
            }
        }

    }

}

static void make_files(
    const zns::string &argUsingNameSpace,
    const zns::string &argPackName,
    const zns::string &argPackNameLuaPathName
) {
    zns::filesystem::path varPackNameLuaPathNamePath{ argPackNameLuaPathName };
    auto varDir=varPackNameLuaPathNamePath.parent_path();

    make_pack_hpp(argUsingNameSpace,argPackName,varDir/"pack.hpp");
    make_pack_cpp_concept(argPackName,
        zns::filesystem::ofstream{ varDir/"pack.cpp" ,std::ios::binary });

    zns::string varPackLuaName="pack-"s+argPackName+".lua";
    zns::replace_all(varPackLuaName,"::"s,"-"s);
    auto varPackLuaPath=varDir/varPackLuaName;
    if (zns::filesystem::exists(varPackLuaPath)==false) {
        zns::filesystem::ofstream ofs{ varPackLuaPath,std::iostream::binary };
        zns::string varAboutToWrite=u8R"(pack={
    namespace="_!_namespace_!_",--[[namespace of the pack 
    demo:if namespace is abc::efg 
    the the file name is pack-abc-efg.lua]]
    depend="",--[[import other namespace]]
    base={"std","boost"},--[[3rd namespace]]
    classes={
        {class="",supers={{class="",deepth=-1,isPrivate=false,},},},
    },--[[]]
})"s;
        varAboutToWrite.replace(varAboutToWrite.find("_!_namespace_!_"s),15,argPackName);
        ofs<<std::endl;
        ofs<<varAboutToWrite;
        ofs<<std::endl;
    }

    update_hpp_files(varDir,argPackName);

}

int main(int argc,char *argv[]) {
    if (argc<=1) { print_help(); return 0; }

    zns::string varPackName;
    zns::string varUsingNameSpace;
    zns::string varPackNameLuaPathName=argv[1];

    if (zns::ends_with(varPackNameLuaPathName,"pack_name.lua"s)) {
        zns::string varLuaFile=read_lua_file(varPackNameLuaPathName);
        if (varLuaFile.empty()) {
            std::cout<<varPackNameLuaPathName<<" is null!";
            std::cout<<"the file shoule lick this:"<<std::endl;
            std::cout<<u8R"(return {
    packName="YouMustSetThePackName",
    usingNameSpace="",
};
)"<<std::endl;
            return -2;
        }

        auto * L=luaL_newstate();
        if (L==nullptr) {
            std::cout<<"can not open lua state"<<std::endl;
            return -3;
        }

        luaL_openlibs(L);

        if (luaL_loadbuffer(L,
            varLuaFile.c_str(),varLuaFile.size(),
            varPackNameLuaPathName.c_str()
            )==LUA_OK) {

            if (lua_pcall(L,0,LUA_MULTRET,0)==LUA_OK) {
                if (lua_istable(L,-1)) {

                    auto var_table_index=lua_gettop(L);

                    {
                        lua_getfield(L,var_table_index,"packName");
                        if (lua_isstring(L,-1)) {
                            varPackName=lua_tostring(L,-1);
                        }
                        else {
                            std::cout<<"packName is not a string"<<std::endl;
                            return -7;
                        }
                    }

                    {
                        lua_getfield(L,var_table_index,"usingNameSpace");
                        if (lua_isstring(L,-1)) {
                            varUsingNameSpace=lua_tostring(L,-1);
                        }
                    }

                }
                else {
                    std::cout<<"the file shoule lick this:"<<std::endl;
                    std::cout<<u8R"(return {
    packName="YouMustSetThePackName",
    usingNameSpace="",
};
)"<<std::endl;
                    return -6;
                }
            }
            else {
                std::cout<<lua_tostring(L,-1)<<std::endl;
                return -5;
            }

        }
        else {
            std::cout<<lua_tostring(L,-1)<<std::endl;
            return -4;
        }

        lua_close(L);

        /************************/
        make_files(
            varUsingNameSpace,
            varPackName,
            varPackNameLuaPathName);
        /************************/

    }
    else {
        print_help();
        std::cout<<varPackNameLuaPathName<<" do not ends with pack_name.lua";
        return -1;
    }
    return 0;
}

/*

*/
