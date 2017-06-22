/*
  * Generated by cppsrc.sh
  * On 2016-08-10 17:23:57,17
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <MoonGlare.h>

#include "DirectAnimationComponent.h"

#include <Core/Component/ComponentManager.h>
#include <Core/Component/ComponentRegister.h>
#include <Core/Component/TransformComponent.h>

#include <DirectAnimationComponent.x2c.h>

#include <Renderer/Commands/OpenGL/ControllCommands.h>
#include <Renderer/Commands/OpenGL/ShaderCommands.h>
#include <Renderer/Commands/OpenGL/TextureCommands.h>
#include <Renderer/Commands/OpenGL/ArrayCommands.h>

#include <Source/Renderer/Frame.h>

namespace MoonGlare {
namespace Renderer {
namespace Component {

::Space::RTTI::TypeInfoInitializer<DirectAnimationComponent, DirectAnimationComponentEntry>DirectAnimationTypeInfo;
RegisterComponentID<DirectAnimationComponent> DirectAnimationComponentReg("DirectAnimation", true, &DirectAnimationComponent::RegisterScriptApi);

DirectAnimationComponent::DirectAnimationComponent(ComponentManager * Owner) 
	: TemplateStandardComponent(Owner)
	, m_TransformComponent(nullptr)
{
	//DebugMemorySetClassName("DirectAnimationComponent");
	//DebugMemoryRegisterCounter("IndexUsage", [this](DebugMemoryCounter& counter) {
	//	counter.Allocated = m_Array.Allocated();
	//	counter.Capacity = m_Array.Capacity();
	//	counter.ElementSize = sizeof(MeshEntry);
	//});
}

DirectAnimationComponent::~DirectAnimationComponent() {
}

//------------------------------------------------------------------------------------------

void DirectAnimationComponent::RegisterScriptApi(ApiInitializer & root) {
	root
	.beginClass<DirectAnimationComponentEntry>("cDirectAnimationComponentEntry")
	//	.addProperty("Visible", &MeshEntry::IsVisible, &MeshEntry::SetVisible)
	//	.addProperty("MeshHandle", &MeshEntry::GetMeshHandle, &MeshEntry::SetMeshHandle)
	//	.addFunction("SetModel", &MeshEntry::SetModel)
		.addData("FirstFrame", &DirectAnimationComponentEntry::m_FirstFrame)
		.addData("LastFrame", &DirectAnimationComponentEntry::m_EndFrame)
	.endClass()
	;
}

//------------------------------------------------------------------------------------------

bool DirectAnimationComponent::Initialize() {
//	m_Array.MemZeroAndClear();
//	m_Array.fill(MeshEntry());

	m_TransformComponent = GetManager()->GetComponent<TransformComponent>();
	if (!m_TransformComponent) {
		AddLog(Error, "Failed to get RectTransformComponent instance!");
		return false;
	}

//	::Graphic::GetRenderDevice()->RequestContextManip([this] () {
//		if (!(m_ShadowShader = Graphic::GetShaderMgr()->GetShader("ShadowMap"))) {
//			AddLogf(Error, "Failed to load ShadowMap shader");
//		}
//		if (!(m_GeometryShader = Graphic::GetShaderMgr()->GetShader("Deferred/Geometry"))) {
//			AddLogf(Error, "Failed to load Dereferred.Geometry shader");
//		}
//	});
	
	return true;
}

bool DirectAnimationComponent::Finalize() {
	return true;
}

void DirectAnimationComponent::Step(const Core::MoveConfig &conf) {
#if 0
	using Configuration::Renderer::CommandQueueID;
	auto &ShadowQueue = conf.m_RenderInput->m_CommandQueues[CommandQueueID::DefferedShadow];
	auto &GeometryQueue = conf.m_RenderInput->m_CommandQueues[CommandQueueID::DefferedGeometry];

	size_t LastInvalidEntry = 0;
	size_t InvalidEntryCount = 0;

	for (size_t i = 0; i < m_Array.Allocated(); ++i) {//ignore root entry
		auto &item = m_Array[i];

		if (!item.m_Flags.m_Map.m_Valid) {
			//mark and continue
			LastInvalidEntry = i;
			++InvalidEntryCount;
			continue;
		}

		if (!GetHandleTable()->IsValid(this, item.m_SelfHandle)) {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			//mark and continue but set valid to false to avoid further checks
			continue;
		}

		if (item.m_Flags.m_Map.m_Playing) {
			item.m_LocalTime += conf.TimeDelta;
		}

		if (!item.m_Flags.m_Map.m_Visible) {
			continue;
		}

		float FramePosition = item.m_LocalTime * item.m_FramesPerSecond;
		auto framecount = item.m_EndFrame - item.m_FirstFrame + 1;
		FramePosition -= static_cast<unsigned>(FramePosition / framecount) * framecount;
		FramePosition += item.m_FirstFrame;

		if (item.m_Flags.m_Map.m_Playing) {
			item.Calculate(FramePosition);//(item.m_LocalTime / 1.5f);
		}

		if (item.m_VAO.Handle() == 0)
			continue;

		auto *tcentry = m_TransformComponent->GetEntry(item.m_Owner);
		if (!tcentry) {
			item.m_Flags.m_Map.m_Valid = false;
			LastInvalidEntry = i;
			++InvalidEntryCount;
			//mark and continue but set valid to false to avoid further checks
			continue;
		}

		//animate mesh
		math::mat4 matrices[60];
		float* tempmat = &matrices[0][0][0];

		for (unsigned oid = 0; oid < item.m_Scene->mRootNode->mNumChildren; ++oid) {
			auto node = item.m_Scene->mRootNode->mChildren[oid];
			if (node->mNumMeshes == 0)
				continue;

			for (unsigned mid = 0; mid < node->mNumMeshes; ++mid) {
				const auto& boneMats = item.GetBoneMatrices(node, mid);

				auto &mesh = item.m_Meshes[node->mMeshes[mid]];
				for (unsigned int a = 0; a < mesh.BoneCount; a++) {
					const aiMatrix4x4& mat = boneMats[a];
					*tempmat++ = mat.a1; *tempmat++ = mat.a2; *tempmat++ = mat.a3; *tempmat++ = mat.a4;
					*tempmat++ = mat.b1; *tempmat++ = mat.b2; *tempmat++ = mat.b3; *tempmat++ = mat.b4;
					*tempmat++ = mat.c1; *tempmat++ = mat.c2; *tempmat++ = mat.c3; *tempmat++ = mat.c4;
					*tempmat++ = mat.d1; *tempmat++ = mat.d2; *tempmat++ = mat.d3; *tempmat++ = mat.d4;
				}

			//	math::mat4 *matrices = (math::mat4*)&boneMats[0];

				for (unsigned vid = 0; vid < mesh.VertexCount; ++vid) {
					auto gvid = mesh.BaseVertex + vid;
					auto &out = item.m_RunTimeMesh[gvid];
					auto &outnormal = item.m_RunTimeMeshNormals[gvid];

					const auto &in = item.m_Mesh[gvid];
					const auto &innormal = item.m_MeshNormals[gvid];
					const auto &inboneids = item.m_BoneIds[gvid];
					const auto &inbonew = item.m_BoneWeights[gvid];

					math::vec4 out4(0);
					math::vec4 out4normal(0);
					math::vec4 in4(in, 1);
					math::vec4 in4normal(innormal, 0);

					for (unsigned bid = 0; bid < 4; ++bid) {
						auto transform = matrices[inboneids[bid]] * inbonew[bid];

						out4 += in4 * transform;
						out4normal += in4normal * transform;
					}

					out = out4;
					outnormal = out4normal;
				}
			}

		}

		void *frameverticles = &item.m_RunTimeMesh[0][0];
		void *framenormals = &item.m_RunTimeMeshNormals[0][0];
		//animate mesh end

		Renderer::RendererConf::CommandKey key{ 0 };

		auto &PrepareQueue = conf.m_BufferFrame->GetControllCommandQueue();

		PrepareQueue.PushCommand<Renderer::Commands::BindArrayBuffer>(key)->m_Handle = item.m_VAO.ChannelHandle(item.m_VAO.CoordChannel);
		auto data = PrepareQueue.PushCommand<Renderer::Commands::ArrayBufferDynamicData>(key);
		data->m_DataPtr = frameverticles;
		data->m_ByteCount = sizeof(item.m_Mesh[0]) * item.m_Mesh.size();

		PrepareQueue.PushCommand<Renderer::Commands::BindArrayBuffer>(key)->m_Handle = item.m_VAO.ChannelHandle(item.m_VAO.NormalChannel);
		data = PrepareQueue.PushCommand<Renderer::Commands::ArrayBufferDynamicData>(key);
		data->m_DataPtr = framenormals;
		data->m_ByteCount = sizeof(item.m_MeshNormals[0]) * item.m_Mesh.size();

		PrepareQueue.PushCommand<Renderer::Commands::BindArrayBuffer>(key)->m_Handle = 0;

		//SetModelMatrix(m_GeometryShader, GeometryQueue, key, tcentry->m_GlobalMatrix);
		//SetModelMatrix(m_ShadowShader, ShadowQueue, key, tcentry->m_GlobalMatrix);

		GeometryQueue.PushCommand<Renderer::Commands::VAOBind>(key)->m_VAO = item.m_VAO.Handle();
		ShadowQueue.PushCommand<Renderer::Commands::VAOBind>(key)->m_VAO = item.m_VAO.Handle();

		for (auto &mesh : item.m_Meshes) {
			//s->SetAlphaThreshold(AlphaT	
//			auto &mat = item.m_Materials[mesh.MaterialIndex];
//			GeometryQueue.PushCommand<Renderer::Commands::Texture2DBind>(key)->m_Texture = mat.Texture.Handle();
			//SetMaterialColor(m_GeometryShader, GeometryQueue, key, math::vec4( mat.BackColor, 1));

			auto arg = GeometryQueue.PushCommand<Renderer::Commands::VAODrawTrianglesBaseVertex>(key);
			arg->m_NumIndices = mesh.NumIndices;
			arg->m_BaseIndex = mesh.BaseIndex;
			arg->m_BaseVertex = mesh.BaseVertex;
			arg->m_IndexValueType = item.m_VAO.IndexValueType();

			auto sharg = ShadowQueue.PushCommand<Renderer::Commands::VAODrawTrianglesBaseVertex>(key);
			*sharg = *arg;
		}

	}

	if (InvalidEntryCount > 0) {
		AddLogf(Performance, "DirectAnimationComponent:%p InvalidEntryCount:%lu LastInvalidEntry:%lu", this, InvalidEntryCount, LastInvalidEntry);
		ReleaseElement(LastInvalidEntry);
	}
#endif
}

bool DirectAnimationComponent::Load(xml_node node, Entity Owner, Handle &hout) {	
	x2c::Component::DirectAnimationComponent::DirectAnimationEntry_t dae;
	dae.ResetToDefault();

	if (!dae.Read(node)) {
		AddLogf(Error, "Failed to read DirectAnimationEntry entry!");
		return false;
	}

	StarVFS::ByteTable data;
	if (!GetFileSystem()->OpenFile(dae.m_AnimationFile, DataPath::URI, data)) {
		AddLogf(Error, "Unable to load model: %s", dae.m_AnimationFile.c_str());
		return false;
	}
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFileFromMemory(
		data.get(), data.size(),
		aiProcess_JoinIdenticalVertices |/* aiProcess_PreTransformVertices | */aiProcess_Triangulate | aiProcess_GenUVCoords | aiProcess_SortByPType,
		strrchr(dae.m_AnimationFile.c_str(), '.'));

	
	if (!scene) {
		AddLog(Error, "Unable to to load model file[Name:'" << dae.m_AnimationFile.c_str() << "']. Error: " << importer.GetErrorString());
		return false;
	}

	scene = importer.GetOrphanedScene();

	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}

	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();
	entry.Reset();

	entry.m_Flags.m_Map.m_Loop = dae.m_Loop;
	entry.m_Flags.m_Map.m_Playing = true;
	entry.m_EndFrame = dae.m_EndFrame;
	entry.m_FirstFrame = dae.m_FirstFrame;
	entry.m_FramesPerSecond = dae.m_FramesPerSecond;
	entry.m_Flags.m_Map.m_Visible = dae.m_Visible;

	//Load materials/textures
	for (unsigned i = 0; i < scene->mNumMaterials; i++) {
		const aiMaterial* pMaterial = scene->mMaterials[i];
		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) <= 0)
			continue;

		aiString Path;
		if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
			AddLog(Error, "Unable to load material for model: " << dae.m_AnimationFile);
		}
		std::string uri = dae.m_AnimationFile;
		uri.erase(uri.begin() + uri.rfind("/") + 1, uri.end());
		uri += strrchr(Path.data, '\\') + 1;

