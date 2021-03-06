#pragma once	

#include <Config/pch_common.h>

#pragma warning (disable: 4251)

#pragma warning (push, 0)

#include <qapplication.h>
#include <qobject.h>
#include <QtWidgets/QMainWindow>
#include <qdialog.h>
#include <qtextdocumentwriter.h>
#include <qfiledialog.h>
#include <QPainter>
#include <QPlainTextEdit>
#include <qfileinfo.h>
#include <QTabWidget.h>
#include <qboxlayout.h>
#include <QtWidgets/QDockWidget>
#include <QTextBlock>
#include <QStandardItemModel>
#include <qmessagebox.h>
#include <qmenu.h>
#include <QVariant>
#include <qtimer.h>
#include <qfont.h>
#include <qlist.h>
#include <qevent.h>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegExp>
#include <QVector>
#include <qudpsocket.h>
#include <qinputdialog.h>
#include <qclipboard.h>
#include <QThread>
#include <qdockwidget.h>
#include <QStyledItemDelegate>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QInputDialog>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#pragma warning ( pop )

#include <OrbitLogger/src/OrbitLogger.h>

#include <cassert>

#include <pugixml-1.6/src/pugixml.hpp>
#include <luaJiT-2.0.4/lua.hpp>

#include <source/Utils/XMLUtils.h>
#include <source/Utils/SetGet.h>
#include "Utils/Memory/nMemory.h"
#include "Utils/Memory.h"
#include <source/Utils/LuaUtils.h>

#include <libSpace/src/Utils/EnumConverter.h>
#include <libSpace/src/RTTI/RTTI.h>
#include <libSpace/src/Utils/DynamicClassRegister.h>
#include <libSpace/src/Memory/Handle.h>
#include <libSpace/src/Utils/CompileMurmurhash3.h>
using namespace Space::Utils::HashLiterals;

#include <Engine/Configuration.h>

namespace MoonGlare { }
using namespace MoonGlare;

Q_DECLARE_METATYPE(MoonGlare::Handle);
