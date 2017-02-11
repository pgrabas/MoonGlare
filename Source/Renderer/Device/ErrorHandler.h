/*
* Generated by cppsrc.sh
* On 2017-02-10 21:54:06,96
* by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

namespace MoonGlare::Renderer::Device {

struct ErrorHandler {
	static void RegisterErrorCallback();

	static void CheckError() {
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			auto ptr = (char*) gluErrorString(err);
			if (ptr)
				AddLog(Error, "OpenGL error: " << ptr);
		}
	}
};

} //namespace MoonGlare::Renderer::Device
