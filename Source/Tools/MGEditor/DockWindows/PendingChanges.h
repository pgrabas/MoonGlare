/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef FileSystemViewer_H
#define FileSystemViewer_H

#include <DockWindow.h>
#include "../Notifications.h"
#include "../FileSystem.h"
#include <ChangesManager.h>

namespace Ui { class PendingChanges; }

namespace MoonGlare {
namespace Editor {
namespace DockWindows {

struct PendingChangesRole {
	enum {
		//FileURI = Qt::UserRole + 1,
		//FileHash,
		//FileFullName,
	};
};

class PendingChanges
	: public QtShared::DockWindow {
	Q_OBJECT;
public:
	PendingChanges(QWidget *parent = nullptr);
 	virtual ~PendingChanges();

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
protected:
protected slots:
	void ChangesChanged(QtShared::iChangeContainer* sender, bool state);
	void Refresh();
	void SaveSingle();
	void SaveAll();

	void ShowContextMenu(const QPoint &);
	void ItemDoubleClicked(const QModelIndex&);
	void ProjectChanged(Module::SharedDataModule datamod);
private: 
	std::unique_ptr<Ui::PendingChanges> m_Ui;
	std::unique_ptr<QStandardItemModel> m_ViewModel;
};

} //namespace DockWindows 
} //namespace Editor 
} //namespace MoonGlare 

#endif