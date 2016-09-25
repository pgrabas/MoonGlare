/*
  * Generated by cppsrc.sh
  * On 2016-09-24  9:36:12,25
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "CustomType.h"
#include "Structure.h"

namespace MoonGlare {
namespace TypeEditor {

//----------------------------------------------------------------------------------

using MapType = std::unordered_map<std::string, std::shared_ptr<const TypeEditorInfo>>;

MapType *gTypeInfoMap = nullptr;

void TypeEditorInfo::RegisterTypeEditor(std::shared_ptr<const TypeEditorInfo> typeinfo, const std::string &Name) {
	if (!gTypeInfoMap)
		gTypeInfoMap = new MapType();
	(*gTypeInfoMap)[Name].swap(typeinfo);
}

std::shared_ptr<const TypeEditorInfo> TypeEditorInfo::GetEditor(const std::string &Name) {
	if (!gTypeInfoMap)
		return nullptr;
	auto it = gTypeInfoMap->find(Name);
	if (it == gTypeInfoMap->end())
		return nullptr;
	return it->second;
}

//----------------------------------------------------------------------------------

class FloatEditor : public CustomTypeEditor, public QDoubleSpinBox {
public:
	FloatEditor(QWidget *Parent) : QDoubleSpinBox(Parent) { 
		setSingleStep(0.1);
		setMinimum(-1.0e5f);
		setMaximum(1.0e5f);
		setDecimals(5);
	}
	static std::string ToDisplayText(const std::string &in) { return in; }

	static constexpr char *GetTypeName() {
		return "float";
	}

	virtual void SetValue(const std::string &in) {
		auto f = valueFromText(in.c_str());
		QDoubleSpinBox::setValue(f);
	}
	virtual std::string GetValue() {
		auto txt = textFromValue(value());
		return txt.toLocal8Bit().constData();
	}
private:
};

template<class T>
class IntegralEditor : public CustomTypeEditor, public QSpinBox {
public:
	IntegralEditor(QWidget *Parent) : QSpinBox(Parent) {
		setSingleStep(1);
		setMinimum(std::numeric_limits<T>::min());
		setMaximum(std::numeric_limits<T>::max());
	}
	static std::string ToDisplayText(const std::string &in) { return in; }

	virtual void SetValue(const std::string &in) {
		auto l = strtol(in.c_str(), nullptr, 10);
		QSpinBox::setValue(l);
	}
	virtual std::string GetValue() {
		return std::to_string(value());
	}
private:
};

//----------------------------------------------------------------------------------

struct CTRegister {
	CTRegister() {
		RegEditor<FloatEditor>();

		RegInteger< int8_t>("s8");
		RegInteger<uint8_t>("u8");
		RegInteger< int16_t>("s16");
		RegInteger<uint16_t>("u16");
		RegInteger< int32_t>("s32");
		RegInteger<uint32_t>("u32");
		//RegInteger< int64_t>("s64");
		//RegInteger<uint64_t>("u64");
	}

	template<typename TYPE>
	void RegInteger(const char *name) {
		TypeEditorInfo::RegisterTypeEditor(std::make_shared<TemplateTypeEditorInfo<IntegralEditor<TYPE>>>(), name);
	}

	template<typename TYPEINFO>
	void RegEditor() {
		TypeEditorInfo::RegisterTypeEditor(std::make_shared<TemplateTypeEditorInfo<TYPEINFO>>(), TYPEINFO::GetTypeName());
	}
};

static const CTRegister reg;

} //namespace TypeEditor 
} //namespace MoonGlare 