//		auto mat = &entry.m_Materials[i];
	//	GetFileSystem()->OpenTexture(mat->Texture, uri, DataPath::URI);
	//	mat->SetRepeatEdges();
	}

	//Load meshes
	{
		using MeshData = DirectAnimationComponentEntry::MeshData;

		Graphic::VertexVector verticles;
		Graphic::NormalVector normals;
		Graphic::TexCoordVector texCords;
		Graphic::IndexVector indices;

		unsigned NumVertices = 0, NumIndices = 0;
		entry.m_Meshes.resize(scene->mNumMeshes);
		for (unsigned i = 0; i < entry.m_Meshes.size(); i++) {
			MeshData &meshd = entry.m_Meshes[i];
			//m_Meshes[i].MaterialIndex = pScene->mMeshes[i]->mMaterialIndex;
			meshd.NumIndices = scene->mMeshes[i]->mNumFaces * 3;
			meshd.BaseVertex = NumVertices;
			meshd.BaseIndex = NumIndices;
			meshd.VertexCount = scene->mMeshes[i]->mNumVertices;
			meshd.ElementMode = Graphic::Flags::fTriangles;

			NumVertices += meshd.VertexCount;
			NumIndices += meshd.BaseIndex;
		}

		verticles.resize(NumVertices);
		normals.resize(NumVertices);
		texCords.resize(NumVertices);

		entry.m_BoneIds.resize(NumVertices, math::uvec4(0));
		entry.m_BoneWeights.resize(NumVertices, math::fvec4(0));
		entry.m_RunTimeMesh.resize(NumVertices);
		entry.m_RunTimeMeshNormals.resize(NumVertices);

		std::vector<unsigned char> BoneIdAllocation;
		BoneIdAllocation.resize(NumVertices, 0);

		indices.reserve(NumIndices);
		
		unsigned boneoffset = 0;
		for (unsigned i = 0; i < entry.m_Meshes.size(); ++i) {
			const aiMesh* mesh = scene->mMeshes[i];
			MeshData& meshd = entry.m_Meshes[i];
			meshd.MaterialIndex = mesh->mMaterialIndex;
			meshd.BoneCount = mesh->mNumBones;

			unsigned BaseVertex = meshd.BaseVertex;
			for (unsigned j = 0; j < meshd.BoneCount; ++j) {
				const auto bone = mesh->mBones[j];
				for (unsigned k = 0; k < bone->mNumWeights; ++k) {
					auto &VertexWeight = bone->mWeights[k];

					auto vertexid = BaseVertex + VertexWeight.mVertexId;
					auto localboneid = BoneIdAllocation[vertexid]++;

					entry.m_BoneIds[vertexid][localboneid] = j + boneoffset;
					entry.m_BoneWeights[vertexid][localboneid] = VertexWeight.mWeight;
				}
			}
			boneoffset += meshd.BoneCount;

			for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
				aiVector3D &vertex = mesh->mVertices[j];
				aiVector3D &normal = mesh->mNormals[j];

				auto vid = BaseVertex + j;

				if (mesh->mTextureCoords[0]) {
					aiVector3D &UVW = mesh->mTextureCoords[0][j]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
					math::vec2 uv(UVW.x, UVW.y);

					texCords[vid] = uv;
				} else {
					texCords[vid] = math::vec2();
				}

				verticles[vid] = math::vec3(vertex.x, vertex.y, vertex.z);
				normals[vid] = math::vec3(normal.x, normal.y, normal.z);
			}

			for (unsigned int k = 0; k < mesh->mNumFaces; k++) {
				aiFace *f = &mesh->mFaces[k];
				THROW_ASSERT(f->mNumIndices == 3, 0);
				for (unsigned j = 0; j < 3; ++j) {
					indices.push_back(f->mIndices[j]);
				}
			}
		}

		entry.m_Mesh.swap(verticles);
		entry.m_MeshNormals.swap(normals);

		verticles.resize(entry.m_Mesh.size(), Graphic::vec3());
		normals.resize(entry.m_Mesh.size(), Graphic::vec3());

		entry.m_VAO.DelayInit(verticles, texCords, normals, indices);
	}

	entry.m_Scene = scene;
	entry.Load();

	entry.m_Owner = Owner;
	if (!GetHandleTable()->Allocate(this, entry.m_Owner, entry.m_SelfHandle, index)) {
		AddLogf(Error, "Failed to allocate handle!");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}
	m_EntityMapper.SetHandle(entry.m_Owner, entry.m_SelfHandle);
	entry.m_Flags.m_Map.m_Valid = true;

	hout = entry.m_SelfHandle;
	return true;
}

