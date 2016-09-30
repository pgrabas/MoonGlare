/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "EntityEditor.h"

#include <ui_EntityEditor.h>
#include <DockWindowInfo.h>
#include <icons.h>
#include "../Windows/MainWindow.h"
#include <FileSystem.h>

#include "EditableEntity.h"

#include <TypeEditor/x2cDataTree.h>
#include <TypeEditor/Structure.h>
#include <TypeEditor/CustomType.h>

namespace MoonGlare {
namespace Editor {
namespace EntityEditor {

struct EntityEditorInfo 
		: public QtShared::DockWindowInfo
		, public QtShared::iEditorInfo {
	virtual std::shared_ptr<QtShared::DockWindow> CreateInstance(QWidget *parent) override {
		return std::make_shared<EntityEditorWindow>(parent);
	}

	EntityEditorInfo(QWidget *Parent): QtShared::DockWindowInfo(Parent) {
		SetSettingID("EntityEditorInfo");
		SetDisplayName(tr("EntityEditor"));
		SetShortcut("F2");
	}
	std::vector<QtShared::EditableFieleInfo> GetSupportedFileTypes() const override {
		return std::vector<QtShared::EditableFieleInfo>{
			QtShared::EditableFieleInfo{ "xep", ICON_16_ENTITYPATTERN_RESOURCE, },
		};
	}
	virtual std::vector<QtShared::FileCreationMethodInfo> GetCreateFileMethods() const override {
		return std::vector<QtShared::FileCreationMethodInfo> {
			QtShared::FileCreationMethodInfo{ "xep", ICON_16_ENTITYPATTERN_RESOURCE, "Create Entity pattern", "xep", },
		};
	}
};
QtShared::DockWindowClassRgister::Register<EntityEditorInfo> EntityEditorInfoReg("EntityEditor");

//----------------------------------------------------------------------------------

EntityEditorWindow::EntityEditorWindow(QWidget * parent)
	:  QtShared::DockWindow(parent) {
	SetSettingID("EntityEditorWindow");

	m_Ui = std::make_unique<Ui::EntityEditor>();
	m_Ui->setupUi(this);

	connect(m_Ui->pushButton, &QPushButton::clicked, this, &EntityEditorWindow::ShowAddComponentMenu);

	connect(Notifications::Get(), SIGNAL(RefreshView()), SLOT(Refresh()));

	m_EntityModel = std::make_unique<QStandardItemModel>();
	m_EntityModel->setHorizontalHeaderItem(0, new QStandardItem("Entity tree"));
	connect(m_EntityModel.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(EntityChanged(QStandardItem *)));
	m_Ui->treeView->setModel(m_EntityModel.get());
	m_Ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_Ui->treeView, SIGNAL(clicked(const QModelIndex &)), SLOT(EntityClicked(const QModelIndex&)));
	connect(m_Ui->treeView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(EntityContextMenu(const QPoint &)));

	m_ComponentModel = std::make_unique<QStandardItemModel>();
	m_ComponentModel->setHorizontalHeaderItem(0, new QStandardItem("Values tree"));
	m_ComponentModel->setHorizontalHeaderItem(1, new QStandardItem("Values"));
	m_ComponentModel->setHorizontalHeaderItem(2, new QStandardItem("Description"));
	connect(m_ComponentModel.get(), SIGNAL(itemChanged(QStandardItem *)), SLOT(ComponentChanged(QStandardItem *)));
	m_Ui->treeViewDetails->setModel(m_ComponentModel.get());
	m_Ui->treeViewDetails->setSelectionMode(QAbstractItemView::SingleSelection);
	m_Ui->treeViewDetails->setContextMenuPolicy(Qt::CustomContextMenu);
	m_Ui->treeViewDetails->setItemDelegate(new TypeEditor::CustomEditorItemDelegate(this));
	m_Ui->treeViewDetails->setColumnWidth(0, 200);
	m_Ui->treeViewDetails->setColumnWidth(1, 100);
	m_Ui->treeViewDetails->setColumnWidth(2, 100);
	connect(m_Ui->treeViewDetails, SIGNAL(clicked(const QModelIndex &)), SLOT(ComponentClicked(const QModelIndex&)));
	connect(m_Ui->treeViewDetails, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ComponentContextMenu(const QPoint &)));

