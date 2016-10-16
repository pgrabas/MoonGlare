#include PCH_HEADER
#include <qobject.h>
#include "EditorSettings.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <DockWindow.h>
#include <qtUtils.h>

#include "Module/DataModule.h"
#include "Notifications.h"
#include "FileSystem.h"

#include "SettingsWindow.h"

namespace MoonGlare {
namespace Editor {

static MainWindow *_Instance = nullptr;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	_Instance = this;
	SetSettingID("MainWindow");
	m_Ui = std::make_unique<Ui::MainWindow>();
	m_Ui->setupUi(this);

	m_FileSystem = std::make_shared<FileSystem>();
	
	connect(m_Ui->actionNew, SIGNAL(triggered()), SLOT(NewModuleAction()));
	connect(m_Ui->actionOpen, SIGNAL(triggered()), SLOT(OpenModuleAction()));
	connect(m_Ui->actionClose, SIGNAL(triggered()), SLOT(CloseModuleAction()));
	connect(m_Ui->actionExit, SIGNAL(triggered()), SLOT(CloseEditorAction()));
	connect(m_Ui->actionEditorConfiguration, &QAction::triggered, [this]() {
		SettingsWindow sw(this);
		sw.exec();
	} );

	m_DockWindows.reserve(256);//because why not
	QtShared::DockWindowClassRgister::GetRegister()->Enumerate([this](auto &ci) {
		auto ptr = ci.SharedCreate(this);
		m_DockWindows.push_back(ptr);

		if (ptr->IsMainMenu()) {
			m_Ui->menuWindows->addAction(ptr->GetIcon(), ptr->GetDisplayName(), ptr.get(), SLOT(Show()), ptr->GetKeySequence());
			AddLogf(Info, "Registered DockWindow: %s", ci.Alias.c_str());
		}

		auto einfo = dynamic_cast<QtShared::iEditorInfo*>(ptr.get());
		if (einfo) {
			auto lst = einfo->GetSupportedFileTypes();
			auto shd = GetSharedData();

			for (auto &it : einfo->GetCreateFileMethods()) {
				auto item = std::make_shared<SharedData::FileCreatorInfo>();
				*item = SharedData::FileCreatorInfo{ ptr, it, };
				AddLogf(Info, "Associated file creator: %s->%s", item->m_Info.m_Ext.c_str(), ci.Alias.c_str());
				shd->m_FileCreators.emplace_back(std::move(item));
			}

			for (auto &item : lst) {
				m_Editors[item.m_Ext] = ptr;
				shd->m_FileIconMap[item.m_Ext] = item.m_Icon;
				AddLogf(Info, "Associated editor: %s->%s", item.m_Ext.c_str(), ci.Alias.c_str());
			}
		}

		ptr->LoadSettings();
	});
	m_DockWindows.shrink_to_fit();

