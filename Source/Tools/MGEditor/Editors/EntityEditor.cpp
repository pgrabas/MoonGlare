/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "EntityEditor.h"

#include <ui_EntityEditor.h>
#include <DockWindowInfo.h>
#include <icons.h>
#include "../Windows/MainWindow.h"
#include <FileSystem.h>

#include <TypeEditor/x2cDataTree.h>
#include <TypeEditor/Structure.h>
#include <TypeEditor/CustomType.h>

#include <iFileIconProvider.h>

namespace MoonGlare {
namespace Editor {
namespace EntityEditor {

struct EntityEditorModule
    : public QtShared::BaseDockWindowModule
    , public QtShared::iFileIconInfo 
    , public QtShared::iEditorInfo 
    , public QtShared::iEditorFactory
{

    EntityEditorModule(SharedModuleManager modmgr) : BaseDockWindowModule(std::move(modmgr)) {
        SetSettingID("EntityEditorInfo");
        SetDisplayName(tr("EntityEditor"));
        SetShortcut("F2");
    }

    std::shared_ptr<EntityEditorWindow> window;

    virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
        if(!window)
            window = std::make_shared<EntityEditorWindow>(parent, GetModuleManager());
        return window;
    }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{ "epx", ICON_16_ENTITYPATTERN_RESOURCE, },
        };
    }		

    virtual std::vector<FileHandleMethodInfo> GetCreateFileMethods() const override {
        return std::vector<FileHandleMethodInfo> {
            FileHandleMethodInfo{ "epx", ICON_16_ENTITYPATTERN_RESOURCE, "Entity pattern...", "create", },
        };
    }

    virtual std::vector<FileHandleMethodInfo> GetOpenFileMethods() const override {
        return std::vector<FileHandleMethodInfo> {
            FileHandleMethodInfo{ "epx", ICON_16_ENTITYPATTERN_RESOURCE, "Edit entity pattern", "open", },
        };
    }

    QtShared::SharedEditor GetEditor(const iEditorInfo::FileHandleMethodInfo &method, const EditorRequestOptions&options) const override {
       // if (!window)
       //     window = std::make_shared<EntityEditorWindow>(parent, GetModuleManager());
        return window;
    }
};
QtShared::ModuleClassRgister::Register<EntityEditorModule> EntityEditorReg("EntityEditor");

//----------------------------------------------------------------------------------

EntityEditorWindow::EntityEditorWindow(QWidget * parent, QtShared::SharedModuleManager modmgr)
    :  QtShared::DockWindow(parent) {
    moduleManager.swap(modmgr);
    m_Ui = std::make_unique<Ui::EntityEditor>();
    m_Ui->setupUi(this);

    SetSettingID("EntityEditorWindow");
    m_Ui->EntityTree->SetSettingID("EntityTree");
    SetChangesName("EntityEditor");
    InsertChangesChild(m_Ui->EntityTree, "Entities");

    connect(Notifications::Get(), SIGNAL(RefreshView()), SLOT(Refresh()));

    m_Ui->EntityTree->SetModuleManager(moduleManager);
    auto ue = std::make_unique<BaseEntity>();
    m_BaseEntity = ue.get();
    m_Ui->EntityTree->SetEntity(std::move(ue));
}

EntityEditorWindow::~EntityEditorWindow() {
    m_Ui.reset();
}

//----------------------------------------------------------------------------------

bool EntityEditorWindow::DoSaveSettings(pugi::xml_node node) const {
    QtShared::DockWindow::DoSaveSettings(node);
    SaveChildSettings(node, m_Ui->EntityTree);
    return true;
}

bool EntityEditorWindow::DoLoadSettings(const pugi::xml_node node) {
    QtShared::DockWindow::DoLoadSettings(node);
    LoadChildSettings(node, m_Ui->EntityTree);
    return true;
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::Refresh() {
    m_Ui->EntityTree->Refresh();
}

void EntityEditorWindow::Clear() {
    m_BaseEntity->Clear();
}

//----------------------------------------------------------------------------------

bool EntityEditorWindow::Create(const std::string &LocationURI, const QtShared::iEditorInfo::FileHandleMethodInfo& what) {
    QString qname;
    if (!QuerryStringInput("Enter name:", qname, 2))
        return false;

    std::string name = qname.toLocal8Bit().constData();
    std::string URI = LocationURI + name + ".epx";

    auto fs = moduleManager->QuerryModule<FileSystem>();
    if (!fs->CreateFile(URI)) {
        ErrorMessage("Failed during creating epx file");
        AddLog(Hint, "Failed to create epx: " << m_CurrentPatternFile);
        return false;
    }

    m_BaseEntity->GetName() = name;

    m_CurrentPatternFile = URI;
    SetModiffiedState(true);
    Refresh();

    AddLog(Hint, "Created epx file: " << URI);

    if (!SaveData()) {
        ErrorMessage("Failed during saving epx file");
        AddLog(Hint, "Failed to save epx: " << m_CurrentPatternFile);
    } else {
        SetModiffiedState(false);
    }

    return true;
}

bool EntityEditorWindow::TryCloseData() {
    AddLog(Hint, "Trying to close epx: " << m_CurrentPatternFile);
    if (m_BaseEntity && IsChanged()) {
        if (!AskForPermission("There is a opened pattern. Do you want to close it?"))
            return false;
        if (AskForPermission("Save changes?"))
            SaveData();
    }
    m_CurrentPatternFile.clear();
    Clear();
    Refresh();
    return true;
}

bool EntityEditorWindow::OpenData(const std::string &file) {
    if (!TryCloseData())
        return false;

    try {
        pugi::xml_document xdoc;

        auto fs = moduleManager->QuerryModule<FileSystem>();
        StarVFS::ByteTable bt;
        if (!fs->GetFileData(file, bt)) {
            //todo: log sth
            throw false;
        }
        if (bt.byte_size() == 0) {
            throw true;
        }

        if (!xdoc.load_string((char*)bt.get())) {
            //TODO: log sth
            throw false;
        }

        if (!m_BaseEntity->Read(xdoc.document_element())) {
            AddLog(Hint, "Failed to read epx Entities: " << m_CurrentPatternFile);
            throw false;
        }

        m_CurrentPatternFile = file;
        AddLog(Hint, "Opened epx: " << m_CurrentPatternFile);
        Refresh();
        return true;
    }
    catch (...) {
        Clear();
        Refresh();
        ErrorMessage("Failure during opening data!");
        AddLog(Hint, "Failed to open epx: " << file);
        return false;
    }
}

bool EntityEditorWindow::SaveData() {
    if (!IsChanged())
        return true;

    try {
        pugi::xml_document xdoc;
        if (!m_BaseEntity->Write(xdoc.append_child("Entity"))) {
            AddLog(Hint, "Failed to write epx Entities: " << m_CurrentPatternFile);
            throw false;
        }

        std::stringstream ss;
        xdoc.save(ss);
        StarVFS::ByteTable bt;
        bt.from_string(ss.str());

        auto fs = moduleManager->QuerryModule<FileSystem>();
        if (!fs->SetFileData(m_CurrentPatternFile.c_str(), bt)) {
            //todo: log sth
            return false;
        }

        SetModiffiedState(false);
        AddLog(Hint, "Saved epx: " << m_CurrentPatternFile);

        return true;
    }
    catch (...) {
        AddLog(Hint, "Failed to save epx: " << m_CurrentPatternFile);
        return false;
    }
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::ProjectChanged(Module::SharedDataModule datamod) {
//	m_Module = datamod;
}

} //namespace EntityEditor 
} //namespace Editor 
} //namespace MoonGlare 
