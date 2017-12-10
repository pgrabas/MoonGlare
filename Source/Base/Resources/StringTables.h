/*
  * Generated by cppsrc.sh
  * On 2015-03-08 19:20:50,34
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once

namespace MoonGlare {
class iFileSystem;
}

namespace MoonGlare::Resources {

class StringTables {
public:
    StringTables(iFileSystem *fs);
    virtual ~StringTables();

    const std::string_view GetString(const string& id, const string& Table) { return GetString(std::string_view(id), std::string_view(Table)); }

    const std::string_view GetString(const std::string_view id, const std::string_view Table);


    void SetLangCode(std::string code);
    void Clear();
    //static void RegisterScriptApi(ApiInitializer &api);
protected:
    iFileSystem *fileSystem = nullptr;
    std::string langCode;
    struct StringTableInfo;

    using StringTableMap = std::map < string, StringTableInfo, std::less<> > ;
    StringTableMap m_TableMap;

    void InitInternalTable();

    bool Load(std::string_view TableName);
};

} //namespace MoonGlare::Resources
