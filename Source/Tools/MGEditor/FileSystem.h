/*
  * Generated by cppsrc.sh
  * On 2016-07-22 11:09:18,36
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include <qobject.h>
#include <QFileSystemWatcher>

#include "Notifications.h"

#include <StarVFS/core/nStarVFS.h>
#include <Foundation/iFileSystem.h>

#include <iFileProcessor.h>
#include <ToolBase/Module.h>


namespace StarVFS {
	class StarVFS;
	using SharedStarVFS = std::shared_ptr<StarVFS>;
}

namespace MoonGlare {
namespace Editor {

class AsyncFileProcessor;

class FileSystem 
	: public QObject
    , public iModule
    , public iFileSystem {
	Q_OBJECT;
public:
 	FileSystem(SharedModuleManager modmgr);
 	virtual ~FileSystem();

	StarVFS::SharedStarVFS GetVFS() { return m_VFS; }

	bool GetFileData(const std::string &uri, StarVFS::ByteTable &data);
	bool SetFileData(const std::string &uri, StarVFS::ByteTable &data);
	bool CreateFile(const std::string &uri);
	bool CreateDirectory(const std::string &uri);

    std::string TranslateToSystem(const std::string &uri);

    bool OpenFile(StarVFS::ByteTable &FileData, const std::string& uri) override {
        return GetFileData(uri, FileData);
    }
    bool OpenFile(StarVFS::ByteTable &FileData, StarVFS::FileID fid) override {
        __debugbreak();
        throw false;
    };

    void FindFilesByExt(const char *ext, StarVFS::DynamicFIDTable &out)  override {
        __debugbreak();
        throw false;
    };
    std::string GetFileName(StarVFS::FileID fid) const override {
        __debugbreak();
        throw false;
    };
    std::string GetFullFileName(StarVFS::FileID fid) const override {
        __debugbreak();
        throw false;
    };

    bool OpenXML(XMLFile &doc, const std::string& uri) {
        __debugbreak();
        throw false;
    }
    bool EnumerateFolder(const std::string& Path, FileInfoTable &FileTable, bool Recursive) override {
        __debugbreak();
        throw false;
    }
    
    bool Initialize() override;
    bool PostInit() override;

	void QueueFileProcessing(const std::string &URI);
public slots:
	void Reload();
signals:
	void Changed();
protected slots:
	void ProjectChanged(Module::SharedDataModule datamod);
private:
	std::mutex m_Mutex;
	StarVFS::SharedStarVFS m_VFS;
	std::string m_BasePath;
	Module::SharedDataModule m_Module;
	std::unordered_map<std::string, std::vector<std::weak_ptr<QtShared::iFileProcessorInfo>>> m_ExtFileProcessorList;
    QtShared::SharedJobProcessor jobProcessor;
    QtShared::SharedJobFence jobFence;
    QFileSystemWatcher fsWatcher;
    QTimer watcherTimeout;
    std::set<std::string> changedPaths;

	bool TranslateURI(const std::string &uri, std::string &out);

    void UpdateFsWatcher();
    void HandleDirectoryChanged(const std::string &URI);
    void RefreshChangedPaths();
};

using SharedFileSystem = std::shared_ptr<FileSystem>;

} //namespace Editor 
} //namespace MoonGlare 