	LoadSettings();
}

MainWindow::~MainWindow() {
	SaveSettings();
	for (auto &it : m_DockWindows)
		it->SaveSettings();
	m_DockWindows.clear();
	m_Ui.release();
}

MainWindow* MainWindow::Get() {
	return _Instance;
}

bool MainWindow::DoSaveSettings(pugi::xml_node node) const {
	SaveGeometry(node, this, "Qt:Geometry");
	SaveState(node, this, "Qt:State");
	return true;
}

bool MainWindow::DoLoadSettings(const pugi::xml_node node) {
	LoadGeometry(node, this, "Qt:Geometry");
	LoadState(node, this, "Qt:State");

	auto conf = GetSettings().getConfiguration();
	auto state = GetSettings().getState();
	if (conf.m_LoadLastModule && !state.m_LastModule.empty()) {
		QTimer::singleShot(500, [this] {
			auto state = GetSettings().getState();
			OpenModule(state.m_LastModule);
		});
	}
	return true;
}

void MainWindow::NewModuleAction() {
	if (m_DataModule) {
		if (!QtShared::Utils::AskForPermission(this)) {
			return;
		}
	}

	auto f = QFileDialog::getSaveFileName(this, "MoonGlare", QString(),
										  QtShared::Utils::GetMoonGlareProjectFilter());

	if (f.isEmpty())
		return;

	if (m_DataModule)
		CloseModule();
	NewModule(f.toLatin1().constData());
}

void MainWindow::OpenModuleAction() {
	if (m_DataModule) {
		if (!QtShared::Utils::AskForPermission(this)) {
			return;
		}
	}

	auto f = QFileDialog::getOpenFileName(this, "MoonGlare", QString(),
										  QtShared::Utils::GetMoonGlareProjectFilter());
	if (f.isEmpty())
		return;

	if (m_DataModule) 
		CloseModule();

	OpenModule(f.toLatin1().constData());
}

void MainWindow::CloseModuleAction() {
	if (m_DataModule) {
		if (!QtShared::Utils::AskForPermission(this)) {
			return;
		}
		CloseModule();
	}
}

void MainWindow::CloseEditorAction() {
	if (m_DataModule) {
		if (!QtShared::Utils::AskForPermission(this)) {
			return;
		}
		CloseModule();
	}

	close();
}

//-----------------------------------------

void MainWindow::NewModule(const std::string& MasterFile) {
	if (m_DataModule)
		return;

	m_DataModule = Module::DataModule::NewModule(MasterFile);
	if (!m_DataModule)
		return;

	GetSettings().getState().m_LastModule = MasterFile;
	Notifications::SendProjectChanged(m_DataModule);
	AddLogf(Info, "New module: %s", MasterFile.c_str());
}

void MainWindow::OpenModule(const std::string& MasterFile)  {
	if (m_DataModule)
		return;

	QFileInfo check_file(MasterFile.c_str());
	if (!check_file.exists())
		return;

	m_DataModule = Module::DataModule::OpenModule(MasterFile);
	if (!m_DataModule)
		return;
	GetSettings().getState().m_LastModule = MasterFile;
	Notifications::SendProjectChanged(m_DataModule);
	AddLogf(Info, "Open module: %s", MasterFile.c_str());
}

void MainWindow::CloseModule() {
	if (!m_DataModule)
		return;

	m_DataModule->SaveNow();
	m_DataModule.reset();

	Notifications::SendProjectChanged(m_DataModule);
}

//-----------------------------------------

void MainWindow::showEvent(QShowEvent * event) {
}

void MainWindow::closeEvent(QCloseEvent * event) {
	m_DataModule.reset();
	event->accept();
}

//-----------------------------------------

void MainWindow::OpenFileEditor(const std::string& FileURI) {
	const char *ext = strrchr(FileURI.c_str(), '.');
	if (!ext) {
		ErrorMessage("Unknown file type!");
		return;
	}
	++ext;

	auto it = m_Editors.find(ext);
	if (it == m_Editors.end()) {
		AddLog(Warning, "No associated editor with selected file type!");
		ErrorMessage("No associated editor with selected file type!");
		return;
	}

	auto dock = it->second;
	auto inst = dock->GetInstance();
	auto editor = dynamic_cast<QtShared::iEditor*>(inst.get());
	if (!editor) {
		AddLogf(Error, "Editing in not supported by %s", typeid(*inst).name());
		ErrorMessage("Editing not supported!");
		return;
	}

	if (editor->OpenData(FileURI)) {
		inst->show();
	} else {
		ErrorMessage("Failed to open file!");
		AddLog(Error, "Failed to open file!");
	}
}

void MainWindow::CreateFileEditor(const std::string & URI, std::shared_ptr<SharedData::FileCreatorInfo> info) {
	auto inst = info->m_DockEditor->GetInstance();
	auto editor = dynamic_cast<QtShared::iEditor*>(inst.get());
	if (!editor) {
		AddLogf(Error, "Fatal Error!");
		ErrorMessage("Fatal Error!");
		return;
	}

	if (editor->Create(URI, info->m_Info)) {
		inst->show();
	} else {
		ErrorMessage("Failed to create file!");
		AddLog(Error, "Failed to create file!");
	}
}

} //namespace Editor
} //namespace MoonGlare