/*
 * cObjectList.cpp
 *
 *  Created on: 09-12-2013
 *      Author: Paweu
 */

#include <pch.h>
#include <MoonGlare.h>

namespace Core {
namespace Objects {

SPACERTTI_IMPLEMENT_CLASS_NOCREATOR(ObjectRegister)
RegisterApiDerivedClass(ObjectRegister, &ObjectRegister::RegisterScriptApi);

ObjectRegister::ObjectRegister() {
	m_Memory = std::make_unique<Memory>();
	Clear();
}

ObjectRegister::~ObjectRegister() {
	for (auto& it : m_Memory->m_ObjectPtr)
		it.reset();
	m_Memory.reset();
}

void ObjectRegister::RegisterScriptApi(ApiInitializer &api) {
//	api
//	.beginClass<ObjectList::ListTypeIterator>("cObjectSubListCIterator")
//		.addFunction("Next", &ObjectList::ListTypeIterator::Next)
//		.addFunction("Ended", &ObjectList::ListTypeIterator::Ended)
//		.addFunction("Index", &ObjectList::ListTypeIterator::Index)
//		.addFunction("Get", &ObjectList::ListTypeIterator::Get)
//	.endClass()
//	.beginClass<ObjectList>("cObjectSubList")
//		.addFunction("size", &ObjectList::isize)
//		.addFunction("GetIterator", &ObjectList::GetIterator)
//	.endClass();
}

void ObjectRegister::Clear() {
	for (auto& it: m_Memory->m_ObjectPtr) 
		it.reset();

	m_Memory->m_HandleAllocator.Clear();
	m_Memory->m_GlobalMatrix.fill(math::mat4());
	m_Memory->m_LocalMatrix.fill(math::mat4());
	Space::MemZero(m_Memory->m_Parent);

	auto h = m_Memory->m_HandleAllocator.Allocate();
	m_Memory->m_HandleAllocator.SetMapping(h, 0);
	m_Memory->m_ObjectPtr[0] = std::make_unique<Object>();
	m_Memory->m_ObjectPtr[0]->SetSelfHandle(h);
}

Handle ObjectRegister::GetRootHandle() {
	auto h = m_Memory->m_HandleAllocator.GetHandleForIndex(0);
	h.SetType(Configuration::Handle::Types::Object);
	return h;
}

Handle ObjectRegister::NewObject(Handle Parent) {
	return Insert(std::make_unique<Object>(), Parent);
}

Handle ObjectRegister::Insert(std::unique_ptr<Object> obj, Handle Parent) {
	ASSERT_HANDLE_TYPE(Object, Parent);

	if (!m_Memory->m_HandleAllocator.IsHandleValid(Parent)) {
		AddLog(Error, "Parent handle is not valid!");
		return Handle();
	}

	auto h = m_Memory->m_HandleAllocator.Allocate();
	if (!m_Memory->m_HandleAllocator.IsHandleValid(h)) {
		AddLog(Error, "No more space!");
		return Handle();
	}
	auto index = m_Memory->m_HandleAllocator.Allocated() - 1;
	m_Memory->m_HandleAllocator.SetMapping(h, index);

	h.SetType(Configuration::Handle::Types::Object);

	obj->SetSelfHandle(h);
	obj->SetOwnerRegister(this);
	m_Memory->m_ObjectPtr[index].swap(obj);
	m_Memory->m_Parent[index] = Parent;
	m_Memory->m_HandleIndex[index] = h.GetIndex();

	return h;
}

void ObjectRegister::Release(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);

	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return;
	}

	m_Memory->m_ObjectPtr[idx]->SetSelfHandle(Handle());
	m_Memory->m_ObjectPtr[idx]->SetOwnerRegister(nullptr);
	m_Memory->m_ObjectPtr[idx].release();
	m_Memory->m_Parent[idx] = Handle();
	m_Memory->m_HandleAllocator.Free(h);

	Reorder(idx);
}

void ObjectRegister::Remove(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);

	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return;
	}

	m_Memory->m_ObjectPtr[idx].reset();
	m_Memory->m_Parent[idx] = Handle();
	m_Memory->m_HandleAllocator.Free(h);
	Reorder(idx);
}

void ObjectRegister::Reorder(size_t start) {

	size_t spread = 1;
	for (size_t i = start; i < m_Memory->m_HandleAllocator.Allocated(); ) {

		size_t other = i + spread;
		__debugbreak();
		auto OtherParentHandle = m_Memory->m_Parent[other];

		if (m_Memory->m_HandleAllocator.IsHandleValid(OtherParentHandle)) {
			m_Memory->m_GlobalMatrix[i] = m_Memory->m_GlobalMatrix[other];
			m_Memory->m_LocalMatrix[i] = m_Memory->m_LocalMatrix[other];
			m_Memory->m_Parent[i] = m_Memory->m_Parent[other];
			m_Memory->m_ObjectPtr[i] = std::move(m_Memory->m_ObjectPtr[other]);
			 
			size_t OtherHandleIdx = m_Memory->m_HandleIndex[other];
			m_Memory->m_HandleIndex[i] = OtherHandleIdx;
			m_Memory->m_HandleAllocator.SetMapping(OtherHandleIdx, i);

			i += spread;
			continue;
		} 
		else
		{
			m_Memory->m_ObjectPtr[i].reset();
			auto SelfHandleIdx = m_Memory->m_HandleIndex[i];
			auto h = m_Memory->m_HandleAllocator.GetHandleForIndex(SelfHandleIdx);
			m_Memory->m_HandleAllocator.Free(h);
			++spread;
			continue;
		}
	}
}

