/*
  * Generated by cppsrc.sh
  * On 2016-09-10 16:34:37,71
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef EditableEntity_H
#define EditableEntity_H

#include <Source/Engine/Core/Component/nfComponent.h>

#include <TypeEditor/x2cDataTree.h>
#include <TypeEditor/Structure.h>
#include <TypeEditor/ComponentInfo.h>

namespace MoonGlare {
namespace Editor {
namespace EntityEditor {

class EditableEntity;
using UniqueEditableEntity = std::unique_ptr<EditableEntity>;
class EditableComponent;
using UniqueEditableComponent = std::unique_ptr<EditableComponent>;

using EditableComponentList = std::vector<UniqueEditableComponent>;
using EditableEntityList = std::vector<UniqueEditableEntity>;

class EditableEntity {
public:
 	EditableEntity(EditableEntity *Parent);
 	virtual ~EditableEntity();

	virtual bool Read(pugi::xml_node node);
	virtual bool Write(pugi::xml_node node);

	std::string& GetName() { return m_Name; }
	EditableEntityList& GetChildrenList() { return m_Children; }
	EditableComponentList& GetComponentList() { return m_Components; }

	EditableEntity* GetParent() const { return m_Parent; }

	void MoveUp(EditableComponent *c);
	void MoveDown(EditableComponent *c);
	void MoveUp(EditableEntity *c);
	void MoveDown(EditableEntity *c);

	int Find(EditableComponent *c) {
		for (int i = 0; i < (int)m_Components.size(); ++i)
			if (m_Components[i].get() == c)
				return i;
		return -1;
	}
	int Find(EditableEntity *c) {
		for (int i = 0; i < (int)m_Children.size(); ++i)
			if (m_Children[i].get() == c)
				return i;
		return -1;
	}

	EditableEntity* AddChild();
	void DeleteChild(EditableEntity *c);
	EditableComponent* AddComponent(Core::ComponentID cid);
	void DeleteComponent(EditableComponent *c);
protected:
	EditableEntity *m_Parent;
	std::string m_Name;
	EditableEntityList m_Children;
	EditableComponentList m_Components;
};

class EditablePattern 
	: public EditableEntity {
public:
	EditablePattern();

	bool OpenPattern(const std::string &filename);
	bool SavePattern(const std::string &filename);
};

class EditableChild
	: public EditableEntity {
public:
	EditableChild(EditableEntity *Parent) : EditableEntity(Parent) { }
};

//----------------------------------------------------------------------------------

class EditableComponent {
public:
	EditableComponent::EditableComponent(EditableEntity *Parent, TypeEditor::SharedComponentInfo cInfo, TypeEditor::UniqueStructure x2cs)
		: m_ComponentInfo(cInfo) {
		m_Parent = Parent;
		m_Data.swap(x2cs);
		m_Data->SetName(m_ComponentInfo->m_Name);
	}
	~EditableComponent() { }

	const TypeEditor::StructureValueList& GetValues() { return m_Data->GetValues(); }
	bool Read(pugi::xml_node node) { return m_Data->Read(node); }
	bool Write(pugi::xml_node node) {  return m_Data->Write(node); }
	const std::string& GetName() { return m_Data->GetName(); }

	static UniqueEditableComponent CreateComponent(EditableEntity *Parent, pugi::xml_node node);
	static UniqueEditableComponent CreateComponent(EditableEntity *Parent, MoonGlare::Core::ComponentID cid);
protected:
	EditableEntity *m_Parent;
	TypeEditor::SharedComponentInfo m_ComponentInfo;
	TypeEditor::UniqueStructure m_Data;
};

} //namespace EntityEditor 
} //namespace Editor 
} //namespace MoonGlare 

#endif
