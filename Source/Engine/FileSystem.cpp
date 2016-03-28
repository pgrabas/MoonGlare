/*
  * Generated by cppsrc.sh
  * On 2015-02-24 13:06:08,58
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include <MoonGlare.h>

#include <StarVFS/core/nStarVFS.h>

namespace MoonGlare {
namespace FileSystem {
	
const DataPathsTable DataSubPaths;

DataPathsTable::DataPathsTable() {
#define _at(X) m_table[(unsigned)DataPath::X] 
	_at(Root)		= "";
	_at(Maps)		= xmlModuleNode_Maps;
	_at(Models)		= xmlModuleNode_Models;
	_at(Fonts)		= xmlModuleNode_Fonts;
	_at(Scenes)		= xmlModuleNode_Scenes;
	_at(Shaders)	= xmlModuleNode_Shaders;
	_at(Scripts)	= xmlModuleNode_Scripts;
	_at(Sounds)		= xmlModuleNode_Sounds;
	_at(Music)		= xmlModuleNode_Music;
	_at(Texture)	= "Textures";
	_at(Tables)		= "Tables";
	_at(Objects)	= "Objects";
	
	_at(XML) = "XML";
#undef _at
}
	
void DataPathsTable::Translate(string& out, const string& in, DataPath origin) const {
	out.clear();
	out.reserve(StaticSettings::FileSystem::PathReserve);

	if (origin != DataPath::Root) {
		out += '/';
		out += m_table[(unsigned)origin];
		out += '/';
	} else {
		if (in.front() != '/')
			out += '/';
	}
	out += in;
}

void DataPathsTable::Translate(string& out, DataPath origin) const {
	out.clear();
	out.reserve(StaticSettings::FileSystem::PathReserve);

	if (origin != DataPath::Root) {
		out += '/';
		out += m_table[(unsigned)origin];
		out += '/';
	} else {
		out += '/';
	}
}
//-------------------------------------------------------------------------------------------------

GABI_IMPLEMENT_CLASS_SINGLETON(MoonGlareFileSystem)
RegisterDebugApi(MoonGlareFileSystem, &MoonGlareFileSystem::RegisterDebugScriptApi, "FileSystem");

MoonGlareFileSystem::MoonGlareFileSystem():
		BaseClass() {
	SetThisAsInstance();
}

MoonGlareFileSystem::~MoonGlareFileSystem() {
}

void MoonGlareFileSystem::RegisterDebugScriptApi(ApiInitializer &api) {
#ifdef DEBUG
	//api
	//.addFunction("SetWireFrameMode", &T::SetWireFrameMode)
	//;
#endif
}

//-------------------------------------------------------------------------------------------------

bool MoonGlareFileSystem::Initialize() {
	ASSERT(!m_StarVFS);
	m_StarVFS = std::make_unique<StarVFS::StarVFS>();
	return true;
}

bool MoonGlareFileSystem::Finalize() {
	m_StarVFS.reset();
	return true;
}

//-------------------------------------------------------------------------------------------------

bool MoonGlareFileSystem::EnumerateFolder(const string& Path, FileInfoTable &FileTable, bool Recursive) {
	ASSERT(m_StarVFS);

	auto ParentFID = m_StarVFS->FindFile(Path);

	if (!m_StarVFS->IsFileValid(ParentFID)) {
		AddLogf(Error, "Failed to open '%s' for enumeration", Path.c_str());
		return false;
	}

	FileTable.reserve(1024);//because thats why
	StarVFS::HandleEnumerateFunc svfsfunc;

	svfsfunc = [this, &svfsfunc, &FileTable, Recursive, ParentFID](StarVFS::FileID fid) ->bool {
		if (ParentFID != fid) {
			auto path = m_StarVFS->GetFilePath(fid, ParentFID);

			FileInfo fi;
			fi.m_IsFolder = m_StarVFS->IsFileDirectory(fid);
			fi.m_RelativeFileName = m_StarVFS->GetFilePath(fid, ParentFID);
			fi.m_FileName = m_StarVFS->GetFileName(fid);
			fi.m_FID = fid;
			FileTable.push_back(fi);
		}

		if (Recursive || ParentFID == fid) {
			auto handle = m_StarVFS->OpenFile(fid);
			if (!handle.EnumerateChildren(svfsfunc)) {
				handle.Close();
				return false;
			}
			handle.Close();
		}

		return true;
	};

	return svfsfunc(ParentFID);
}

bool MoonGlareFileSystem::EnumerateFolder(DataPath origin, FileInfoTable &FileTable, bool Recursive) {
	std::string path;
	DataSubPaths.Translate(path, origin);
	return EnumerateFolder(path, FileTable, Recursive);
}

bool MoonGlareFileSystem::EnumerateFolder(const string& SubPath, DataPath origin, FileInfoTable &FileTable, bool Recursive) {
	std::string path;
	DataSubPaths.Translate(path, SubPath, origin);
	return EnumerateFolder(path, FileTable, Recursive);
}

//-------------------------------------------------------------------------------------------------

bool MoonGlareFileSystem::OpenFile(const string& FileName, DataPath origin, StarVFS::ByteTable &FileData) {
	ASSERT(m_StarVFS);

	std::string path;
	DataSubPaths.Translate(path, FileName, origin);
	
	FileData.reset();
	auto fid = m_StarVFS->FindFile(path.c_str());
	if (!m_StarVFS->IsFileValid(fid)) {
		AddLogf(Warning, "Failed to find file: %s", path.c_str());
		return false;
	}
	if (m_StarVFS->IsFileDirectory(fid)) {
		AddLogf(Error, "Attempt to read directory: %s", path.c_str());
		return false;
	}
	if (!m_StarVFS->GetFileData(fid, FileData)) {
		AddLogf(Warning, "Failed to read file: (fid:%d) %s", fid, path.c_str());
		return false;
	}

	return true;
}

bool MoonGlareFileSystem::OpenResourceXML(XMLFile &doc, const string& Name, DataPath origin) {
	string buffer;
	buffer.reserve(128);
	switch (origin) {
	case DataPath::Models:
		buffer += Name;
		buffer += "/Model.xml";
		break;
	case DataPath::Maps:
		buffer += Name;
		buffer += "/Map.xml";
		break;
	case DataPath::Scenes:
	case DataPath::Shaders:
	case DataPath::Fonts:
	case DataPath::Objects:
	default:
		buffer += Name;
		buffer += ".xml";
		break;
	}
	return OpenXML(doc, buffer, origin);
}

bool MoonGlareFileSystem::OpenXML(XMLFile &doc, const string& FileName, DataPath origin) {
	ASSERT(m_StarVFS);
	
	doc.reset();
	StarVFS::ByteTable data;
	if (!OpenFile(FileName, origin, data)) {
		//already logged, no need for more
		return false;
	}

	doc = std::make_unique<pugi::xml_document>();
	auto result = doc->load_string(data.get());

	return static_cast<bool>(result);
}

//-------------------------------------------------------------------------------------------------

bool MoonGlareFileSystem::OpenTexture(Graphic::Texture &tex, const string& FileName, DataPath origin, bool ApplyDefaultSettings) {
	StarVFS::ByteTable data;
	if (!OpenFile(FileName, origin, data)) {
		//already logged, no need for more
		return false;
	}
	if (DataClasses::Texture::LoadTexture(tex, data.get(), data.size(), ApplyDefaultSettings)) {
		return true;
	}
	return false;
}

bool MoonGlareFileSystem::OpenTexture(TextureFile &tex, const string& FileName, DataPath origin, bool ApplyDefaultSettings) {
	StarVFS::ByteTable data;
	if (!OpenFile(FileName, origin, data)) {
		tex.reset();
		//already logged, no need for more
		return false;
	}
	tex = std::make_unique<Graphic::Texture>();
	if (DataClasses::Texture::LoadTexture(*tex.get(), data.get(), data.size(), ApplyDefaultSettings)) {
		return true;
	}
	tex.reset();
	return false;
}

//----------------------------------------------------------------------------------
/*/
bool MoonGlareFileSystem::EnumerateFolder(DataPath origin, FileTable &files) {
	string path;
	DataSubPaths.Translate(path, origin);
	return BaseClass::EnumerateFolder(path, files);
}

