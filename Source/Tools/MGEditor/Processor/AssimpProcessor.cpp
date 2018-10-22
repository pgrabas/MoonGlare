/*
  * Generated by cppsrc.sh
  * On 2016-10-13 18:53:51,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER

#include <fmt/format.h>

#include <icons.h>
#include <iFileProcessor.h>
#include <iFileIconProvider.h>
#include <iCustomEnum.h>
#include <ToolBase/Module.h>
#include <iIssueReporter.h>

#include <libs/LuaWrap/src/LuaDeleter.h>
#include <libs/LuaWrap/src/LuaException.h>

#include "../Windows/MainWindow.h"
#include "../FileSystem.h"

#pragma warning ( push, 0 )
#include <assimp/Importer.hpp>     
#include <assimp/scene.h>          
#include <assimp/postprocess.h>  
#pragma warning ( pop )

namespace MoonGlare {
namespace Editor {
namespace Processor {

struct AssimpProcessor
    : public QtShared::iFileProcessor {

    AssimpProcessor(SharedModuleManager modmgr, QtShared::SharedSetEnum MeshEnum, QtShared::SharedSetEnum MaterialEnum, std::string URI) 
        : QtShared::iFileProcessor(std::move(URI)), MeshEnum(MeshEnum), MaterialEnum(MaterialEnum), moduleManager(modmgr) { }

    ProcessResult ProcessFile() override {
        MeshEnum->Add(m_URI);

        //std::string nodesubPath = m_URI + "@node://";

        try {
            auto fs = moduleManager->QuerryModule<FileSystem>();
            StarVFS::ByteTable bt;
            if (!fs->GetFileData(m_URI, bt)) {
                //todo: log sth
                throw std::runtime_error("Unable to read file: " + m_URI);
            }
            if (bt.byte_size() == 0) {
                //todo: log sth
            }

            Assimp::Importer importer;
            unsigned flags = aiProcess_JoinIdenticalVertices |/* aiProcess_PreTransformVertices | */
                aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType;

            const aiScene* scene = importer.ReadFileFromMemory(bt.get(), bt.byte_size(), flags, strrchr(m_URI.c_str(), '.'));

            if (!scene) {
                //std::cout << "Unable to open file!\n";
                return ProcessResult::UnknownFailure;
            }

            std::string meshsubPath = m_URI + "@mesh://";
            for (unsigned i = 0; i < scene->mNumMeshes; ++i) {
                auto mesh = scene->mMeshes[i];
                MeshEnum->Add(meshsubPath + "*" + std::to_string(i));
                if (mesh->mName.length > 0) {
                    MeshEnum->Add(meshsubPath + mesh->mName.data);
                }
            }


            std::string materialsubPath = m_URI + "@material://";
            for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
                auto material = scene->mMaterials[i];
                MaterialEnum->Add(materialsubPath + "*" + std::to_string(i));

                //try {
                //    if (aiString v;  material->Get("?mat.name", 0, 0, v) == aiReturn_SUCCESS) {
                //        MaterialEnum->Add(materialsubPath + v.data);
                //    }
                //}
                //catch (...) {}
            }
        }
        catch (...) {
            return ProcessResult::UnknownFailure;

        }
        return ProcessResult::Success;
    }
private:
    QtShared::SharedSetEnum MeshEnum;
    QtShared::SharedSetEnum MaterialEnum;
    SharedModuleManager moduleManager;
};

//----------------------------------------------------------------------------------

struct AssimpProcessorModule
    : public iModule
    , public QtShared::iFileProcessorInfo
    , public QtShared::iCustomEnumSupplier
    , public QtShared::iFileIconInfo {

    AssimpProcessorModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    QtShared::SharedSetEnum MeshListEnum;
    QtShared::SharedSetEnum MaterialListEnum;

    QtShared::SharedFileProcessor CreateFileProcessor(std::string URI) override {
        return std::make_shared<AssimpProcessor>(GetModuleManager(), MeshListEnum, MaterialListEnum, std::move(URI));
    }

    std::vector<std::string> GetSupportedTypes() {
        return{ "3ds", "fbx", "blend" };
    }

    std::vector<std::shared_ptr<QtShared::iCustomEnum>> GetCustomEnums(QtShared::CustomEnumProvider *provider) override {
        MeshListEnum = provider->CreateEnum("string:Mesh.Mesh");
        MaterialListEnum = provider->CreateEnum("string:Mesh.Material");
        return { };
    }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{ "blend", ICON_16_3DMODEL_RESOURCE, },
            FileIconInfo{ "3ds", ICON_16_3DMODEL_RESOURCE, },
            FileIconInfo{ "fbx", ICON_16_3DMODEL_RESOURCE, },
        };
    }
};
ModuleClassRgister::Register<AssimpProcessorModule> AssimpProcessorModuleReg("AssimpProcessorModule");

//----------------------------------------------------------------------------------

} //namespace Processor 
} //namespace Editor 
} //namespace MoonGlare 