void DirectAnimationComponent::ReleaseElement(size_t Index) {
#if 0
	auto lastidx = m_Array.Allocated() - 1;

	if (lastidx == Index) {
		auto &last = m_Array[lastidx];
		GetHandleTable()->Release(this, last.m_SelfHandle); // handle may be already released; no need to check for failure
		last.Reset();
	} else {
		auto &last = m_Array[lastidx];
		auto &item = m_Array[Index];

		std::swap(last, item);

		if (!GetHandleTable()->SetHandleIndex(this, item.m_SelfHandle, Index)) {
			AddLogf(Error, "Failed to move DirectAnimationComponent handle index!");
		}
		GetHandleTable()->Release(this, last.m_SelfHandle); // handle may be already released; no need to check for failure
		last.Reset();
	}
	m_Array.DeallocateLast();
#endif // 0
}

bool DirectAnimationComponent::Create(Entity Owner, Handle &hout) {
#if 0
	size_t index;
	if (!m_Array.Allocate(index)) {
		AddLogf(Error, "Failed to allocate index!");
		return false;
	}

	auto &entry = m_Array[index];
	entry.m_Flags.ClearAll();

	Handle &ch = hout;
	if (!GetHandleTable()->Allocate(this, Owner, ch, index)) {
		AddLogf(Error, "Failed to allocate handle!");
		//no need to deallocate entry. It will be handled by internal garbage collecting mechanism
		return false;
	}

	entry.m_Owner = Owner;
	entry.m_SelfHandle = ch;

	entry.m_Flags.m_Map.m_Valid = true;
	entry.m_Flags.m_Map.m_MeshHandleChanged = false;
	entry.m_Flags.m_Map.m_Visible = true;

	m_EntityMapper.SetHandle(entry.m_Owner, ch);

	return true;
#endif // 0
	return false;
}

} //namespace Component 
} //namespace Renderer
} //namespace MoonGlare 
