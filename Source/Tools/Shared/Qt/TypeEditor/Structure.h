/*
  * Generated by cppsrc.sh
  * On 2016-09-24  9:36:31,89
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef Structure_H
#define Structure_H

namespace MoonGlare {
namespace TypeEditor {

class X2CEditor;
class Structure;

class StructureValue {
public:
	virtual ~StructureValue() {}

	virtual const std::string& GetName() = 0;
	virtual std::string GetValue() = 0;
	virtual void SetValue(const std::string& v) = 0;
	virtual const std::string& GetDescription() = 0;
	virtual const std::string& GetTypeName() = 0;
};

using UniqueStructureValue = std::unique_ptr<StructureValue>;
using StructureValueList = std::vector<UniqueStructureValue>;

using UniqueStructure = std::unique_ptr<Structure>;

struct StructureInfo {
	std::string m_Name;
	std::string m_DisplayName;
	UniqueStructure(*m_CreateFunc)(X2CEditor *Editor, Structure *Parent);
};

using SharedStructureInfo = std::shared_ptr<const StructureInfo>;

class Structure {
public:
	Structure(X2CEditor *Editor, Structure *Parent) : m_Editor(Editor), m_Parent(Parent) { }
	virtual ~Structure() {}

	virtual const StructureValueList& GetValues() = 0;
	virtual bool Read(pugi::xml_node node) = 0;
	virtual bool Write(pugi::xml_node node) = 0;
	virtual void ResetToDefault() = 0;

	virtual const std::string& GetName() = 0;
	virtual void SetName(const std::string& value) = 0;
	
	virtual void SetExternlDataSink(void *src) = 0;

	template<class STRUCT>
	static UniqueStructure X2CSCreateFunc(X2CEditor *Editor, Structure *Parent) {
		return std::make_unique<StructureTemplate<STRUCT>>(Editor, Parent);
	}
	template<typename STRUCT>
	static SharedStructureInfo MakeX2CStructureInfo() {
		StructureInfo si;
		si.m_DisplayName = si.m_Name = STRUCT::GetTypeName();
		si.m_CreateFunc = &X2CSCreateFunc<STRUCT>;
		return std::make_shared<StructureInfo>(std::move(si));
	}
	template<typename STRUCT> static void RegisterX2CStructure() { RegisterStructureInfo(MakeX2CStructureInfo<STRUCT>()); }

	static void RegisterStructureInfo(SharedStructureInfo typeinfo);
	static SharedStructureInfo GetStructureInfo(const std::string &Name);
protected:
	X2CEditor *m_Editor;
	Structure *m_Parent;
};

#ifdef _X2C_IMPLEMENTATION_
template<typename X2CLASS>
class StructureTemplate : public Structure {
public:
	struct EditableValue : public StructureValue {
		virtual const std::string& GetName() override { return m_Name; }
		virtual std::string GetValue() override {
			std::string v;
			m_Read(v);
			return std::move(v);
		}
		virtual void SetValue(const std::string& v) override { m_Write(v); }
		virtual const std::string& GetDescription() override { return m_Description; }
		virtual const std::string& GetTypeName() override { return m_Type; }

		std::function<void(std::string &output)> m_Read;
		std::function<void(const std::string &input)> m_Write;
		std::string m_Name;
		std::string m_Description;
		std::string m_Type;
	};

	StructureTemplate(X2CEditor *Editor = nullptr, Structure *Parent = nullptr) : Structure(Editor, Parent) {
		m_TypeName = X2CLASS::GetTypeName();
		m_LocalValue = std::make_unique<X2CLASS>();
		m_Value = m_LocalValue.get();
		m_Value->ResetToDefault();

		::x2c::cxxpugi::StructureMemberInfoTable members;
		std::unordered_map<std::string, std::function<void(X2CLASS &self, const std::string &input)>> WriteFuncs;
		std::unordered_map<std::string, std::function<void(const X2CLASS &self, std::string &output)>> ReadFuncs;

		m_Value->GetMemberInfo(members);
		m_Value->GetWriteFuncs(WriteFuncs);
		m_Value->GetReadFuncs(ReadFuncs);
		m_Values.reserve(members.size());
		for (auto &member : members) {
			auto ptr = std::make_unique<EditableValue>();
			ptr->m_Name = member.m_Name;
			ptr->m_Description = member.m_Description;
			ptr->m_Type = member.m_TypeName;

			auto read = ReadFuncs[member.m_Name];
			auto write = WriteFuncs[member.m_Name];

			ptr->m_Read = [this, read](std::string &out) {
				read(*this->m_Value, out);
			};
			ptr->m_Write = [this, write](const std::string &out) {
				write(*this->m_Value, out);
			};
			m_Values.emplace_back(UniqueStructureValue(ptr.release()));
		}
	}
	~StructureTemplate() {}

	virtual const StructureValueList& GetValues() override  { return m_Values; }
	virtual bool Read(pugi::xml_node node) override  {
		return m_Value->Read(node);
	}
	virtual bool Write(pugi::xml_node node) override {
		return m_Value->Write(node);
	}
	virtual const std::string& GetName() override {
		return m_TypeName;
	}
	virtual void SetName(const std::string& value) override {
		m_TypeName = value;
	}
	virtual void SetExternlDataSink(void *src) override {
		if (!src)
			m_Value = m_LocalValue.get();
		else
			m_Value = (X2CLASS*)src;
	}
	virtual void ResetToDefault() override {
		m_Value->ResetToDefault();
	}
private:
	StructureValueList m_Values;
	X2CLASS *m_Value;
	std::unique_ptr<X2CLASS> m_LocalValue;
	std::string m_TypeName;
};
#endif


} //namespace TypeEditor 
} //namespace MoonGlare 

#endif