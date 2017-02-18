/*
  * Generated by cppsrc.sh
  * On 2016-09-25 17:31:20,49
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef GUIShader_H
#define GUIShader_H

namespace MoonGlare {
namespace GUI {

class GUIShader : public ::Graphic::Shader {
public:
	GUIShader(GLuint ShaderProgram, const std::string &ProgramName) : ::Graphic::Shader(ShaderProgram, ProgramName) {
		m_BaseColorLocation = Location("gBaseColor");
		m_PanelAspectLocation = Location("gPanelAspect");
		m_PanelSizeLocation = Location("gPanelSize");
		m_PanelBorderLocation = Location("gPanelBorder");
		m_TileModeLocation = Location("gTileMode");
	}

	GLint m_BaseColorLocation;
	GLint m_PanelAspectLocation;
	GLint m_PanelSizeLocation;
	GLint m_PanelBorderLocation;
	GLint m_TileModeLocation;

	void Bind(Renderer::Commands::CommandQueue &Queue, Renderer::RendererConf::CommandKey key) {
		Queue.PushCommand<Renderer::Commands::ShaderBind>(key)->m_Shader = Handle();
	}
	void Bind(Renderer::Commands::CommandQueue &Queue) {
		Renderer::RendererConf::CommandKey key{ 0 };
		Queue.PushCommand<Renderer::Commands::ShaderBind>(key)->m_Shader = Handle();
	}

	void SetModelMatrix(Renderer::Commands::CommandQueue &Queue, Renderer::RendererConf::CommandKey key, const emath::fmat4 &ModelMat) {
		auto loc = Location(ShaderParameters::ModelMatrix);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformMatrix4>(key);
		arg->m_Location = loc;
		arg->m_Matrix = ModelMat;
	}

	void SetCameraMatrix(Renderer::Commands::CommandQueue &Queue, Renderer::RendererConf::CommandKey key,  const emath::fmat4 &CameraMat) {
		auto loc = Location(ShaderParameters::CameraMatrix);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformMatrix4>(key);
		arg->m_Location = loc;
		arg->m_Matrix = CameraMat;
	}

	void SetWorldMatrix(Renderer::Commands::CommandQueue &Queue, Renderer::RendererConf::CommandKey key, const emath::fmat4 & ModelMat, const emath::fmat4 &CameraMat) {
		auto loc = Location(ShaderParameters::WorldMatrix);
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformMatrix4>(key);
		arg->m_Location = loc;
		arg->m_Matrix = CameraMat * ModelMat;
	}
	void SetColor(Renderer::Commands::CommandQueue &Queue, Renderer::RendererConf::CommandKey key, const math::vec4 &color) {
		auto loc = Location("gBaseColor");
		if (!IsValidLocation(loc))
			return;

		auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformVec4>(key);
		arg->m_Location = loc;
		arg->m_Vec = color;
	}

	void SetPanelSize(Renderer::Commands::CommandQueue &Queue, Renderer::RendererConf::CommandKey key, const Point &Size) {
		if (IsValidLocation(m_PanelAspectLocation)) {
			auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformFloat>(key);
			arg->m_Location = m_PanelAspectLocation;
			arg->m_Float = Size[0] / Size[1];
		}

		if (IsValidLocation(m_PanelSizeLocation)) {
			auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformVec2>(key);
			arg->m_Location = m_PanelSizeLocation;
			*((Point*)(&arg->m_Vec)) = Size;
		}
	}
	void SetBorder(Renderer::Commands::CommandQueue &Queue, Renderer::RendererConf::CommandKey key, float Border) {
		if (IsValidLocation(m_PanelBorderLocation)) {
			auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformFloat>(key);
			arg->m_Location = m_PanelBorderLocation;
			arg->m_Float = Border;
		}
	}
	void SetTileMode(Renderer::Commands::CommandQueue &Queue, Renderer::RendererConf::CommandKey key, const glm::ivec2 &mode) {
		if (IsValidLocation(m_TileModeLocation)) {
			auto arg = Queue.PushCommand<Renderer::Commands::ShaderSetUniformIVec2>(key);
			arg->m_Location = m_TileModeLocation;
			*((glm::ivec2*)(&arg->m_Vec)) = mode;
		}
	}

	//	void TextureBind(Renderer::CommandQueue &Queue, Renderer::TextureHandle handle) {
	//		Queue.PushCommand<Renderer::Commands::Texture2DBind>()->m_Texture = handle;
	//	}

	//	void VAOBind(Renderer::CommandQueue &Queue, Renderer::VAOHandle handle) {
	//		Queue.PushCommand<Renderer::Commands::VAOBind>()->m_VAO = handle;
	//	}
	//	void VAORelease(Renderer::CommandQueue &Queue) {
	//		Queue.PushCommand<Renderer::Commands::VAORelease>();
	//	}

	void Enable(Renderer::Commands::CommandQueue &Queue, GLenum what) {
		Renderer::RendererConf::CommandKey key{ 0 };
		Queue.PushCommand<Renderer::Commands::Enable>(key)->m_What = what;
	}
	void Disable(Renderer::Commands::CommandQueue &Queue, GLenum what) {
		Renderer::RendererConf::CommandKey key{ 0 };
		Queue.PushCommand<Renderer::Commands::Disable>(key)->m_What = what;
	}
};

} //namespace GUI 
} //namespace MoonGlare 

#endif