Handle ObjectRegister::GetParentHandle(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return Handle();
	}
	return m_Memory->m_Parent[idx];
}

Object *ObjectRegister::Get(Handle h) {
	ASSERT_HANDLE_TYPE(Object, h);
	size_t idx;
	if (!m_Memory->m_HandleAllocator.GetMapping(h, idx)) {
		AddLog(Warning, "Invalid handle!");
		return nullptr;
	}
	return m_Memory->m_ObjectPtr[idx].get();
}

bool ObjectRegister::InitializeObjects() {
	bool ret = true;
	for (auto &it : *this)
		ret &= it->Initialize();
	return ret;
}

Handle ObjectRegister::LoadObject(Handle Parent, xml_node MetaXML, GameScene *OwnerScene) {
	auto objH = NewObject(Parent);
	Object *obj = Get(objH);
	obj->SetOwnerScene(OwnerScene);

	if (!obj->LoadPattern(MetaXML)) {
		AddLogf(Error, "An Error has occur during loading meta of predef object (%s)", MetaXML.attribute("Name").as_string(ERROR_STR));
		Remove(objH);
		return Handle();
	}

	for (xml_node it = MetaXML.child("Child"); it; it = it.next_sibling("Child")) {
		auto h = LoadObject(objH, it, OwnerScene);
		Object *childobj = Get(objH);
		if (!childobj) {
			AddLogf(Error, "Loading predef object child failed (%s)", it.attribute("Name").as_string(ERROR_STR));
			continue;
		}
	}

	return objH;
}

Handle ObjectRegister::LoadObject(const std::string &Name, GameScene *OwnerScene, Handle Parent) {
	FileSystem::XMLFile Meta;
	if (!GetFileSystem()->OpenResourceXML(Meta, Name, DataPath::Objects)) {
		AddLogf(Error, "Unable to open master resource xml for object '%s'", Name.c_str());
		return Handle();
	}

	if (!Get(Parent)) {
		Parent = GetRootHandle();
	}

	auto objH = LoadObject(Parent, Meta->document_element(), OwnerScene);
	Object *obj = Get(objH);
	if (!obj) {
		AddLogf(Error, "Loading predef object failed: '%s'", Name.c_str());
		Remove(objH);
		return Handle();
	}

	return objH;
}

bool ObjectRegister::LoadObjects(const xml_node SrcNode, GameScene *OwnerScene) {
	if (!SrcNode) 
		return true;

	auto RootHandle = GetRootHandle();

	for (xml_node itnode = SrcNode.child("Item"); itnode; itnode = itnode.next_sibling("Item")) {
		const char* name = itnode.attribute(xmlAttr_Object).as_string(0);
		if (!name) {
			AddLog(Error, "An object definition without predef object name!");
			continue;
		}

		auto objH = LoadObject(name, OwnerScene, RootHandle);
		Object *obj = Get(objH);
		if (!obj) {
			//already logged
			continue;
		}

		if (!obj->LoadDynamicState(itnode)) {
			AddLog(Error, "Unable to load dynamic state of object. Pattern: '" << name << "'");
			Remove(objH);
			continue;
		}
	}

	return true;
}

void ObjectRegister::Process(const MoveConfig &conf) {
	for (size_t i = 1, j = m_Memory->m_HandleAllocator.Allocated(); i < j; ++i) {

		size_t pid;
		if (!m_Memory->m_HandleAllocator.GetMapping(m_Memory->m_Parent[i], pid)) {
			continue;
		}

		auto obj = m_Memory->m_ObjectPtr[i].get();

		auto &gm = m_Memory->m_GlobalMatrix[i];
		auto &lm = m_Memory->m_LocalMatrix[i];

		obj->GetPositionTransform().getOpenGLMatrix((float*)&lm);
		gm = m_Memory->m_GlobalMatrix[pid] * lm;

		if (obj->GetMoveController())
			obj->GetMoveController()->DoMove(conf);
		if (obj->GetLightSource()) {
			auto *l = obj->GetLightSource();
			l->Update();
		}

		if (obj->GetVisible() && obj->GetModel()) {
			float scale = obj->GetEffectiveScale();
			auto sgm = gm;
			sgm[0] *= scale;
			sgm[1] *= scale;
			sgm[2] *= scale;
			conf.RenderList.push_back(std::make_pair(sgm, obj->GetModel()));
		}
	}
}

} //namespace Objects
} //namespace Core
