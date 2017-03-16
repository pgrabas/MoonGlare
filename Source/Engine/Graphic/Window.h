/*
	Generated by cppsrc.sh
	On 2014-12-13 13:22:00,37
	by Paweu
*/

#pragma once
#ifndef Window_H
#define Window_H

namespace Graphic {

class Window : public cRootClass {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(Window, cRootClass);
	DECLARE_EXCACT_SCRIPT_CLASS_GETTER();
public:
	Window(GLFWwindow *w);
	unsigned GetRefreshRate() const;
	const uvec2& Size() const { return m_Size; }
private:
	GLFWwindow *m_Window;
	uvec2 m_Size;
};

} //namespace Graphic 

#endif
