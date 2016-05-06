/*
	Generated by cppsrc.sh
	On 2015-01-14 22:23:46,68
	by Paweu
*/

#pragma once
#ifndef LightingPassShader_H
#define LightingPassShader_H

namespace Graphic {
namespace Dereferred {

class LightingPassShader : public Shader {
	SPACERTTI_DECLARE_ABSTRACT_CLASS(LightingPassShader, Shader);
public:
 	LightingPassShader(GLuint ShaderProgram, const string &ProgramName);
 	virtual ~LightingPassShader();

	struct SamplerIndex {
		enum {
			Empty = 0,
			Position,
			Diffuse,
			Normal,

			PlaneShadow = 5,
		};
	};

	void Bind(const Light::LightBase &light) const {
		glUniform3fv(m_ColorLocation, 1, &light.Color[0]); 
		glUniform1f(m_AmbientIntensityLocation, light.AmbientIntensity);
		glUniform1f(m_DiffuseIntensityLocation, light.DiffuseIntensity);
		glUniform1i(m_EnableShadowsLocation, light.CastShadows);
	}

	void SetShadowMapSize(const math::vec3 &size) {
		glUniform3fv(m_ShadowMapSizeLocation, 1, &size[0]); 
	}
	void SetShadowMapSize(const math::vec2 &size) {
		glUniform3f(m_ShadowMapSizeLocation, size[0], size[1], 0); 
	}
	void SetLightMatrix(const math::mat4 &mat) {
		glUniformMatrix4fv(m_LightMatrixLocation, 1, GL_FALSE, &mat[0][0]);
	}

	void BindShadowMap(const PlaneShadowMap &smap) {
		smap.BindAsTexture(SamplerIndex::PlaneShadow);
		SetShadowMapSize(smap.GetSize());
	}

protected:
	GLuint m_ColorLocation;
	GLuint m_AmbientIntensityLocation;
	GLuint m_DiffuseIntensityLocation;
private:
	GLuint m_ShadowMapSizeLocation;
	GLuint m_LightMatrixLocation;
	GLuint m_EnableShadowsLocation;
};

} //namespace Dereferred
} //namespace Graphic 

#endif
