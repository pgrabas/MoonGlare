#pragma once

namespace MoonGlare::Asset {

class TextureLoader {
public:
    virtual ~TextureLoader() {}

    enum class PixelFormat : uint16_t {
        RGB8 = GL_RGB,
        RGBA8 = GL_RGBA,
        GrayScale8 = GL_LUMINANCE, //?
        LuminanceAlpha = GL_LUMINANCE_ALPHA,
    };

    static unsigned BppFromPixelFormat(PixelFormat pf) {
        switch (pf) {
        case PixelFormat::RGB8:
            return 24;
        case PixelFormat::RGBA8:
            return 32;
        case PixelFormat::GrayScale8:
            return 8;
        case PixelFormat::LuminanceAlpha:
            return 16;
        default:
            return 0;
        }
    }

    enum class ValueFormat : uint16_t {
        UnsignedByte = GL_UNSIGNED_BYTE,//this constants should not be used here!!!
        Float = GL_FLOAT,
    };

    using ImageUniquePtr = std::unique_ptr<void, void(*)(void*)>;

    struct TexturePixelData {
        void* m_Pixels;
        unsigned m_PixelsByteSize;
        ValueFormat m_PixelType;
        PixelFormat m_PixelFormat;
        emath::usvec2 m_PixelSize;
        ImageUniquePtr m_ImageMemory{ nullptr, [](void*) {} };
    };

    virtual bool LoadTexture(const std::string &fpath, TexturePixelData &out) = 0;
    virtual bool LoadTextureMeta(const std::string &fpath, TexturePixelData &out) = 0;

    virtual TexturePixelData AllocateImage(PixelFormat pf, const emath::usvec2 &Size) = 0;
    virtual void StoreScreenShot(TexturePixelData out) = 0;
};

class AssetLoader {
public:
    virtual ~AssetLoader() {}

    virtual TextureLoader* GetTextureLoader() const { return nullptr; }

    virtual bool ReadFile(const std::string &URI, StarVFS::ByteTable &out) = 0;
};

} //namespace MoonGlare::Renderer::Resources 
