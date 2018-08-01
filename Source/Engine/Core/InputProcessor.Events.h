#pragma once

namespace MoonGlare::Core {

struct InputProcessorOnCharEvent {
    static constexpr char* EventName = "Char";
    static constexpr char* HandlerName = "OnCharEvent";
   
    Entity recipient;
    int m_Char;

    friend std::ostream& operator<<(std::ostream& out, const InputProcessorOnCharEvent & e) {
        out << "InputProcessorOnCharEvent"
            << "[char:" << (int)e.m_Char << ", "
            << "recipient:" << e.recipient
            << "]";
        return out;
    }

    static ApiInitializer RegisterScriptApi(ApiInitializer api) {
        return api
            .beginClass<InputProcessorOnCharEvent>("InputProcessorOnCharEvent")
                .addData("Recipient", &InputProcessorOnCharEvent::recipient, false)
                .addData("char", &InputProcessorOnCharEvent::m_Char, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

struct InputProcessorOnKeyEvent {
    static constexpr char* EventName = "Key";
    static constexpr char* HandlerName = "OnKeyEvent";

    Entity recipient;
    int m_Key;

    friend std::ostream& operator<<(std::ostream& out, const InputProcessorOnKeyEvent & e) {
        out << "InputProcessorOnKeyEvent"
            << "[key:" << (int)e.m_Key << ", "
            << "recipient:" << e.recipient
            << "]";
        return out;
    }

    static ApiInitializer RegisterScriptApi(ApiInitializer api) {
        return api
            .beginClass<InputProcessorOnKeyEvent>("InputProcessorOnKeyEvent")
                .addData("Recipient", &InputProcessorOnKeyEvent::recipient, false)
                .addData("key", &InputProcessorOnKeyEvent::m_Key, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};

} //namespace MoonGlare::Core

