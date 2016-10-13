/*
  * Generated by cppsrc.sh
  * On 2015-05-26 18:55:35,14
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>
#include <Engine/DataClasses/iFont.h>
#include "FreeTypeHelper.h"
#include "TrueTypeFont.h"
#include "TrueTypeWrapper.h"

namespace MoonGlare {
namespace Modules {
namespace TrueTypeFont {
	
SPACERTTI_IMPLEMENT_STATIC_CLASS(TrueTypeFont);

TrueTypeFont::TrueTypeFont(const string& Name): 
		BaseClass(Name) {
}

TrueTypeFont::~TrueTypeFont() {
}

//----------------------------------------------------------------

bool TrueTypeFont::DoInitialize() {
	if (!BaseClass::DoInitialize())
		return false;

	auto meta = OpenMetaData();
	if (!meta) {
		//silently ignore
		return false;
	}
	auto root = meta->document_element();

	if (!GetFileSystem()->OpenFile(root.child("File").text().as_string(ERROR_STR), DataPath::Fonts, m_FontFile)) {
		AddLog(Error, "Unable to open font file!");
		return false;
	}

	auto error = FT_New_Memory_Face(ftlib,
                            (const FT_Byte*)m_FontFile.get(),	/* first byte in memory */
                            m_FontFile.size(),							/* size in bytes        */
                            0,											/* face_index           */
                            &m_FontFace );

	if (error) {
		AddLogf(Error, "Unable to load font: '%s' error code: %d", GetName().c_str(), error);
		return false;
	}

	m_CacheHight = 32;
	error = FT_Set_Char_Size(
					m_FontFace,			/* handle to face object           */
					0,					/* char_width in 1/64th of points  */
		FT_F26Dot6(m_CacheHight*64.0f),	/* char_height in 1/64th of points */
					96,				/* horizontal device resolution    */
					96 );				/* vertical device resolution      */

	if (error) {
		AddLogf(Error, "Unable to set font size: '%s' error code: %d", GetName().c_str(), error);
		return Finalize();
	}

	//todo: select unicode charmap

	return true;
}

bool TrueTypeFont::DoFinalize() {
	m_GlyphCache.clear();
	FT_Done_Face(m_FontFace);
	m_FontFace = nullptr;
	m_FontFile.reset();
	return BaseClass::DoFinalize();
}

//----------------------------------------------------------------

FontInstance TrueTypeFont::GenerateInstance(const wstring &text, const Descriptor *style, bool UniformPosition) const {
	if (text.empty() || !IsReady()) {
		return FontInstance(new EmptyWrapper());
	}

	static Graphic::IndexVector BaseIndex{ 0, 1, 2, 0, 2, 3, };

	auto wrapper = new TrueTypeWrapper();
	wrapper->m_Chars.reserve(text.length());
	float h;
	if (style) {
		wrapper->m_Color = style->Color;
		h = style->Size;
	} else {
		wrapper->m_Color = Graphic::vec3(1);
		h = m_CacheHight;
	}

	Graphic::VertexVector Verticles;
	Graphic::TexCoordVector TexCoords;
	Graphic::NormalVector Normals;
	Graphic::IndexVector Index;

	unsigned text_len = text.length();
	Verticles.reserve(4 * text_len);
	TexCoords.reserve(4 * text_len);
	Index.reserve(6 * text_len);
	wrapper->m_Chars.reserve(text_len);

	Graphic::vec3 char_scale(h / m_CacheHight);
	const wstring::value_type *cstr = text.c_str();
	Graphic::vec2 pos(0);
	float hmax = 0;

	auto ScreenSize = math::fvec2(Graphic::GetRenderDevice()->GetContextSize());
	float Aspect = ScreenSize[0] / ScreenSize[1];

	while (*cstr) {
		wchar_t c = *cstr;
		++cstr;

		if (c == L' ' && !*cstr) 
			break; //ignore trailing space char

		auto *g = GetGlyph(c);
		if (!g)
			continue;

		auto bs = g->m_BitmapSize;
		auto chpos = g->m_Position;
		chpos *= char_scale.x;
		bs *= char_scale.x;
		auto subpos = pos + chpos;

		if (c != L' ') {
			if (UniformPosition) {
				subpos = subpos / ScreenSize * math::fvec2(Aspect * 2.0f, 2.0f);
				bs = bs / ScreenSize * math::fvec2(Aspect * 2.0f, 2.0f);
			}

			wrapper->m_Chars.push_back(g);
			auto base = Verticles.size();
			Verticles.push_back(Graphic::vec3(subpos.x + 0,		subpos.y + bs.y, 0) *= char_scale);
			Verticles.push_back(Graphic::vec3(subpos.x + 0,		subpos.y + 0,	 0) *= char_scale);
			Verticles.push_back(Graphic::vec3(subpos.x + bs.x,	subpos.y + 0,	 0) *= char_scale);
			Verticles.push_back(Graphic::vec3(subpos.x + bs.x,	subpos.y + bs.y, 0) *= char_scale);
			auto &tc = g->m_TextureSize;
			TexCoords.push_back(Graphic::vec2(0,	tc.y));
			TexCoords.push_back(Graphic::vec2(0,	0));
			TexCoords.push_back(Graphic::vec2(tc.x,	0));
			TexCoords.push_back(Graphic::vec2(tc.x,	tc.y));

			for (auto idx : BaseIndex)
				Index.push_back(base + idx);
		}

		pos.x += g->m_Advance.x * char_scale.x;
		hmax = math::max(h, bs.y);
	}
	pos.y = hmax;

	if (UniformPosition)
		wrapper->m_size = pos / math::fvec2(ScreenSize) * math::fvec2(Aspect * 2.0f, 2.0f);
	else
		wrapper->m_size = pos;

	wrapper->m_VAO.DelayInit(Verticles, TexCoords, Normals, Index);
	
	return FontInstance(wrapper);
}

