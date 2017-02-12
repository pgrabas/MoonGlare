/*
  * Generated by cppsrc.sh
  * On 2015-05-26 18:55:35,14
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef TrueTypeFont_H
#define TrueTypeFont_H

namespace MoonGlare {
namespace Modules {
namespace TrueTypeFont {

using namespace DataClasses::Fonts;

class TrueTypeFont : public iFont {
	SPACERTTI_DECLARE_STATIC_CLASS(TrueTypeFont, iFont);
public:
 	TrueTypeFont(const string& Name);
 	virtual ~TrueTypeFont();

	FontRect TextSize(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const override;
	FontInstance GenerateInstance(const wstring &text, const Descriptor *style = nullptr, bool UniformPosition = false) const override;
protected:
	bool DoInitialize() override;
	bool DoFinalize() override;

	FontGlyph* GetGlyph(wchar_t codepoint) const;
private: 
	StarVFS::ByteTable m_FontFile;
	FT_Face m_FontFace = nullptr; 
	float m_CacheHight;

	mutable FontGlyphMap m_GlyphCache;
};

} //namespace TrueTypeFont 
} //namespace Modules 
} //namespace MoonGlare 

#endif
