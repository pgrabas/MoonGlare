/*
  * Generated by cppsrc.sh
  * On 2015-03-07 23:38:28,72
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include "../Graphic.h"

namespace Graphic {
namespace Shaders {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(D2Shader);
ShaderClassRegister::Register<D2Shader> D2ShaderReg;

D2Shader::D2Shader(GLuint ShaderProgram, const string &ProgramName):
		BaseClass(ShaderProgram, ProgramName)  {
}

D2Shader::~D2Shader() {
}

} //namespace Shaders 
} //namespace Graphic 