	m_AddComponentMenu = std::make_unique<QMenu>(this);
	std::unordered_map<std::string, QMenu*> SubMenus;
	for (auto &it : TypeEditor::ComponentInfo::GetComponents()) {
		auto info = it.second;

		QMenu *menu;

		std::string name = info->m_DisplayName;
		auto pos = name.find(".");
		if (pos != std::string::npos) {
			std::string menuname = name.substr(0, pos);
			name = name.substr(pos + 1);

			auto menuit = SubMenus.find(menuname);
			if (menuit == SubMenus.end()) {
				menu = m_AddComponentMenu->addMenu(menuname.c_str());
				SubMenus[menuname] = menu;
			} else {
				menu = menuit->second;
			}
		} else {
			menu = m_AddComponentMenu.get();
		}
			
		menu->addAction(info->m_DisplayName.c_str(), [this, info]() {
			m_CurrentItem.m_EditableEntity->AddComponent(info->m_CID);
			SetModiffiedState(true);
			RefreshDetails();
		});
	}
}

EntityEditorWindow::~EntityEditorWindow() {
	m_Ui.reset();
}

//----------------------------------------------------------------------------------

bool EntityEditorWindow::DoSaveSettings(pugi::xml_node node) const {
	QtShared::DockWindow::DoSaveSettings(node);
	SaveState(node, m_Ui->splitter, "Splitter:State");
//	SaveColumns(node, "treeView:Columns", m_Ui->treeView, 3);
	SaveColumns(node, "treeViewDetails:Columns", m_Ui->treeViewDetails, 3);
	//node.append_child("File").text() = m_CurrentPatternFile.c_str();
	return true;
}

bool EntityEditorWindow::DoLoadSettings(const pugi::xml_node node) {
	QtShared::DockWindow::DoLoadSettings(node);
	LoadState(node, m_Ui->splitter, "Splitter:State");
//	LoadColumns(node, "treeView:Columns", m_Ui->treeView, 3);
	LoadColumns(node, "treeViewDetails:Columns", m_Ui->treeViewDetails, 3);
	//m_CurrentPatternFile = node.child("File").text().as_string("");
	return true;
}

//----------------------------------------------------------------------------------

bool EntityEditorWindow::Create(const std::string &LocationURI, const QtShared::FileCreationMethodInfo& what) {
	QString qname;
	if (!QuerryStringInput("Enter name:", qname))
		return false;

	std::string name = qname.toLocal8Bit().constData();
	std::string URI = LocationURI + name + ".xep";

	auto fs = MainWindow::Get()->GetFilesystem();
	if (!fs->CreateFile(URI)) {
		ErrorMessage("Failed during creating xep file");
		AddLog(Hint, "Failed to create xep: " << m_CurrentPatternFile);
		return false;
	}

	auto root = std::make_unique<EditablePattern>();
	root->GetName() = name;
	m_RootEntity.reset(root.release());
	m_CurrentPatternFile = URI;
	SetModiffiedState(true);
	Refresh();

	AddLog(Hint, "Created xep file: " << URI);

	if (!SaveData()) {
		ErrorMessage("Failed during saving xep file");
		AddLog(Hint, "Failed to save xep: " << m_CurrentPatternFile);
	} else {
		SetModiffiedState(false);
	}

	return true;
}

bool EntityEditorWindow::TryCloseData() {
	AddLog(Hint, "Trying to close xep: " << m_CurrentPatternFile);
	if (m_RootEntity && IsChanged()) {
		if (!AskForPermission("There is a opened pattern. Do you want to close it?"))
			return false;
		if (AskForPermission("Save changes?"))
			SaveData();
	}
	m_EntityModel->removeRows(0, m_EntityModel->rowCount());
	m_ComponentModel->removeRows(0, m_ComponentModel->rowCount());
	m_RootEntity.reset();
	m_CurrentItem = EditableItemInfo();
	Refresh();
	m_CurrentPatternFile.clear();
	return true;
}

bool EntityEditorWindow::OpenData(const std::string &file) {
	TryCloseData();
 
	auto root = std::make_unique<EditablePattern>();
	if (!root->OpenPattern(file)) {
		ErrorMessage("Failure during opening data!");
		AddLog(Hint, "Failed to open xep: " << file);
		return false;
	}
	m_RootEntity.reset(root.release());
	m_CurrentPatternFile = file;
	AddLog(Hint, "Opened xep: " << m_CurrentPatternFile);
	Refresh();
	return true;
}

bool EntityEditorWindow::SaveData() {
	if (!m_RootEntity)
		return false;
	auto pat = dynamic_cast<EditablePattern*>(m_RootEntity.get());
	
	if (pat) {
		if (!pat->SavePattern(m_CurrentPatternFile)) {
			//TODO: sth?
		} else {
			SetModiffiedState(false);
			AddLog(Hint, "Saved xep: " << m_CurrentPatternFile);
			return true;
		}
	}
	//TODO: log sth
	AddLog(Hint, "Failed to save xep: " << m_CurrentPatternFile);
	return true;
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::Refresh() {
	m_EntityModel->removeRows(0, m_EntityModel->rowCount());
	m_CurrentItem = EditableItemInfo();

	if (!m_RootEntity)
		return;

	QStandardItem *root = m_EntityModel->invisibleRootItem();

	std::function<void(EditableEntity*, QStandardItem*)> buidFunc;
	buidFunc = [&buidFunc](EditableEntity *e, QStandardItem *item) {
		QStandardItem *Elem = new QStandardItem(e->GetName().c_str());
		EditableItemInfo eii;
		eii.m_EditableEntity = e;
		eii.m_Item = Elem;
		eii.m_Parent = e->GetParent();
		Elem->setData(QVariant::fromValue(eii), UserRoles::EditableItemInfo);

		QList<QStandardItem*> cols;
		cols << Elem;
		item->appendRow(cols);

		for (auto &child : e->GetChildrenList()) {
			buidFunc(child.get(), Elem);
		}
	};

	buidFunc(m_RootEntity.get(), root);
	m_Ui->treeView->setCurrentIndex(root->index());
	m_Ui->treeView->expandAll();
}

void EntityEditorWindow::RefreshDetails() {
	m_ComponentModel->removeRows(0, m_ComponentModel->rowCount());
	m_CurrentComponent = EditableComponentValueInfo();
	if (!m_CurrentItem)
		return;

	QStandardItem *root = m_ComponentModel->invisibleRootItem();

	for (auto &component : m_CurrentItem.m_EditableEntity->GetComponentList()) {

		QStandardItem *Elem = new QStandardItem(component->GetName().c_str());
		Elem->setFlags(Elem->flags() & ~Qt::ItemIsEditable);

		{
			EditableComponentValueInfo ecvi;
			ecvi.m_OwnerComponent = component.get();
			ecvi.m_Item = Elem;
			ecvi.m_ValueInterface = nullptr;
			ecvi.m_EditableEntity = m_CurrentItem.m_EditableEntity;
			Elem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
		}
		{
			QList<QStandardItem*> cols;
			cols << Elem;
			root->appendRow(cols);
		}

		for (auto &value : component->GetValues()) {
			QStandardItem *CaptionElem = new QStandardItem(value->GetName().c_str());
			CaptionElem->setFlags(CaptionElem->flags() & ~Qt::ItemIsEditable);

			EditableComponentValueInfo ecvi;
			ecvi.m_OwnerComponent = component.get();
			ecvi.m_Item = CaptionElem;
			ecvi.m_ValueInterface = value.get();
			ecvi.m_EditableEntity = m_CurrentItem.m_EditableEntity;

			QStandardItem *ValueElem = new QStandardItem();
			ValueElem->setData(value->GetValue().c_str(), Qt::EditRole);
			auto einfoit = TypeEditor::TypeEditorInfo::GetEditor(ecvi.m_ValueInterface->GetTypeName());
			if (einfoit) {
				ValueElem->setData(einfoit->ToDisplayText(value->GetValue()).c_str(), Qt::DisplayRole);
			} else {
				ValueElem->setData(value->GetValue().c_str(), Qt::DisplayRole);
			}
				
			CaptionElem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
			ValueElem->setData(QVariant::fromValue(ecvi), UserRoles::EditableComponentValueInfo);
			ValueElem->setData(QVariant::fromValue(ecvi.m_ValueInterface), TypeEditor::CustomEditorItemDelegate::QtRoles::StructureValue);

			{
				QList<QStandardItem*> cols;
				cols << CaptionElem;
				cols << ValueElem;
				Elem->appendRow(cols);
			}
		}
	}
	m_Ui->treeViewDetails->collapseAll();
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::EntityClicked(const QModelIndex& index) {
	auto row = index.row();
	auto parent = index.parent();
	auto selectedindex = parent.isValid() ? parent.child(row, 0) : index.sibling(row, 0);

	auto itemptr = m_EntityModel->itemFromIndex(selectedindex);
	if (!itemptr) {
		m_CurrentItem = EditableItemInfo();
	} else {
		m_CurrentItem = itemptr->data(UserRoles::EditableItemInfo).value<EditableItemInfo>();
	}

	RefreshDetails();
}

void EntityEditorWindow::ComponentClicked(const QModelIndex & index) {
	auto row = index.row();
	auto parent = index.parent();
	auto selectedindex = parent.isValid() ? parent.child(row, 0) : index.sibling(row, 0);

	auto itemptr = m_ComponentModel->itemFromIndex(selectedindex);
	if (!itemptr) {
		m_CurrentComponent = EditableComponentValueInfo();
	} else {
		m_CurrentComponent = itemptr->data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
	}
}

void EntityEditorWindow::ComponentChanged(QStandardItem * item) {
	if (!item)
		return;
	//EditableComponentValueInfo info = item->data(UserRoles::EditableComponentValueInfo).value<EditableComponentValueInfo>();
	//if (!info)
	//	return;
	//std::string value = item->data(Qt::DisplayRole).toString().toLocal8Bit().constData();
	//info.m_ValueInterface->SetValue(value);
	SetModiffiedState(true);
}

void EntityEditorWindow::EntityChanged(QStandardItem * item) {
	if (!item)
		return;

	EditableItemInfo info = item->data(UserRoles::EditableItemInfo).value<EditableItemInfo>();
	if (!info)
		return;

	auto value = item->data(Qt::DisplayRole).toString().toLocal8Bit().constData();
	info.m_EditableEntity->GetName() = value;
	SetModiffiedState(true);
}

void EntityEditorWindow::ComponentContextMenu(const QPoint & pos) {
	if (!m_CurrentComponent)
		return;

	QMenu menu(this);
	auto ComponentInfo = m_CurrentComponent;

	if (ComponentInfo.m_ValueInterface) {
		return;
	}

	menu.addAction("Move up", [this, ComponentInfo]() {
		ComponentInfo.m_EditableEntity->MoveUp(ComponentInfo.m_OwnerComponent);
		SetModiffiedState(true);
		RefreshDetails();
	});
	menu.addAction("Move down", [this, ComponentInfo]() {
		ComponentInfo.m_EditableEntity->MoveDown(ComponentInfo.m_OwnerComponent);
		SetModiffiedState(true);
		RefreshDetails();
	});
	menu.addSeparator();
	menu.addAction("Delete component", [this, ComponentInfo]() {
		ComponentInfo.m_EditableEntity->DeleteComponent(ComponentInfo.m_OwnerComponent);
		SetModiffiedState(true);
		RefreshDetails();
	});

	menu.exec(QCursor::pos());
}

void EntityEditorWindow::EntityContextMenu(const QPoint &pos) {
	if (!m_CurrentItem)
		return;

	QMenu menu(this);
	auto EntityInfo = m_CurrentItem;

	if (EntityInfo.m_Parent) {
		menu.addAction("Move up", [this, EntityInfo]() {
			EntityInfo.m_Parent->MoveUp(EntityInfo.m_EditableEntity);
			SetModiffiedState(true);
			Refresh();
		});
		menu.addAction("Move Down", [this, EntityInfo]() {
			EntityInfo.m_Parent->MoveDown(EntityInfo.m_EditableEntity);
			SetModiffiedState(true);
			Refresh();
		});
		menu.addSeparator();
	}

	if (EntityInfo.m_Parent) {
		menu.addAction("Delete child", [this, EntityInfo]() {
			if (!AskForPermission())
				return;
			EntityInfo.m_Parent->DeleteChild(EntityInfo.m_EditableEntity);
			SetModiffiedState(true);
			Refresh();
		});
	}

	menu.addAction("Add child", [this, EntityInfo]() {
		EntityInfo.m_EditableEntity->AddChild();
		SetModiffiedState(true);
		Refresh();
	});
	menu.addSeparator();

	menu.exec(QCursor::pos());
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::ShowAddComponentMenu() {
	if (!m_CurrentItem)
		return;

	m_AddComponentMenu->exec(QCursor::pos());
}

//----------------------------------------------------------------------------------

void EntityEditorWindow::ProjectChanged(Module::SharedDataModule datamod) {
//	m_Module = datamod;
}

} //namespace EntityEditor 
} //namespace Editor 
} //namespace MoonGlare 