bool MoonGlareFileSystem::EnumerateFolder(DataPath origin, const string& subpath, FileTable &files) {
	string path;
	DataSubPaths.Translate(path, subpath, origin);
	return BaseClass::EnumerateFolder(path, files);
}
*/
//----------------------------------------------------------------------------------
/*
void MoonGlareFileSystem::OnModuleLoad(iContainer *container, unsigned LoadFlags) {
	BaseClass::OnModuleLoad(container, LoadFlags);

//	AddLog(TODO, "Check if container contain module metadata");

	auto module = std::make_unique<DataClasses::DataModule>(container);

	if (!GetDataMgr()->ImportModule(module)) {
		AddLog(Error, "Unable to import data  module");
	} else {
		AddLogf(Hint, "Container '%s' has been imported", container->GetFileName().c_str());
	}
}

InternalFileSystem::ContainerPrecheckStatus MoonGlareFileSystem::OnBeforeContainerAppend(iContainer *container, unsigned LoadFlags) {
	//querry module
	//TODO: ?	
	return ContainerPrecheckStatus::Append;
}
*/
//----------------------------------------------------------------------------------

GABI_IMPLEMENT_STATIC_CLASS(DirectoryReader);
RegisterApiDerivedClass(DirectoryReader, &DirectoryReader::RegisterScriptApi);

void DirectoryReader::RegisterScriptApi(ApiInitializer &api) {
	api
	.deriveClass<ThisClass, BaseClass>("cDirectoryReader")
		//.addFunction("GetDataReader", &ThisClass::GetDataReader)
	.endClass()
	;
}

} //namespace FileSystem 
} //namespace MoonGlare 
