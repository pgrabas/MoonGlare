/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include PCH_HEADER
#include "mgdtSettings.h"

//-----------------------------------------

mgdtSettings* mgdtSettings::_Instance = nullptr;

mgdtSettings::mgdtSettings() {
	_Instance = this;
	m_FileName = "MGInsider.Settings.xml";
}

mgdtSettings::~mgdtSettings() {
}

mgdtSettings& mgdtSettings::get() {
	return *_Instance;
}

//-----------------------------------------

#if 0
void mgdtSettings::Window_t::WindowPosition_t::Apply(QWidget *window) {
	assert(window);
	QRect pos(window->geometry());
	bool changed = false;
	if (GetSettings().Window.RememberPosition) {
		int a = PosX;
		if (a > 0) pos.setX(a);
		int b = PosY;
		if (b > 0) pos.setY(b);
		if (a > 0 || b > 0)
			changed = true;
	}
	if (GetSettings().Window.RememberSize) {
		int a = Width;
		if (a > 0) pos.setWidth(a);
		int b = Height;
		if (b > 0) pos.setHeight(b);
		if (a > 0 || b > 0)
			changed = true;
	}
	if (changed)
		window->setGeometry(pos);
}

void mgdtSettings::Window_t::WindowPosition_t::Store(QWidget *window) {
	assert(window);
	QRect pos(window->geometry());
	PosX = pos.x();
	PosY = pos.y();
	Width = pos.width();
	Height = pos.height();
}
#endif

//-----------------------------------------