#pragma once

#include <StarVFS/core/nfStarVFS.h>

namespace MoonGlare {

struct FileInfo {
    std::string m_FileName;
    std::string m_RelativeFileName;
    StarVFS::FileID m_FID;
    bool m_IsFolder;
};
using FileInfoTable = std::vector<FileInfo>;

using XMLFile = std::unique_ptr<pugi::xml_document>;

class iFileSystem {
protected:
    virtual ~iFileSystem() {};
public:
    virtual bool OpenFile(StarVFS::ByteTable &FileData, StarVFS::FileID fid) = 0;
    virtual bool OpenFile(StarVFS::ByteTable &FileData, const std::string& uri) = 0;

    template<typename T>
    bool OpenXML(XMLFile &doc, const T &file) {
        doc.reset();
        StarVFS::ByteTable data;
        if (!OpenFile(data, file))
            return false;

        doc = std::make_unique<pugi::xml_document>();
        auto result = doc->load_string((char*)data.get());

        return static_cast<bool>(result);
    }

    virtual bool EnumerateFolder(const std::string& Path, FileInfoTable &FileTable, bool Recursive) = 0;

    virtual void FindFilesByExt(const char *ext, StarVFS::DynamicFIDTable &out) = 0;

    virtual std::string GetFileName(StarVFS::FileID fid) const = 0;
    virtual std::string GetFullFileName(StarVFS::FileID fid) const = 0;

};

}