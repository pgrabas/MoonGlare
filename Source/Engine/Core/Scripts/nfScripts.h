#pragma once

namespace MoonGlare::Core::Scripts {

class eLuaPanic : public std::runtime_error {
public:
    eLuaPanic(const string &msg) : std::runtime_error(msg) {
        __debugbreak();
    }
};

class ScriptEngine;

} //namespace MoonGlare::Core::Scripts

#include "LuaApi.h"
