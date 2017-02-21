/*
	Generated by cppsrc.sh
	On 2014-12-16  6:13:04,05
	by Paweu
*/

#pragma once
#ifndef Bitmap_H
#define Bitmap_H

namespace MoonGlare {
namespace Modules {
namespace BitmapFont {

using namespace DataClasses::Fonts;

class BitmapFontWrapper : public Wrapper {
	SPACERTTI_DECLARE_STATIC_CLASS(BitmapFontWrapper, Wrapper);
	DISABLE_COPY();
	friend class BitmapFont;
public:
	BitmapFontWrapper(const BitmapFont *font);
	~BitmapFontWrapper();

	virtual void Render(Graphic::cRenderDevice &dev) override;
	virtual void RenderMesh(Graphic::cRenderDevice &dev) override;

	virtual void GenerateCommands(Renderer::Commands::CommandQueue &Queue, uint16_t key) override;
protected:
	Graphic::vec3 m_Color;
	Graphic::VAO m_VAO;
	Graphic::VAO::MeshData m_Mesh;
	const TextureFile &m_Texture;
	const BitmapFont *m_font;
};

class BitmapFont : public iFont {
	SPACERTTI_DECLARE_CLASS(BitmapFont, iFont);
public:
	BitmapFont(const string& Name);
	~BitmapFont();

	FontRect TextSize(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const override;
	FontInstance GenerateInstance(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const override;

	float GetHeight() const { return (float)m_BFD.CharHeight; }
	const TextureFile& GetTexture() const { return m_Texture; }

	virtual bool RenderText(const std::wstring &text, Renderer::Frame *frame, const FontRenderRequest &options, FontRect &outTextRect, FontResources &resources) override;

	bool GenerateCommands(Renderer::Commands::CommandQueue &q, Renderer::Frame *frame, const std::wstring &text, const FontRenderRequest &options);
protected:
	class cBFDHeader {
	public:
		cBFDHeader();
		unsigned Width, Height, CharWidth, CharHeight;
		unsigned char BeginingKey, KeyWidths[256];
	};

	TextureFile m_Texture;
	cBFDHeader m_BFD;
	string m_TextureFile;
	string m_BfdFile;

	Graphic::Shaders::Shader *m_RtShader = nullptr;

	virtual bool DoInitialize() override;
};

} //namespace BitmapFont 
} //namespace Modules 
} //namespace MoonGlare 

#endif
