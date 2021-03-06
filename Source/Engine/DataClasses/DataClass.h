#ifndef DataClassH
#define DataClassH

namespace MoonGlare {
namespace DataClasses {

class BasicResource {
public:
	BasicResource();
	BasicResource(const string& Name);
	static void RegisterScriptApi(ApiInitializer &api);

    const string& GetName() const { return m_Name; }
    void SetName(const string& name) { m_Name = name; }
    const char* GetCharName() const { return GetName().c_str(); }
protected:
	XMLFile OpenMetaData() const;
private:
    string m_Name;
};

/** basic class for data containers like font */
class DataClass : public BasicResource {
public:
	virtual ~DataClass();
	DataClass();
	DataClass(const string& Name);

	virtual bool Initialize();
	virtual bool Finalize();

	DefineFlagGetter(m_Flags, Flags::Ready, Ready);
	DefineFlagGetter(m_Flags, Flags::Loaded, Loaded);
protected:
	unsigned m_Flags;

	virtual bool ReadNameFromXML(const xml_node Node);

	virtual bool DoInitialize();
	virtual bool DoFinalize();
	
	struct Flags {
		enum {
			Ready		= 0x0001,
			Loaded		= 0x0002,
		};
	};

	DefineFlagSetter(m_Flags, Flags::Ready, Ready);
	DefineFlagSetter(m_Flags, Flags::Loaded, Loaded);
};

} // namespace DataClasses
} //namespace MoonGlare 

#endif
