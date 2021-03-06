#pragma once

#include <Foundation/Settings.h>

namespace MoonGlare::Core::Scripts::Settings {

using ValueVariant = MoonGlare::Settings::ValueVariant;
using ApplyMethod = MoonGlare::Settings::ApplyMethod;

struct Setting {
    ApplyMethod applyMethod = ApplyMethod::Restart;
};

class iSettingsProvider {
public:
    struct InvalidSettingId {}; //may be thrown from Set or Get

    virtual std::unordered_map<std::string, Setting> GetSettingList(std::string_view prefix) const = 0;
    virtual void Set(std::string_view prefix, std::string_view id, ValueVariant value) = 0;
    virtual ValueVariant Get(std::string_view prefixx, std::string_view id) = 0;
};

class iLuaSettingsModule {
public:
    virtual void RegisterProvider(std::string prefix, iSettingsProvider *provider) = 0;
};

} //namespace MoonGlare::Core::Script::Settings
