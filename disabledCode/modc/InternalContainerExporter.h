/*
  * Generated by cppsrc.sh
  * On 2015-03-29 16:03:06,65
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once
#ifndef InternalContainer_H
#define InternalContainer_H

namespace modc {

struct InternalContainerFileNode;
using InternalContainerFileNodePtr = std::unique_ptr < InternalContainerFileNode > ;
struct InternalContainerFileNode {
	bool Directory = false;
	bool Stored = false;
	string Name;
	MoonGlare::FileSystem::FileSize Size = 0;
	string DataName;
	std::unique_ptr<char[]> Data;
	InternalContainerFileNode *Parent = nullptr;

	std::list<InternalContainerFileNodePtr> Children;

	const unsigned FileIndex;

	InternalContainerFileNode(): FileIndex(_Counter.fetch_add(1)) { 
	}
	//InternalContainerFileNode(const InternalContainerFileNode&) = delete;
	//InternalContainerFileNode(InternalContainerFileNode &&o) = default;

	string Path() const {
		std::stack <const InternalContainerFileNode*> s;
		auto *p = this;
		while (p) {
			if (p->Parent)
				s.push(p);
			p = p->Parent;
		}
		std::ostringstream ss;
		while (!s.empty()) {
			auto p = s.top();
			s.pop();
			ss << p->Name;
			if (p->Directory)
				ss << "/";
		}
		return ss.str();
	}

protected:
	static std::atomic<unsigned> _Counter;
};

struct InternalContainerSettings {
	std::list<string> NamespaceLocation;

	void Parse(SettingsMap &data);
};

class InternalContainer : public MoonGlare::FileSystem::iContainer {
	GABI_DECLARE_STATIC_CLASS(InternalContainer, MoonGlare::FileSystem::iContainer);
	DISABLE_COPY();
public:
 	InternalContainer(const string &FileName, const InternalContainerSettings &settings);
 	virtual ~InternalContainer();

	virtual MoonGlare::FileSystem::FileWritter GetFileWritter(const string& file) override;

	bool StoreFile(InternalContainerFileNode *file);

	//virtual bool FileExists(const string& file) const override;
	//virtual bool EnumerateFolder(const string &Path, FolderEnumerateFunc func) const override;
private: 
	string m_FileName;
	std::ofstream m_File;
	InternalContainerSettings m_Settings;

	InternalContainerFileNodePtr m_Root;

	void CloseContainer();

	InternalContainerFileNode* FindOrAlloc(const string& FileName);
};

} //namespace modc 

#endif
