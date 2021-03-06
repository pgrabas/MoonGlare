#pragma once

#include "SaveableItem.h"

namespace MoonGlare {
namespace Editor {
namespace Module {

class DataModule 
    : public SaveableItemParent {
public:
    static std::shared_ptr<DataModule> NewModule(const std::string &BaseDirectrory);
    static std::shared_ptr<DataModule> OpenModule(const std::string &BaseDirectrory);
    DataModule(const std::string &BaseDirectrory);
    virtual ~DataModule();

    const std::string& GetBaseDirectory() { return baseDirectrory; }
    std::string GetEditorConfigDirectory() { return baseDirectrory + ".editor/"; }

    std::string GetInputSettingsFile() { return GetEditorConfigDirectory() + "InputSettings.xml"; }
protected:

    bool Create();
    bool Open();

    virtual bool DoPeriodicSave() override;
private:
    std::string baseDirectrory;
};

using SharedDataModule = std::shared_ptr<DataModule>;

} //namespace Module 
} //namespace Editor 
} //namespace MoonGlare 
