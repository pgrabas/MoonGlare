/*
  * Generated by cppsrc.sh
  * On 2016-07-22  0:08:40,44
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER
#include "OutputDockTab.h"

#include <ui_OutputDockTab.h>
#include <DockWindowInfo.h>
#include "OutputDock.h"
#include <icons.h>

#include <ToolBase/Module.h>

#include <fmt/format.h>

namespace MoonGlare {
namespace Editor {
namespace DockWindows {

//----------------------------------------------------------------------------------

OutputTabWidget::OutputTabWidget(OutputDock * parent) : QWidget(parent) {
    m_Ui = std::make_unique<Ui::OutputDockTab>();
    m_Ui->setupUi(this);
    owner = parent;
}

OutputTabWidget::~OutputTabWidget() {
    owner->OnTabDestroy(this);
    m_Ui.reset();
}

//----------------------------------------------------------------------------------

void OutputTabWidget::PushLine(const std::string &line) {
    owner->UIUpdate([this, line]() {
        m_Ui->plainTextEdit->appendPlainText(line.c_str());
    });
}


} //namespace DockWindows 
} //namespace Editor 
} //namespace MoonGlare 
