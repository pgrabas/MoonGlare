#include <lua.hpp>

#include <OrbitLoggerConf.h>

#include "ErrorHandling.h"

namespace MoonGlare::Scripts {

int LuaErrorHandler(lua_State *L) {
    const char *cs = lua_tostring(L, -1);
    AddLogf(ScriptRuntime, "Lua Error: %s", cs);

    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (lua_isnil(L, -1)) {
        return 0;
    }
    lua_getfield(L, -1, "traceback");
    if (lua_isnil(L, -1)) {
        return 0;
    }
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);

    cs = lua_tostring(L, -1);
    AddLogf(ScriptRuntime, "Trace: %s", cs);

    lua_pop(L, 2);
    return 0;
}

int LuaTraceback(lua_State *L) {
#ifdef DEBUG
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (lua_isnil(L, -1)) {
        return 0;
    }
    lua_getfield(L, -1, "traceback");
    if (lua_isnil(L, -1)) {
        return 0;
    }
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);

    const char *cs = lua_tostring(L, 1);
    AddLogf(Error, "Lua callstack:\n%s", cs);
    lua_pop(L, 2);
#endif
    return 0;
}

int LuaPanicHandler(lua_State *L) {
    const char *m = lua_tostring(L, 1);
    AddLogf(Error, "Lua panic! message: %s", m);
    LuaTraceback(L);
    if (!m)
        throw LuaPanic("NO MESSAGE");
    throw LuaPanic(m);
}

} //namespace MoonGlare::Scripts