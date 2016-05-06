/*
	Generated by cppsrc.sh
	On 2015-01-17 23:03:11,97
	by Paweu
*/

#pragma once
#ifndef PointLightShader_H
#define PointLightShader_H

namespace Graphic {
namespace Dereferred {

class PointLightShader : public LightingPassShader {
	SPACERTTI_DECLARE_CLASS_NOCREATOR(PointLightShader, LightingPassShader);
public:
 	PointLightShader(GLuint ShaderProgram, const string &ProgramName);
 	virtual ~PointLightShader();

	void Bind(const Light::PointLight &light) const {
		BaseClass::Bind(light);
		glUniform3fv(m_PositionLocation, 1, &light.Position[0]); 
		glUniform1f(m_AttenuationLinearLocation, light.Attenuation.Linear);
		glUniform1f(m_AttenuationExpLocation, light.Attenuation.Exp);
		glUniform1f(m_AttenuationConstantLocation, light.Attenuation.Constant);
		glUniform1f(m_AttenuationMinThresholdLocation, light.Attenuation.MinThreshold);
	}
protected:
	GLuint m_PositionLocation;
	GLuint m_AttenuationLinearLocation;
	GLuint m_AttenuationExpLocation;
	GLuint m_AttenuationConstantLocation;
	GLuint m_AttenuationMinThresholdLocation;
};

} //namespace Dereferred 
} //namespace Graphic 

#endif
