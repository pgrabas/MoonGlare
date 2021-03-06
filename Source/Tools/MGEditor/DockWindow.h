#ifndef DOCKWINDOW_H
#define DOCKWINDOW_H

#include <qobject.h>
#include <ToolBase/iSettingsUser.h>
#include "qtUtils.h"

namespace MoonGlare {
namespace QtShared {

class DockWindow 
	: public QDockWidget
	, public UserQuestions
	, public iSettingsUser
	, public std::enable_shared_from_this<DockWindow> {
	Q_OBJECT;
public:
	DockWindow(QWidget *parent, bool AutoRefresh = false);
	~DockWindow();

	void closeEvent(QCloseEvent * event);
	void showEvent(QShowEvent * event);
	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;

	void SetAutoRefresh(bool value, unsigned Interval = 1000);
signals:
	void WindowClosed(DockWindow* Sender);
    void UIUpdate(std::function<void()> h);
public slots:
	virtual void Refresh();
protected slots:
    void UIUpdateImpl(std::function<void()> h);
private:
	bool m_AutoRefresh;
	std::unique_ptr<QTimer> m_RefreshTimer;
};

} //namespace QtShared
} //namespace MoonGlare

Q_DECLARE_METATYPE(std::function<void()>);

#endif // SUBWINDOW_H
