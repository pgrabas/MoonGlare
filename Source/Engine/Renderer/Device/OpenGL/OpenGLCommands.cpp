/*
  * Generated by cppsrc.sh
  * On 
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "../../Generic/nfGeneric.h"
#include "nfOpenGL.h"
#include "nOpenGL.h"
#include "OpenGLCommands.h"

namespace MoonGlare {
namespace Renderer {
namespace Devices {
namespace OpenGL {
namespace Commands {

//---------------------------------------------------------------------------------------

void NewTextureCommand::Call(NewTextureCommand* data) {
	glGenTextures(data->m_Count, data->m_Memory);
}

void DeleteTextureCommand::Call(DeleteTextureCommand* data) {
	glDeleteTextures(data->m_Count, data->m_Memory);
}

} //namespace Commands
} //namespace OpenGL
} //namespace Devices 
} //namespace Renderer 
} //namespace MoonGlare 
