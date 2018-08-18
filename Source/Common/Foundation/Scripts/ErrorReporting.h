#pragma once

#include "ErrorHandling.h"

#define LuaRunError(lua, Reason, DetailsFmt, ...)                                                           \
do {                                                                                                        \
    auto msg = fmt::format("{} : Error : {} : " DetailsFmt, ScriptFunctionName, Reason, __VA_ARGS__ );      \
    lua_pushstring(lua, msg.c_str());                                                                       \
    lua_error(lua);                                                                                         \
} while (false) 

#define LuaRunWarning(lua, Reason, DetailsFmt, ...)                                                         \
do {                                                                                                        \
    auto msg = fmt::format("{} : Warning : {} : " DetailsFmt, ScriptFunctionName, Reason, __VA_ARGS__ );    \
    if(lua) { lua_pushstring(lua, msg.c_str()); LuaTraceback(lua); }                                        \
} while (false) 

