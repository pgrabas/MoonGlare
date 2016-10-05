/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef EntityEditorModel_H
#define EntityEditorModel_H

#include <iSettingsUser.h>
#include <iEditor.h>
#include <ChangesManager.h>
#include <qtUtils.h>

#include <TypeEditor/Structure.h>
#include <TypeEditor/CustomType.h>
#include <TypeEditor/ComponentInfo.h>

#include <Source/Engine/Core/Component/nfComponent.h>

namespace Ui { class EntityEditorModel; }

namespace MoonGlare {
namespace QtShared {
namespace DataModels {

class EditableEntity;
using UniqueEditableEntity = std::unique_ptr<EditableEntity>;
class EditableComponent;
using UniqueEditableComponent = std::unique_ptr<EditableComponent>;

using EditableComponentList = std::vector<UniqueEditableComponent>;
using EditableEntityList = std::vector<UniqueEditableEntity>;

class EditableEntity {
public:
	EditableEntity(EditableEntity *Parent = nullptr);
	virtual ~EditableEntity();

	virtual bool Read(pugi::xml_node node, const char *NodeName = nullptr);
	virtual bool Write(pugi::xml_node node, const char *NodeName = nullptr);

	std::string& GetName() { return m_Name; }
	const std::string& GetPatternURI() { return m_PatternURI; }
	void SetPatternURI(std::string v) { m_PatternURI.swap(v); }
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

	void Clear();
protected:
	EditableEntity *m_Parent;
	std::string m_Name;
	std::string m_PatternURI;
	EditableEntityList m_Children;
	EditableComponentList m_Components;
};

class EditableComponent {
public:
	EditableComponent::EditableComponent(EditableEntity *Parent, TypeEditor::SharedComponentInfo cInfo, TypeEditor::UniqueStructure x2cs)
		: m_ComponentInfo(cInfo) {
		m_Parent = Parent;
		m_Data.swap(x2cs);
		m_Data->SetName(m_ComponentInfo->m_Name);
	}
	~EditableComponent() {}

	const TypeEditor::StructureValueList& GetValues() { return m_Data->GetValues(); }
	bool Read(pugi::xml_node node) { return m_Data->Read(node); }
	bool Write(pugi::xml_node node) { return m_Data->Write(node); }
	const std::string& GetName() { return m_Data->GetName(); }

	static UniqueEditableComponent CreateComponent(EditableEntity *Parent, pugi::xml_node node);
	static UniqueEditableComponent CreateComponent(EditableEntity *Parent, MoonGlare::Core::ComponentID cid);
protected:
	EditableEntity *m_Parent;
	TypeEditor::SharedComponentInfo m_ComponentInfo;
	TypeEditor::UniqueStructure m_Data;
};

class EntityEditorModel
		: public QWidget 
		, public QtShared::UserQuestions
		, public QtShared::iChangeContainer
		, public QtShared::iSettingsUser
{
	Q_OBJECT;
public:
	EntityEditorModel(QWidget *parent = nullptr);
 	virtual ~EntityEditorModel();

	struct UserRoles {
		enum {
			EditableItemInfo = Qt::UserRole + 1,
			EditableComponentValueInfo,
		};
	};

	struct EditableComponentValueInfo {
		EditableComponent *m_OwnerComponent = nullptr;
		TypeEditor::StructureValue *m_ValueInterface = nullptr;
		EditableEntity *m_EditableEntity = nullptr;
		QStandardItem *m_Item = nullptr;

		operator bool() const { return m_Item && m_OwnerComponent; }
	};

	struct EditableItemInfo {
		QStandardItem *m_Item = nullptr;
		EditableEntity *m_EditableEntity = nullptr;
		EditableEntity *m_Parent = nullptr;
		bool m_PatternURIMode = false;

		operator bool() const { return m_Item && m_EditableEntity; }
	};

	void SetEntity(std::unique_ptr<EditableEntity> NewRoot) {
		m_RootEntity.swap(NewRoot);
		Refresh();
	}

	virtual bool DoSaveSettings(pugi::xml_node node) const override;
	virtual bool DoLoadSettings(const pugi::xml_node node) override;
public slots:
	void Refresh();
	void RefreshDetails();
protected slots:
	void EntityClicked(const QModelIndex& index);
	void ComponentClicked(const QModelIndex& index);
	void ComponentChanged(QStandardItem *item);
	void EntityChanged(QStandardItem *item);
	void ComponentContextMenu(const QPoint &);
	void EntityContextMenu(const QPoint &);
	
	void ShowAddComponentMenu();
	//void ProjectChanged(Module::SharedDataModule datamod);
private: 
	std::unique_ptr<Ui::EntityEditorModel> m_Ui;
	std::unique_ptr<QStandardItemModel> m_EntityModel;
	std::unique_ptr<QStandardItemModel> m_ComponentModel;
	std::unique_ptr<EditableEntity> m_RootEntity;
	std::unique_ptr<QMenu> m_AddComponentMenu;
	//std::string m_CurrentPatternFile;
	EditableItemInfo m_CurrentItem;
	EditableComponentValueInfo m_CurrentComponent;
};

} //namespace DataModels
} //namespace QtShared
} //namespace MoonGlare

Q_DECLARE_METATYPE(MoonGlare::QtShared::DataModels::EntityEditorModel::EditableItemInfo);
Q_DECLARE_METATYPE(MoonGlare::QtShared::DataModels::EntityEditorModel::EditableComponentValueInfo);

#endif
