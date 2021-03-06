/*
  * Generated by cppsrc.sh
  * On 2016-07-21 20:17:41,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include PCH_HEADER
#include <qobject.h>
#include "DataModule.h"
#include <boost/algorithm/string.hpp>

namespace MoonGlare {
namespace Editor {
namespace Module {

std::shared_ptr<DataModule> DataModule::NewModule(const std::string & BaseDirectrory) {
	auto ptr = std::make_shared<DataModule>(BaseDirectrory);
	if (!ptr->Create())
		return nullptr;
	return ptr;
}

std::shared_ptr<DataModule> DataModule::OpenModule(const std::string & BaseDirectrory) {
	auto ptr = std::make_shared<DataModule>(BaseDirectrory);
	if (!ptr->Open())
		return nullptr;
	return ptr;
}

DataModule::DataModule(const std::string & BaseDirectrory)
	: baseDirectrory(BaseDirectrory) {

    boost::replace_all(baseDirectrory, "\\", "/");
    if (baseDirectrory.back() != '/')
        baseDirectrory += '/';
}

DataModule::~DataModule() {
}

bool DataModule::Create() {
	SetChanged();
	return true;
}

bool DataModule::Open() {
    return true;
}

bool DataModule::DoPeriodicSave() {
    return true;
}

} //namespace Module 
} //namespace Editor 
} //namespace MoonGlare 
