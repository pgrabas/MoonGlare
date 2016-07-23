/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef iSettingsUser_H
#define iSettingsUser_H

#include <qobject.h>

namespace MoonGlare {
namespace QtShared {

class iSettings : public QObject {
	Q_OBJECT;
public:
	void Save();
	void Load();

	static iSettings* GetiSettings();

	struct iSettingsUser {
		virtual ~iSettingsUser() {}

		void SaveSettings();
		void LoadSettings();
		virtual bool DoSaveSettings(pugi::xml_node node) const = 0;
		virtual bool DoLoadSettings(const pugi::xml_node node) = 0;
		const std::string& GetSettingID() const { return m_SettingID; }
	protected:
		void SetSettingID(std::string v) { m_SettingID.swap(v); }

		template<class T>
		static void SaveGeometry(pugi::xml_node node, T *widget, const char *Name) {
			XML::UniqeChild(node, Name).text() = widget->saveGeometry().toHex().constData();
		}
		template<class T>
		static void LoadGeometry(const pugi::xml_node node, T *widget, const char *Name) {
			widget->restoreGeometry(QByteArray::fromHex(node.child(Name).text().as_string()));
		}

		template<class T>
		static void SaveState(pugi::xml_node node, T *widget, const char *Name) {
			XML::UniqeChild(node, Name).text() = widget->saveState().toHex().constData();
		}
		template<class T>
		static void LoadState(const pugi::xml_node node, T *widget, const char *Name) {
			widget->restoreState(QByteArray::fromHex(node.child(Name).text().as_string()));
		}
	private:
		std::string m_SettingID;
	};

protected:
	iSettings();
	~iSettings();

	virtual void SaveStaticSettings(pugi::xml_node node);
	virtual void LoadStaticSettings(pugi::xml_node node);

	void SaveSettings(iSettingsUser *user);
	void LoadSettings(iSettingsUser *user);
	pugi::xml_node StaticSettingsRoot();
	pugi::xml_node DynamicSettingsRoot();
	
	std::unique_ptr<pugi::xml_document> m_SettingsDoc;
	std::string m_FileName;
};

using iSettingsUser = iSettings::iSettingsUser;

} //namespace QtShared
} //namespace MoonGlare

#endif
