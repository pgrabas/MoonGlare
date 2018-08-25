/*
  * Generated by cppsrc.sh
  * On 2015-05-26 18:55:35,14
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>

#define NEED_VAO_BUILDER
#define NEED_MATERIAL_BUILDER

#include <nfMoonGlare.h>
#include <Engine/DataClasses/iFont.h>
#include "FreeTypeHelper.h"
#include "TrueTypeFont.h"

#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>

#include <Renderer/Frame.h>
#include <Renderer/Renderer.h>
#include <Renderer/RenderDevice.h>
#include <Renderer/TextureRenderTask.h>
#include <Renderer/Resources/ResourceManager.h>
#include <Renderer/Device/Types.h>

namespace MoonGlare {
namespace Modules {
namespace TrueTypeFont {
    
TrueTypeFont::TrueTypeFont(const string& Name): 
    iFont(Name) {
}

TrueTypeFont::~TrueTypeFont() {
}

//----------------------------------------------------------------

bool TrueTypeFont::DoInitialize() {
    if (!iFont::DoInitialize())
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
    return iFont::DoFinalize();
}

//----------------------------------------------------------------
//
//  _                    _ 	___________________
// | |                  | |					   |topline
// | |_  __  __   __ _  | |	___________________|_______________	baseline
// | __| \ \/ /  / _` | | |				  
// | |_   >  <  | (_| | | |				
//  \__| /_/\_\  \__, | |_|	____________________
//                __/ |    					   |
//               |___/     	___________________|bottomline
//
// |-----------------------| width

TrueTypeFont::FontRect TrueTypeFont::TextSize(const wstring & text, const Descriptor * style, bool UniformPosition) const {

    float topline = 0;
    float bottomline = 0;
    float width = 0;

    float h;
    if (style) {
        h = style->Size;
    } else {
        h = m_CacheHight;
    }

    math::vec3 char_scale(h / m_CacheHight);
    const wstring::value_type *cstr = text.c_str();
//	Graphic::vec2 pos(0);
//	float hmax = h;
//
//	auto ScreenSize = ...;
//	float Aspect = ScreenSize[0] / ScreenSize[1];

    while (*cstr) {
        wchar_t c = *cstr;
        ++cstr;

        auto *g = GetGlyph(c, nullptr, nullptr);
        if (!g)
            continue;

        auto BitmapSize = g->m_BitmapSize * char_scale.x;
        auto CharPos = g->m_Position * char_scale.x;

        topline = std::min(topline, CharPos.y);
        bottomline = std::max(bottomline, CharPos.y + BitmapSize.y);

//		auto subpos = pos + chpos;
//		float bsy = bs.y;
//
//		if (UniformPosition) {
//			subpos = subpos / ScreenSize * math::fvec2(Aspect * 2.0f, 2.0f);
//			bs = bs / ScreenSize * math::fvec2(Aspect * 2.0f, 2.0f);
//		}
//
        width += g->m_Advance.x * char_scale.x;
//		hmax = math::max(hmax, bsy);
    }
//	pos.y = hmax;
//
//	if (UniformPosition)
//		wrapper->m_size = pos / math::fvec2(ScreenSize) * math::fvec2(Aspect * 2.0f, 2.0f);
//	else
//		wrapper->m_size = pos;

    FontRect rect;
    rect.m_CanvasSize = math::fvec2(width, -topline + bottomline);
    rect.m_TextPosition = math::fvec2(0, -topline);
    rect.m_TextBlockSize = math::fvec2(width, h);
    return rect;
}

bool TrueTypeFont::GenerateCommands(Renderer::Commands::CommandQueue &q, Renderer::Frame * frame, const std::wstring &text, const FontRenderRequest & options) {
    if (text.empty())
        return true;

    static const unsigned BaseIndex[] = { 0, 1, 2, 0, 2, 3, };

    Renderer::VAOResourceHandle vao{ };
    if (!frame->AllocateFrameResource(vao))
        return false;

    unsigned textlen = text.length();
    unsigned VerticlesCount = textlen * 4;
    unsigned IndexesCount = textlen * 6;
    emath::fvec3 *Verticles = frame->GetMemory().Allocate<emath::fvec3>(VerticlesCount);
    emath::fvec2 *TextureUV = frame->GetMemory().Allocate<emath::fvec2>(VerticlesCount);
    uint16_t *VerticleIndexes = frame->GetMemory().Allocate<uint16_t>(IndexesCount);

    {
        auto vaob = frame->GetResourceManager()->GetVAOResource().GetVAOBuilder(q, vao, false);
        vaob.BeginDataChange();

        using ichannels = Renderer::Configuration::VAO::InputChannels;
        vaob.CreateChannel(ichannels::Vertex);
        vaob.SetChannelData<float, 3>(ichannels::Vertex, &Verticles[0][0], VerticlesCount);

        vaob.CreateChannel(ichannels::Texture0);
        vaob.SetChannelData<float, 2>(ichannels::Texture0, &TextureUV[0][0], VerticlesCount);

        vaob.CreateChannel(ichannels::Index);
        vaob.SetIndex(ichannels::Index, VerticleIndexes, IndexesCount);

        vaob.EndDataChange();
        vaob.BindVAO();
    }

//    float y = 0;
    float h = m_CacheHight;

    if (options.m_Size > 0) 
        h = options.m_Size;

    math::vec3 char_scale(h / m_CacheHight);
    const wstring::value_type *cstr = text.c_str();
    math::vec2 pos(0);
    float hmax = h;

    auto CurrentVertexQuad = Verticles;
    auto CurrentTextureUV = TextureUV;
    auto CurrentIndex = VerticleIndexes;
    bool allglyphs = true;
    while (*cstr) {
        wchar_t c = *cstr;
        ++cstr;

        auto *g = GetGlyph(c, &q, frame);
        if (!g) {
            allglyphs = false;
            continue;
        }

        allglyphs = allglyphs && g->m_Loaded;

        auto bs = g->m_BitmapSize;
        auto chpos = g->m_Position;
        chpos *= char_scale.x;
        bs *= char_scale.x;
        auto subpos = pos + chpos;
        float bsy = bs.y;

        if (c != L' ') {
            CurrentVertexQuad[0] = emath::fvec3(subpos.x + 0, subpos.y + bs.y, 0);
            CurrentVertexQuad[1] = emath::fvec3(subpos.x + 0, subpos.y + 0, 0);
            CurrentVertexQuad[2] = emath::fvec3(subpos.x + bs.x, subpos.y + 0, 0);
            CurrentVertexQuad[3] = emath::fvec3(subpos.x + bs.x, subpos.y + bs.y, 0);

            const auto &tc = g->m_TextureSize;
            CurrentTextureUV[0] = emath::fvec2(0, tc.y);
            CurrentTextureUV[1] = emath::fvec2(0, 0);
            CurrentTextureUV[2] = emath::fvec2(tc.x, 0);
            CurrentTextureUV[3] = emath::fvec2(tc.x, tc.y);

            size_t basevertex = CurrentVertexQuad - Verticles;
            size_t baseIndex = CurrentIndex - VerticleIndexes;
            for (auto idx : BaseIndex) {
                *CurrentIndex = static_cast<uint16_t>(idx + basevertex);
                ++CurrentIndex;
            }

            auto *resmgr = frame->GetResourceManager();
            auto mat = resmgr->GetMaterialManager().GetMaterial(g->m_GlyphMaterial);

            auto texbind = q.PushCommand<Renderer::Commands::Texture2DResourceBind>();
            texbind->m_HandlePtr = resmgr->GetTextureResource().GetHandleArrayBase() + mat->mapTexture[0].index;

            auto arg = q.PushCommand<Renderer::Commands::VAODrawTrianglesBase>();
            arg->m_NumIndices = 6;
            arg->m_BaseIndex = baseIndex * 2;
            arg->m_IndexValueType = Renderer::Device::TypeInfo<std::remove_reference_t<decltype(*VerticleIndexes)>>::TypeId;

            CurrentVertexQuad += 4;
            CurrentTextureUV += 4;
        }

        pos.x += g->m_Advance.x * char_scale.x;
        hmax = math::max(hmax, bsy);
    }

    return allglyphs;
}

//----------------------------------------------------------------

FontGlyph* TrueTypeFont::GetGlyph(wchar_t codepoint, Renderer::Commands::CommandQueue *q, Renderer::Frame * frame) const {
    auto &glyph = m_GlyphCache[codepoint];

    if (glyph && glyph->m_Loaded)
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

    if(!glyph)
        glyph = std::make_unique<FontGlyph>();
 
    FT_Glyph ffglyph = nullptr;
    if (FT_Get_Glyph(m_FontFace->glyph, &ffglyph)) {
        AddLogf(Error, "Get glyph failed! codepoint: 0x%x", (unsigned)codepoint);
        return nullptr;
    }

    // Convert The Glyph To A Bitmap.
    if(FT_Glyph_To_Bitmap( &ffglyph, ft_render_mode_normal, 0, 1 )) {
        AddLogf(Error, "Glyph  render failed! codepoint: 0x%x", (unsigned)codepoint);
        FT_Done_Glyph(ffglyph);
        return nullptr;
    }
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)ffglyph;
 
    // This Reference Will Make Accessing The Bitmap Easier.
    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    glyph->m_BitmapSize = math::vec2(bitmap.width, bitmap.rows);
    glyph->m_Position = math::vec2(bitmap_glyph->left, -bitmap_glyph->top + m_CacheHight);
    glyph->m_Advance = math::vec2(m_FontFace->glyph->advance.x >> 6, 0);// +glyph->m_Position;

    if (q && bitmap.width > 0 && bitmap.rows > 0) {
        //translate it and upload to render device
        unsigned int width = math::next_power2(bitmap.width);
        unsigned int height = math::next_power2(bitmap.rows);
        uint32_t *expanded_data = frame->GetMemory().Allocate<uint32_t>(width * height);

        for (unsigned j = 0; j < height; j++) {
            for (unsigned i = 0; i < width; i++) {
                char value = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width*j];
                uint32_t u = value | value << 8 | value << 16 | value << 24;
                expanded_data[(i + j*width)] = u;
            }
        }

        float x = (float)bitmap.width / (float)width;
        float y = (float)bitmap.rows / (float)height;
        glyph->m_TextureSize = math::vec2(x, y);

        auto *resmgr = frame->GetResourceManager();
        auto &texR = resmgr->GetTextureResource();

        Renderer::Configuration::TextureLoad tload = Renderer::Configuration::TextureLoad::Default();
        tload.m_Filtering = Renderer::Configuration::Texture::Filtering::Linear;
        tload.m_Edges = Renderer::Configuration::Texture::Edges::Clamp;
        auto size = emath::usvec2(width, height);

        using namespace Renderer::Device;

        Renderer::MaterialTemplate matT;
        matT.diffuseColor = { 1,1,1,1 };
        matT.diffuseMap.enabled = true;
        matT.diffuseMap.textureHandle = texR.CreateTexture(*q, (const uint8_t*)expanded_data, size, tload, PixelFormat::RGBA8, ValueFormat::UnsignedByte);

        glyph->m_GlyphMaterial = resmgr->GetMaterialManager().CreateMaterial("", matT);

        glyph->m_Loaded = true;
        auto faceglyph = m_FontFace->glyph;
        DebugLogf(Hint, "TTF[%s]: char %c[0x%x]  size:(%3d; %3d) pos:(%4d; %4d) bsize(%3d; %3d)",
                GetName().c_str(),
                (unsigned)codepoint, (unsigned)codepoint,
                faceglyph->advance.x, faceglyph->advance.y, 
                bitmap_glyph->left, bitmap_glyph->top,
                bitmap.width, bitmap.rows
            );
    } else {
        //glyph->m_Loaded = true;
    }
    FT_Done_Glyph(ffglyph);
    return glyph.get();
}

} //namespace TrueTypeFont 
} //namespace Modules 
} //namespace MoonGlare 
