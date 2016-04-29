#ifndef DOCKWINDOWINFO_H
#define DOCKWINDOWINFO_H

class DockWindow;

#include "mgdtSettings.h"

class DockWindowInfo 
		: public QObject
		, public GabiLib::GabiObject
		, public iSettingsUser {
	Q_OBJECT;
	GABI_DECLARE_STATIC_CLASS(DockWindowInfo, GabiLib::GabiObject);
public:
	DockWindowInfo();
	virtual ~DockWindowInfo();

	const QString& GetIconResName() const{ return m_IconResName; }
	const QString& GetDisplayName() const { return m_DisplayName; }
	const QString& GetShortcut() const { return m_ShortCut; }

	QIcon GetIcon() const { return QIcon(GetIconResName()); }
	QKeySequence GetKeySequence() const { return QKeySequence(GetShortcut()); }

	std::shared_ptr<DockWindow> GetInstance(QWidget *parent);
	void ReleaseInstance();
public slots:
	void Show();
protected:
	void SetIconResName(QString v) { m_IconResName.swap(v); }
	void SetDisplayName(QString v) { m_DisplayName.swap(v); }
	void SetShortcut(QString v) { m_ShortCut.swap(v); }

	virtual std::shared_ptr<DockWindow> CreateInstance(QWidget *parent) = 0;

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
protected slots:
	void WindowClosed(DockWindow* Sender);
private:
	QString m_IconResName;
	QString m_DisplayName;
	QString m_ShortCut;
	std::shared_ptr<DockWindow> m_Instance;
};

using DockWindowClassRgister = GabiLib::DynamicClassRegister<DockWindowInfo>;
using SharedDockWindowInfo = std::shared_ptr<DockWindowInfo>;

#endif // SUBWINDOW_H