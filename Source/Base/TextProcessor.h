/*
  * Generated by cppsrc.sh
  * On 2016-10-12 18:39:40,06
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#pragma once

namespace MoonGlare::Resources { class StringTables; }

namespace MoonGlare {

class TextProcessor {
public:
    TextProcessor(Resources::StringTables *Tables = nullptr);
    ~TextProcessor();

    void SetTables(Resources::StringTables *Tables) { stringTables = Tables; }
    void Process(const std::string& input, std::string &out);
protected:
    Resources::StringTables *stringTables;
};

} //namespace MoonGlare 