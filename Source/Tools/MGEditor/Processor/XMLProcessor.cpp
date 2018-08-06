/*
  * Generated by cppsrc.sh
  * On 2016-10-13 18:53:51,09
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/
#include PCH_HEADER

#include <boost/algorithm/string.hpp>

#include <icons.h>
#include <iFileProcessor.h>
#include <iFileIconProvider.h>
#include <iCustomEnum.h>
#include <Module.h>
#include <iIssueReporter.h>

#include "../FileSystem.h"

#include <pugixml.hpp>

namespace MoonGlare::Editor:: Processor {
       
struct XMLProcessor
    : public QtShared::iFileProcessor {

    XMLProcessor(QtShared::SharedModuleManager modmgr, QtShared::SharedSetEnum xmlEnum, std::string URI) : QtShared::iFileProcessor(std::move(URI)), xmlEnum(xmlEnum), moduleManager(modmgr) { }

    std::string MakeIssueId() {
        return "XMLProcessor|" + m_URI + "|Error";
    }

    ProcessResult ProcessFile() override {
        auto fs = moduleManager->QuerryModule<FileSystem>();
        StarVFS::ByteTable bt;
        if (!fs->GetFileData(m_URI, bt)) {
            //todo: log sth
            throw std::runtime_error("Unable to read file: " + m_URI);
        }
        if (bt.byte_size() == 0) {
            //todo: log sth
        }

        pugi::xml_document xdoc;

        auto result = xdoc.load_buffer(bt.get(), bt.byte_size());

        auto reporter = moduleManager->QuerryModule<QtShared::IssueReporter>();
        if(result) {
            reporter->DeleteIssue(MakeIssueId());
        } else {
            QtShared::Issue issue;
            issue.fileName = m_URI;
            issue.message = result.description();
            issue.type = QtShared::Issue::Type::Error;
            issue.group = "xml";
            issue.internalID = MakeIssueId();

            int line = 1, column = 0;
            for (size_t pos = 0; pos < bt.byte_size(); ++pos) {
                if (pos == result.offset) {
                    break;
                }
                if (bt.get()[pos] == '\n') {
                    ++line; 
                    column = 0;
                } else {
                    ++column;
                }
            }
            issue.sourceColumn = column;
            issue.sourceLine = line;

        //    std::regex pieces_regex(R"==(\*\s*Line\s*(\d+)\s*,\s*Column\s*(\d+)\s*(.*))==", std::regex::icase);
        //    std::smatch pieces_match;
        //    if (std::regex_match(errs, pieces_match, pieces_regex)) {
        //        issue.sourceLine = std::strtol(pieces_match[1].str().c_str(), nullptr, 10);
        //        issue.sourceColumn = std::strtol(pieces_match[2].str().c_str(), nullptr, 10);
        //        issue.message = pieces_match[3];
        //    }

            reporter->ReportIssue(std::move(issue));
        }

        xmlEnum->Add(m_URI);

        return ProcessResult::Success;
    }
private:
    QtShared::SharedSetEnum xmlEnum;
    QtShared::SharedModuleManager moduleManager;
};

//----------------------------------------------------------------------------------

struct XMLProcessorModule
    : public QtShared::iModule
    , public QtShared::iFileProcessorInfo
    , public QtShared::iCustomEnumSupplier
    , public QtShared::iFileIconInfo {

    XMLProcessorModule(SharedModuleManager modmgr) : iModule(std::move(modmgr)) {}

    QtShared::SharedSetEnum xmlEnum = std::make_shared<QtShared::SetEnum>("");

    QtShared::SharedFileProcessor CreateFileProcessor(std::string URI) override {
        return std::make_shared<XMLProcessor>(GetModuleManager(), xmlEnum, std::move(URI));
    }

    std::vector<std::string> GetSupportedTypes() {
        return{ "xml", "sdx", "ebx", };
    }

    std::vector<std::shared_ptr<QtShared::iCustomEnum>> GetCustomEnums() const override  {
        return {/* xmlEnum, */};
    }

    std::vector<FileIconInfo> GetFileIconInfo() const override {
        return std::vector<FileIconInfo>{
            FileIconInfo{ "xml", ICON_16_CONFIG_FILE, },
        };
    }
};

QtShared::ModuleClassRgister::Register<XMLProcessorModule> XMLProcessorModuleReg("XMLProcessorModule");

//----------------------------------------------------------------------------------
} 