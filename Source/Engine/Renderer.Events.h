#pragma once

namespace MoonGlare::Renderer {

struct RendererResourceLoaderEvent {
    static constexpr char* EventName = "RendererResourceLoaderEvent";
    static constexpr char* HandlerName = "OnRendererResourceLoaderEvent";
    static constexpr bool Public = false;

    bool busy;
    //char __padding[4 - sizeof(busy)];
    int revision;

    friend std::ostream& operator<<(std::ostream& out, const RendererResourceLoaderEvent & dt) {
        out << "RendererResourceLoaderEvent"
            << "[busy:" << (int)dt.busy
            << ",revision:" << dt.revision
            << "]";
        return out;
    }

    static ApiInitializer RegisterScriptApi(ApiInitializer api) {
        return api
            .beginClass<RendererResourceLoaderEvent>("RendererResourceLoaderEvent")
                .addData("Busy", &RendererResourceLoaderEvent::busy, false)
                .addData("Revision", &RendererResourceLoaderEvent::revision, false)
                .addStaticString("EventName", EventName)
                .addStaticString("HandlerName", HandlerName)
            .endClass();
    }
};


}

