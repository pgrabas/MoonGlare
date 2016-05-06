/*
  * Generated by cppsrc.sh
  * On 2015-03-01  7:44:40,30
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include "GUI.h"
#include "Widgets/Image.h"

namespace MoonGlare {
namespace GUI {

SPACERTTI_IMPLEMENT_STATIC_CLASS(Mouse);
RegisterApiDerivedClass(Mouse, &Mouse::RegisterScriptApi);

Mouse::Mouse(GUIEngine *Owner):	
		BaseClass(),
		m_Owner(Owner) {
}

Mouse::~Mouse() {
}

//----------------------------------------------------------------

void Mouse::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cGUIMouse")
		.addFunction("SetCursorImage", &ThisClass::SetCursorImage)
	.endClass();
}

//----------------------------------------------------------------

bool Mouse::Initialize() {
	m_Screen = m_Owner->GetRootWidget();
	m_PositionLimit = m_Screen->GetBoundingRect();
	m_Position = m_PositionLimit.RightBottom / 2.0f;
	m_PositionLimit.RightBottom -= Point(1);

	m_Image = new Widgets::Image(m_Screen);
	m_Screen->AddWidget(m_Image);

	m_Image->SetAlignMode(AlignMode::None);
	m_Image->SetName("__CursorImage");
	m_Image->SetPosition(m_Position);
	m_Image->SetImage("Cursor");
	m_Image->SetVisible(true);
	m_Image->SetMetaObject(true);
	return true;
}

bool Mouse::Finalize() {
	auto screen = m_Owner->GetRootWidget();
	if (m_Image)
		screen->RemoveWidget(m_Image);
	//m_Image.reset();
	return true;
}

//----------------------------------------------------------------

void Mouse::Process(const Core::MoveConfig &conf) {
	auto delta = ::Core::GetInput()->GetMouseDelta();

	bool Moved = delta[0] != 0 || delta[1] != 0;
	if (Moved) {
		m_Position += delta;
		if (m_Position[0] < m_PositionLimit.LeftTop[0]) 
			m_Position[0] = m_PositionLimit.LeftTop[0];
		else
			if (m_Position[0] > m_PositionLimit.RightBottom[0]) 
				m_Position[0] = m_PositionLimit.RightBottom[0];

		if (m_Position[1] < m_PositionLimit.LeftTop[1]) 
			m_Position[1] = m_PositionLimit.LeftTop[1];
		else		   
			if (m_Position[1] > m_PositionLimit.RightBottom[1]) 
				m_Position[1] = m_PositionLimit.RightBottom[1];

		m_Image->SetPosition(m_Position);
		
		iWidget *w = m_Screen->GetWidgetAt(m_Position);

		if (w != m_HoveredWidget) {
			//AddLog(Hint, "hwc: " << m_HoveredWidget << " -> " << w << "  at:" << m_Position);
			SetHovered(w);
		}

		if (m_HoveredWidget) {
			Events::MouseMoveEvent ev;
			ev.MousePosition = m_Position;
			ev.MouseDelta = delta;
			m_HoveredWidget->OnMouseMove(ev); 
		}
	}

	m_Image->Process(conf);
}

void Mouse::Draw(Graphic::MatrixStack &dev) {
	if (m_Image)
		m_Image->Draw(dev);
}

//----------------------------------------------------------------

void Mouse::SetHovered(iWidget *Widget) {
	if (m_HoveredWidget) {
		Events::MouseLeaveEvent ev;
		ev.MousePosition = m_Position;
		m_HoveredWidget->OnMouseLeave(ev);
	}

	if (Widget && !Widget->IsEnabled()) {
		m_HoveredWidget = nullptr;
		return;
	}

	m_HoveredWidget = Widget;
	if (m_HoveredWidget) {
		Events::MouseEnterEvent ev;
		ev.MousePosition = m_Position;
		m_HoveredWidget->OnMouseEnter(ev); 
	}
}

void Mouse::SetCursorImage(const string& Name) {
	if (m_Image)
		m_Image->SetImage(Name);
}

//----------------------------------------------------------------

void Mouse::OnMouseDown(Events::MouseDownEvent &event) { 
	if (!m_HoveredWidget)
		return;
	event.MousePosition = m_Position - m_HoveredWidget->GetScreenPosition();
	m_HoveredWidget->OnMouseDown(event); 
}

void Mouse::OnMouseUp(Events::MouseUpEvent &event) { 
	if (!m_HoveredWidget)
		return;
	event.MousePosition = m_Position - m_HoveredWidget->GetScreenPosition();
	m_HoveredWidget->OnMouseUp(event); 
}

} //namespace GUI 
} //namespace MoonGlare 
