#pragma once

#include <ToolBase/Module.h>
#include "../iCustomEnum.h"

namespace MoonGlare::TypeEditor {

struct CustomEditorItemDelegate : public QStyledItemDelegate {
    struct QtRoles {
        enum {
            StructureValue = Qt::UserRole + 1000,
        };
    };

    CustomEditorItemDelegate(SharedModuleManager moduleManager, QWidget *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
private:
    std::weak_ptr<QtShared::CustomEnumProvider> customEnumProvider;
};

} //namespace MoonGlare::TypeEditor
