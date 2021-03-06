#include "pch.h"

#include "../../nfRenderer.h"
#include "../../iAsyncLoader.h"
#include "TextureResource.h"
#include "FreeImageLoader.h"

#include "FreeImageUtils.h"

namespace MoonGlare::Renderer::Resources::Texture {

void FreeImageLoader::OnFileReady(const std::string &requestedURI, StarVFS::ByteTable &filedata, ResourceLoadStorage &storage) {

    LoadTexture(storage, filedata.get(), filedata.byte_size());
}

void FreeImageLoader::LoadTexture(ResourceLoadStorage &storage, void *image, size_t datasize) {

    FIMEMORY *fim = FreeImage_OpenMemory((BYTE*)image, datasize);
    FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fim);

    int flags = 0;
    switch (fif) {
    case FIF_PNG:
        //flags |= PNG_IGNOREGAMMA;
        break;
    default:
        break;
    }

    FIBITMAP *dib = FreeImage_LoadFromMemory(fif, fim, flags);
    FreeImage_CloseMemory(fim);

    auto dibrelease = ImageUniquePtr((void*)dib, &DibDeallocator);

    LoadImage(storage, dib, fif);
}

void FreeImageLoader::LoadImage(ResourceLoadStorage &storage, FIBITMAP *bitmap, FREE_IMAGE_FORMAT fif) {
    assert(bitmap);

    //switch (fif) {
    //case FIF_PNG: //png images need to be flipped
        //FreeImage_FlipVertical(bitmap);
        //break;
    //default:
        //nothing todo
        //break;
    //}

    //FreeImage_FlipVertical(dib);
    //FreeImage_FlipHorizontal(dib);

    PixelFormat srcFormat = PixelFormat::RGB8;
    PixelFormat format = PixelFormat::RGB8;
    ValueFormat valueFormat = //(ValueFormat)GL_UNSIGNED_INT_8_8_8_8;
        //GL_BYTE;//
    ValueFormat::UnsignedByte;

    auto mask = FreeImage_GetBlueMask(bitmap);
    //config.m_Flags.useSRGBColorSpace = false;
    
    switch (FreeImage_GetBPP(bitmap)) {
    case 32:
        if (mask != FI_RGBA_RED_MASK)
            SwapRedAndBlue(bitmap);
        format = PixelFormat::RGBA8;
        srcFormat = config.m_Flags.useSRGBColorSpace ? PixelFormat::SRGBA8 : PixelFormat::RGBA8;
        break;
    case 24:
        if (mask != FI_RGBA_RED_MASK)
            SwapRedAndBlue(bitmap);
        format = PixelFormat::RGB8;
        srcFormat = config.m_Flags.useSRGBColorSpace ? PixelFormat::SRGB8 : PixelFormat::RGB8;
        break;
    default: {
        __debugbreak();
        assert(false);
        //FIBITMAP *dib24 = FreeImage_ConvertTo24Bits(dib);
        //FreeImage_Unload(dib);
        //dib = dib24;
        //SwapRedAndBlue(dib);
        //out.m_PixelFormat = PixelFormat::RGB8;
    }
    }

    auto size = emath::usvec2(FreeImage_GetWidth(bitmap), FreeImage_GetHeight(bitmap));
    size_t bytesize = (size_t)size[0] * (size_t)size[1] * ((size_t)FreeImage_GetBPP(bitmap) / 8);

    void* pixels = FreeImage_GetBits(bitmap);

    void *storagepixels = storage.m_Memory.m_Allocator.Clone((uint8_t*)pixels, bytesize);
    if (!storagepixels) {
        throw NotEnoughStorage{ bytesize };
    }

    owner->SetTexturePixels(handle, storage.m_Queue, storagepixels, size, config, srcFormat, format, true, valueFormat, {});
}

}