//----------------------------------------------------------------

FontGlyph* TrueTypeFont::GetGlyph(wchar_t codepoint) const {
	auto &glyph = m_GlyphCache[codepoint];

	if (glyph)
		return glyph.get();

	auto glyph_index = FT_Get_Char_Index(m_FontFace, codepoint);
	auto load_flags = FT_LOAD_DEFAULT;// FT_LOAD_RENDER;
	auto error = FT_Load_Glyph(
					  m_FontFace,    /* handle to face object */
					  glyph_index,   /* glyph index           */
					  load_flags);   /* load flags, see below */

	if (error) {
		AddLogf(Error, "Unable to load glyph for char: 0x%x", codepoint);
		return nullptr;
	}

	glyph = std::make_unique<FontGlyph>();
 
	FT_Glyph ffglyph = nullptr;
	if (FT_Get_Glyph(m_FontFace->glyph, &ffglyph)) {
		AddLogf(Error, "Get glyph failed! codepoint: 0x%x", codepoint);
		return nullptr;
	}

    // Convert The Glyph To A Bitmap.
    if(FT_Glyph_To_Bitmap( &ffglyph, ft_render_mode_normal, 0, 1 )) {
		AddLogf(Error, "Glyph  render failed! codepoint: 0x%x", codepoint);
		FT_Done_Glyph(ffglyph);
		return nullptr;
	}
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)ffglyph;
 
    // This Reference Will Make Accessing The Bitmap Easier.
    FT_Bitmap& bitmap=bitmap_glyph->bitmap;

	glyph->m_BitmapSize = Graphic::vec2(bitmap.width, bitmap.rows);
	glyph->m_Position = Graphic::vec2(bitmap_glyph->left, -bitmap_glyph->top + m_CacheHight);
	glyph->m_Advance = Graphic::vec2(m_FontFace->glyph->advance.x >> 6, 0);// +glyph->m_Position;

	//if texture exists
	if (bitmap.width > 0 && bitmap.rows > 0) {
		//translate it and upload to render device
		unsigned width = math::next_power2(bitmap.width);
		unsigned height = math::next_power2(bitmap.rows);
		short *expanded_data = new short[width * height];
 
		for(unsigned j=0; j <height;j++) {
			for(unsigned i=0; i < width; i++){
				char value = (i>=bitmap.width || j>=bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width*j]; 
				short u = value | value << 8;
				expanded_data[(i + j*width)] = u;// expanded_data[2 * (i + j*width) + 1] = value;
			}
		}

		float x = (float)bitmap.width / (float)width;
		float y = (float)bitmap.rows / (float)height;
		glyph->m_TextureSize = Graphic::vec2(x, y);

		auto ptr = glyph.get();
		Graphic::GetRenderDevice()->DelayedContextManip([expanded_data, width, height, ptr] {
			ptr->m_Texture.New();
			//ptr->m_Texture.SetLinearFiltering();
			//ptr->m_Texture.SetClampToEdges();
			ptr->m_Texture.SetTextureBits(expanded_data, Graphic::uvec2(width, height), Graphic::Flags::LuminanceAlpha, Graphic::Flags::fUnsignedChar);
			ptr->m_Texture.ApplyGlobalSettings();
			delete[] expanded_data;
			ptr->m_Ready = true;
		});
	}
#if 0
	auto faceglyph = m_FontFace->glyph;
	AddLogf(Hint, "char %c[0x%x]  size:(%ld; %ld)  pos:(%d; %d)   bsize(%d; %d)",
				(unsigned)codepoint, (unsigned)codepoint,
				faceglyph->advance.x, faceglyph->advance.y, 
				bitmap_glyph->left, bitmap_glyph->top,
				bitmap.width, bitmap.rows
			);
#endif
	return glyph.get();
}

} //namespace TrueTypeFont 
} //namespace Modules 
} //namespace MoonGlare 
