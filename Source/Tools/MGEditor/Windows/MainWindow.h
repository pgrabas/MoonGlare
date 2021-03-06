#ifndef MAINFORM_H
#define MAINFORM_H

#include <DockWindowInfo.h>
#include <Notifications.h>
#include <qtUtils.h>
#include <iEditor.h>
#include <iFileProcessor.h>
#include <ChangesManager.h>

#include <ToolBase/Module.h>
#include <MiscIfs.h>

namespace Ui { class MainWindow; }

namespace MoonGlare {
namespace Editor {

namespace Module {
	class DataModule;
}
class FileSystem;
using SharedFileSystem = std::shared_ptr<FileSystem>;

struct SharedData {
	std::unique_ptr<QtShared::ChangesManager> m_ChangesManager;

	SharedData() {
		m_ChangesManager = std::make_unique<QtShared::ChangesManager>();
	}
};

class MainWindow
	: public QMainWindow
	, public iModule
	, public QtShared::UserQuestions
	, public iSettingsUser
	, public QtShared::MainWindowProvider
	, public QtShared::QtWindowProvider<MainWindow> {
	Q_OBJECT
public:
	MainWindow(SharedModuleManager modmgr);
	~MainWindow();
	static MainWindow* Get();

	SharedData* GetSharedData() { return &m_SharedData; }
protected:
	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
	
	QWidget *GetMainWindowWidget() override { return this; }
	MainWindow *GetWindow() override { return this; }
	bool PostInit() override;
private:
	std::unique_ptr<Ui::MainWindow> m_Ui;
    QTimer refreshTimer;

    QLabel *jobProcessorStatus = nullptr;

	std::shared_ptr<QtShared::EditorProvider> m_EditorProvider;
    QtShared::SharedJobProcessor jobProcessor;

//	std::vector<QtShared::SharedBaseDockWindowModule> m_DockWindows;
	std::unordered_map<std::string, QtShared::SharedBaseDockWindowModule> m_Editors;
	SharedData m_SharedData;

	Module::SharedDataModule m_DataModule;
	void NewModule(const std::string& MasterFile);
	void OpenModule(const std::string& MasterFile);
	void CloseModule();

public slots:
    void RefreshStatus();
protected slots:
	void NewModuleAction();
	void OpenModuleAction();
	void CloseModuleAction();
	void CloseEditorAction();
    void BuildModuleAction();
    void PackModuleAction();
};

} //namespace Editor
} //namespace MoonGlare

#endif // MAINFORM_H
