/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef OpenGLCommands_H
#define OpenGLCommands_H

namespace MoonGlare {
namespace Renderer {
namespace Devices {
namespace OpenGL {
namespace Commands {

enum {
	BytesPerCommand = 16,
};

using CommandFunction = void(*)(void*);

struct OpenGLCommandInterface {

	template<class T>
	static CommandFunction Functor() { return (CommandFunction)&T::Call; }
};

using CommandBuffer = Generic::CommandBuffer<BytesPerCommand, CommandFunction, OpenGLCommandInterface, ::Utils::Memory::NoLockPolicy>;

//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

struct NewTextureCommand {
	GLsizei m_Count;
	GLuint* m_Memory;

	static void Call(NewTextureCommand* data);
};

struct DeleteTextureCommand {
	GLsizei m_Count;
	GLuint* m_Memory;

	static void Call(DeleteTextureCommand* data);
};

} //namespace Commandss
} //namespace OpenGL
} //namespace Devices 
} //namespace Renderer 
} //namespace MoonGlare 

#endif
