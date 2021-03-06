#pragma once

#include <string>
#include <string_view>

namespace MoonGlare::Scripts {

class iRequireRequest {
public:
    virtual bool OnRequire(lua_State *lua, std::string_view name) = 0;
};

class iRequireModule {
public:
    virtual void RegisterRequire(const std::string &name, iRequireRequest *iface) = 0;
    virtual bool Querry(lua_State *lua, std::string_view name) = 0;
};

} //namespace MoonGlare::Scripts
