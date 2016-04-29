/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include PCH_HEADER
#include "mgdtSettings.h"

struct PugiWritterSerializer {
	PugiWritterSerializer(pugi::xml_node node) : m_Node(node) { }

	template<class T, class DEF>
	void Value( T& val, const char* Name, DEF default) {
		XML::Value::Write(m_Node.append_child(Name).text(), val);
	}
	template<class T, class Def>
	void ValueContainer(T& val, const char* Name, Def default) {
		auto child = m_Node.append_child(Name);
		for (auto &it : val) {
			auto node = child.append_child("Item");
			it.Save(node);
		}
	}
	template<class Def>
	void qStringList(QStringList& val, const char* Name, Def default) {
		auto child = m_Node.append_child(Name);
		for (auto &it : val) {
			auto node = child.append_child("Item");
			auto str = it.toStdString();
			node.text() = str.c_str();
		}
	}
	template<class LIST, class Def>
	void customlist(LIST& val, const char* Name, Def default) {
		auto child = m_Node.append_child(Name);
		for (auto &it : val) {
			auto node = child.append_child("Item");
			it.Save(node);
		}
	}
	template<class T>
	void Group(T& val, const char* Name) {
		PugiWritterSerializer w(m_Node.append_child(Name));
		val.Serialize(w);
	}
private:
	pugi::xml_node m_Node;
};

struct PugiReaderSerializer {
	PugiReaderSerializer(pugi::xml_node node): m_Node(node) { }

	template<class T, class DEF>
	void Value(T& val, const char* Name, DEF default) {
		val = XML::Value::Read(m_Node.child(Name).text(), default);
	}
	template<class T, class Def>
	void ValueContainer(T& val, const char* Name, Def default) {
		val.clear();
		auto child = m_Node.child(Name);
		if (!child) {
			if (default)
				default(val);
			return;
		}
		XML::ForEachChild(child, "Item", [&val](pugi::xml_node node) {
			T::value_type t;
			if (t.Load(node))
				val.push_back(t);
			return 0;
		});
	}
	template<class Def>
	void qStringList(QStringList& val, const char* Name, Def default) {
		val.clear();
		auto child = m_Node.child(Name);
		if (!child) {
			//if (default)
				//default(val);
			return;
		}
		XML::ForEachChild(child, "Item", [&val](pugi::xml_node node) {
			std::string s;
			s = XML::Value::Read(node.text(), "");
			val.push_back(QString(s.c_str()));
			return 0;
		});
	}
	template<class LIST, class Def>
	void customlist(LIST& val, const char* Name, Def default) {
		val.clear();
		auto child = m_Node.child(Name);
		if (!child) {
			//if (default)
			//default(val);
			return;
		}
		XML::ForEachChild(child, "Item", [&val](pugi::xml_node node) {
			LIST::value_type s;
			s.Load(node);
			val.push_back(s);
			return 0;
		});
	}
	template<class T>
	void Group(T& val, const char* Name) {
		PugiReaderSerializer w(m_Node.child(Name));
		val.Serialize(w);
	}
private:
	pugi::xml_node m_Node;
};

struct DefaultSetter {
	template<class T, class DEF>
	void Value(T& val, const char* Name, DEF default) {
		val = default;
	}

	template<class T, class Def>
	void ValueContainer(T& val, const char* Name, Def default) {
		val.clear();
		default(val);
	}
	template<class Def>
	void qStringList(QStringList& val, const char* Name, Def default) {
		val.clear();
	}
	template<class LIST, class Def>
	void customlist(LIST& val, const char* Name, Def default) {
		val.clear();
	}
	template<class T>
	void Group(T& val, const char* Name) {
		val.Serialize(*this);
	}
};

//-----------------------------------------

mgdtSettings* mgdtSettings::_Instance = nullptr;

mgdtSettings::mgdtSettings() {
	DefaultSetter setter;
	setter.Group(*this, "");

	m_SettingsDoc = std::make_unique<pugi::xml_document>();
	m_SettingsDoc->load_file("Settings.MGInsider.xml");
	
	Load();
}

mgdtSettings::~mgdtSettings() {
}

mgdtSettings& mgdtSettings::get() {
	if (!_Instance) {
		_Instance = new mgdtSettings();
	}
	return *_Instance;
}

//-----------------------------------------

void mgdtSettings::Load() {
	PugiReaderSerializer reader(StaticSettingsRoot());
	Serialize(reader);
}

void mgdtSettings::Save() {
	{
		auto root = StaticSettingsRoot();
		root.parent().remove_child(root);
	}
	PugiWritterSerializer writter(StaticSettingsRoot());
	Serialize(writter);
	m_SettingsDoc->save_file("Settings.MGInsider.xml");
}

void mgdtSettings::SaveSettings(iSettingsUser *user) {
	auto &group = user->GetSettingID();
	auto root = DynamicSettingsRoot();
	auto child = root.child(group.c_str());
	if (!child) {
		child = root.append_child(group.c_str());
	}
	user->DoSaveSettings(child);
}

void mgdtSettings::LoadSettings(iSettingsUser *user) {
	user->DoLoadSettings(DynamicSettingsRoot().child(user->GetSettingID().c_str()));
}

pugi::xml_node mgdtSettings::StaticSettingsRoot() {
	auto root = m_SettingsDoc->document_element();
	if (!root) {
		root = m_SettingsDoc->append_child("Settings");
	}
	auto Static = root.child("Settings");
	if (!Static) {
		Static = root.append_child("Settings");
	}
	return Static;
}

pugi::xml_node mgdtSettings::DynamicSettingsRoot() {
	auto root = m_SettingsDoc->document_element();
	if (!root) {
		root = m_SettingsDoc->append_child("Settings");
	}
	auto dynamic = root.child("Dynamic");
	if (!dynamic) {
		dynamic = root.append_child("Dynamic");
	}
	return dynamic;
}

//-----------------------------------------

QFont mgdtSettings::Editor_t::GetEditorFont() {
	QFont font;
    font.setFamily(FontName.c_str());
    font.setPointSize(FontSize);
    font.setFixedPitch(true);
	return font;
}

void mgdtSettings::Window_t::WindowPosition_t::Apply(QWidget *window) {
	assert(window);
	QRect pos(window->geometry());
	bool changed = false;
	if (GetSettings().Window.RememberPosition) {
		int a = PosX;
		if (a > 0) pos.setX(a);
		int b = PosY;
		if (b > 0) pos.setY(b);
		if (a > 0 || b > 0)
			changed = true;
	}
	if (GetSettings().Window.RememberSize) {
		int a = Width;
		if (a > 0) pos.setWidth(a);
		int b = Height;
		if (b > 0) pos.setHeight(b);
		if (a > 0 || b > 0)
			changed = true;
	}
	if (changed)
		window->setGeometry(pos);
}

void mgdtSettings::Window_t::WindowPosition_t::Store(QWidget *window) {
	assert(window);
	QRect pos(window->geometry());
	PosX = pos.x();
	PosY = pos.y();
	Width = pos.width();
	Height = pos.height();
}

//-----------------------------------------

void iSettingsUser::SaveSettings() {
	GetSettings().SaveSettings(this);
}

void iSettingsUser::LoadSettings() {
	GetSettings().LoadSettings(this);
}
