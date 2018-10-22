/*
  * Generated by cppsrc.sh
  * On 2016-10-09 20:20:47,90
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef iFileIconInfo_H
#define iFileIconInfo_H

#include <ToolBase/Module.h>

namespace MoonGlare {
namespace QtShared {

class iFileIconInfo {
public:
	virtual ~iFileIconInfo() { }

	struct FileIconInfo {
		std::string m_Ext;
		std::string m_Icon;
	};

	virtual std::vector<FileIconInfo> GetFileIconInfo() const { return{}; }
};

class FileIconProvider : public iModule {
public:
	FileIconProvider(SharedModuleManager modmgr);
	bool PostInit() override;

	const std::string& GetExtensionIcon(const std::string &ext) const;
	const std::string& GetExtensionIcon(const std::string &ext, const std::string& default) const;
private:
	std::unordered_map<std::string, std::string> m_FileIconMap;
};

} //namespace QtShared 
} //namespace MoonGlare 

#endif
