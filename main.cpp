#include <iostream>
#include <functional>
#include <fstream>

#include "json.hpp"

extern "C" {
# include "lua.h"
# include "lauxlib.h"
# include "lualib.h"
}

#define N_FUNCTIONS 6

using json = nlohmann::json;

static int l_meta(lua_State * state);
static int l_beginclip(lua_State * state);
static int l_endclip(lua_State * state);
static int l_frame(lua_State * state);
static int l_embed(lua_State * state);
static int l_rootclip(lua_State * state);

struct function_registry {
    const char * name;
    int (*function) (lua_State *);
};

static function_registry regs[N_FUNCTIONS] = {
        {"meta", &l_meta},
        {"beginclip", &l_beginclip},
        {"endclip", &l_endclip},
        {"frame", &l_frame},
        {"embed", &l_embed},
        {"rootclip", &l_rootclip}
};

json j;

int main(int argc, char *argv[]) {

    if( argc < 2 ) {
        std::cout << "Too few arguments!" << std::endl;
        exit(1);
    }

    lua_State * state = luaL_newstate();

    if( luaL_loadfile(state, argv[1]) ) {
        std::cout << "Failed to open or parse file: " << argv[1] << std::endl;
        exit(1);
    }

    for(auto & reg : regs) {
        std::cout << "Registering function: " << reg.name << std::endl;

        lua_pushcfunction(state, reg.function);
        lua_setglobal(state, reg.name);
    }

    if( lua_pcall(state, 0, 0, 0) ) {
        std::cout << "Failed to run script!" << std::endl;
        exit(1);
    }

    lua_close(state);

    std::ofstream o("pretty.json");
    o << std::setw(0) << j << std::endl;
    o.close();

    return 0;
}

static int l_meta(lua_State * state) {
    luaL_checktype(state, 1, LUA_TTABLE);

    std::cout << "meta" << std::endl;

    lua_getfield(state, 1, "audio");
    auto audio = lua_tostring(state, -1);

    lua_getfield(state, 1, "year");
    auto year = lua_tointeger(state, -1);

    lua_getfield(state, 1, "team");
    auto team = lua_tostring(state, -1);

    lua_getfield(state, 1, "title");
    auto title = lua_tostring(state, -1);

    std::cout << "audio: " << audio << std::endl;
    std::cout << "year: " << year << std::endl;
    std::cout << "team: " << team << std::endl;
    std::cout << "title: " << title << std::endl;

    j["meta"] = {
            {"team", team},
            {"year", year},
            {"title", title},
            {"audio", audio},
            {"clips", json::array()}
    };

    return 0;
}

static int l_beginclip(lua_State * state) {
    auto width = lua_tointeger(state, 1);
    auto height = lua_tointeger(state, 2);
    auto name = lua_tostring(state, 3);

    std::cout << "beginclip(" << width << ", " << height << ", " << name << ")" << std::endl;

    auto & meta = j["meta"];

    meta["clips"][meta["clips"].size()] = {
            {"width", width},
            {"height", height},
            {"name", name},
            {"frames", json::array()},
            {"embeds", json::array()}
    };

    return 0;
}

static int l_endclip(lua_State * state) {
    return 0;
}

static int l_frame(lua_State * state) {
    luaL_checktype(state, 1, LUA_TTABLE);
    lua_pushnil(state);

    auto & meta = j["meta"];
    auto & clip = meta["clips"][meta["clips"].size() - 1];

    std::vector<int> pixels;

    while( lua_next(state, 1) != 0 ) {

        lua_typename(state, lua_type(state, -2));
        lua_typename(state, lua_type(state, -1));

        int pixel = lua_tointeger(state, -1);

        pixels.push_back(pixel);

        lua_pop(state, 1);
    }

    clip["frames"][clip["frames"].size()]["pixels"] = pixels;

    auto duration = lua_tointeger(state, 2);

    clip["frames"][clip["frames"].size()]["duration"] = duration;

    return 0;
}

static int l_embed(lua_State * state) {
    luaL_checktype(state, 2, LUA_TTABLE);
    luaL_checktype(state, 3, LUA_TTABLE);

    auto name = lua_tostring(state, 1);

    lua_getfield(state, 2, "0");
    auto x = lua_tointeger(state, -1);

    lua_getfield(state, 3, "0");
    auto y = lua_tointeger(state, -1);

    auto z = lua_tointeger(state, 3);
    auto t = lua_tointeger(state, 4);

    auto & meta = j["meta"];
    auto & clip = meta["clips"][meta["clips"].size() - 1];

    clip["embeds"][clip["embeds"].size()] = {
            {"name", name},
            {"x", x},
            {"y", y},
            {"z", z},
            {"t", t}
    };

    return 0;
}

static int l_rootclip(lua_State * state) {
    auto name = lua_tostring(state, 1);

    std::cout << "rootclip: " << name << std::endl;

    j["meta"]["rootclip"] = name;

    return 1;
}