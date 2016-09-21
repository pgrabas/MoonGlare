/*
  * Generated by cppsrc.sh
  * On 2015-03-01  9:09:00,10
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "GUI.h"
#include <Engine/Graphic/nGraphic.h>

namespace MoonGlare {
namespace GUI {

SPACERTTI_IMPLEMENT_STATIC_CLASS(Screen);
RegisterApiDerivedClass(Screen, &Screen::RegisterScriptApi);

Screen::Screen(GUIEngine *Owner, Graphic::Window *Window):
		BaseClass(this),
		m_Window(Window) {
	m_Screen = this;
	m_GUIEngine = Owner;

	m_Camera.SetDefaultOrthogonal(math::fvec2(Window->Size()));

	m_AlignMode = AlignMode::FillParent; //hue hue :)
	SetName("Screen");

	m_Position = Point(0, 0);
	m_Size = Point(Window->Size());
	m_BoundingRect.LeftTop = m_Position;
	m_BoundingRect.RightBottom = m_Size;
	//m_Margin.Set(10);
}

Screen::~Screen() {
}

//----------------------------------------------------------------------------------

void Screen::Process(const Core::MoveConfig &conf) {
	for (auto &it : m_Children)
		it->Process(conf);
}

void Screen::Draw(Graphic::MatrixStack &dev){
	dev.Device.Bind(&m_Camera);
	for (auto &it : m_Children)
		it->Draw(dev);
}

//----------------------------------------------------------------------------------

void Screen::Lock() {
	BaseClass::Lock();
	for (auto &it : m_Children) it->Lock();
}

void Screen::Unlock() {
	BaseClass::Unlock();
	for (auto &it : m_Children) it->Unlock();
}	

//----------------------------------------------------------------------------------

void Screen::RemoveWidget(iWidget *widget) {
	m_Children.Remove(widget);
	if (!IsLocked())
		RecalculateMetrics();
}

void Screen::AddWidget(Widget &widget) {
	widget->SetParent(this);
	m_Children.Add(widget);
	if (!IsLocked())
		RecalculateMetrics();
}

void Screen::CleanWidgets() {
	Lock();
	m_Children.Clean();
	if (!IsLocked())
		RecalculateMetrics();
	Unlock();
}

iWidget* Screen::FindWidget(const string& name) const {
	if (GetName() == name)
		return const_cast<ThisClass*>(this);

	for (auto &it : m_Children) {
		auto ptr = it->FindWidget(name);
		if (ptr) return ptr;
	}

	return nullptr;
}

//----------------------------------------------------------------------------------

void Screen::ScriptSet(Utils::Scripts::TableDispatcher &table) {
	//BaseClass::ScriptSet(table);
	Lock();
	m_Children.Clean();
	try {
		//auto td = table.GetTable("Children");
		m_Children.CreateChildren(table, this);
	}
	catch (...) { /* ignore */ }
	Unlock();
	RecalculateMetrics();
}

//----------------------------------------------------------------------------------

void Screen::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("ScreenWidget")
	.endClass()
	;
}

//----------------------------------------------------------------------------------

void Screen::RecalculateMetrics() {
	for (auto &it : m_Children)
		it->RecalculateMetrics();
}

//----------------------------------------------------------------------------------

void Screen::SetStyle(SharedStyleSet style) {
	BaseClass::SetStyle(style);
	for (auto &it : m_Children)
		it->SetStyle(style);
}

void Screen::SetPosition(const Point &pos) {
	AddLog(Debug, "Attempt to set screen position!");
}

void Screen::SetSize(const Point &size) {
	AddLog(Debug, "Attempt to set screen size!");
}

void Screen::SetMargin(const Margin &margin) {
	AddLog(Debug, "Attempt to set screen margin!");
}

void Screen::SetAlignMode(AlignMode Mode) {
	AddLog(Debug, "Attempt to set screen align mode!");
}

void Screen::SetMetrics(const Point &pos, const Point &size) {
	AddLog(Debug, "Attempt to set screen metrics!");
}

void Screen::SetMetrics(const Point &pos, const Point &size, const Margin& margin) {
	AddLog(Debug, "Attempt to set screen metrics!");
}

//----------------------------------------------------------------------------------

iWidget* Screen::GetWidgetAt(const Point &p) const {
	if (!m_BoundingRect.IsPointInside(p))
		return nullptr;
	
	for (auto &it : m_Children) {
		auto *w = it->GetWidgetAt(p);
		if (w)
			return w;
	}

	return nullptr;
}

} //namespace GUI 
} //namespace MoonGlare 
