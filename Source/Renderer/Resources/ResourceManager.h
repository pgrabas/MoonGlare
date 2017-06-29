/*
  * Generated by cppsrc.sh
  * On 2017-02-13 22:45:58,54
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include <Assets/AssetLoaderInterface.h>
#include "TextureResource.h"
#include "VAOResource.h"
#include "ShaderResource.h"
#include "MaterialManager.h"

#include "../iAsyncLoader.h"

namespace MoonGlare::Renderer::Resources {

class AsyncLoader;

class MeshManager;
class textureResource;

class alignas(16) ResourceManager final {
public:
    ResourceManager();
    ~ResourceManager();

    bool Initialize(RendererFacade *Renderer, Asset::AssetLoader* Assets);
    bool Finalize();

    const Configuration::RuntimeConfiguration* GetConfiguration() const;
    RendererFacade *GetRendererFacade() { return m_RendererFacade; }
    AsyncLoader* GetLoader() { return m_AsyncLoader.get(); }
    iAsyncLoader* GetLoaderIf();

    TextureResource& GetTextureResource() {
        RendererAssert(this); 
        return *textureResource;
    }
    VAOResource& GetVAOResource() {
        RendererAssert(this);
        return m_VAOResource;
    }
    ShaderResource& GetShaderResource() {
        RendererAssert(this);
        return m_ShaderResource;
    }
    MaterialManager& GetMaterialManager() {
        RendererAssert(this);
        return m_MaterialManager;
    }
    MeshManager& GetMeshManager() {
        RendererAssert(this);
        return *meshManager;
    }

    void Release(Frame *frame, TextureResourceHandle &texres) {
        GetTextureResource().Release(frame, texres);
    }
    void Release(Frame *frame, VAOResourceHandle &vaores) {
        GetVAOResource().Release(frame, vaores);
    }
    bool Allocate(Frame *frame, TextureResourceHandle &resH) {
        return GetTextureResource().Allocate(frame, resH);
    }
    bool Allocate(Frame *frame, VAOResourceHandle &resH) {
        return GetVAOResource().Allocate(frame, resH);
    }
private: 
    RendererFacade *m_RendererFacade = nullptr;
    Asset::AssetLoader *m_AssetLoader = nullptr;
    std::unique_ptr<AsyncLoader> m_AsyncLoader;
    void* padding;
    VAOResource m_VAOResource;
    ShaderResource m_ShaderResource;
    MaterialManager m_MaterialManager;

    std::unique_ptr<TextureResource> textureResource;
    std::unique_ptr<MeshManager> meshManager;
};

static_assert((sizeof(ResourceManager) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer::Resources 
