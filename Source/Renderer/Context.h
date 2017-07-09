/*
  * Generated by cppsrc.sh
  * On 2017-02-10 21:51:36,38
  * by Kalessin
*/
/*--END OF HEADER BLOCK--*/

#pragma once

#include "nfRenderer.h"
#include "Configuration.Renderer.h"
#include "iContext.h"

namespace MoonGlare::Renderer {

class 
    //alignas(16)
    Context final : public iContext {
    using Conf = Configuration::Context;
public:
    struct VideoMode {
        int m_Width;
        int m_Height;
        int m_RefreshRate;
        int _padding;
    };

    Context();
    ~Context();

    static bool InitializeSubSystem();
    static bool FinalizeSubSystem();

    static int GetMonitorCount();
    static std::vector<VideoMode> GetMonitorModes(int MonitorIndex);
    static void DumpMonitors();

    //IContext
    void EnterCharMode() override;
    void ExitCharMode() override;
    void HookMouse() override;
    void ReleaseMouse() override;
    emath::ivec2 GetSize() const override;
    void SetInputHandler(iContextInputHandler *iph) override;
    void SetPosition(const emath::ivec2 &pos) override ;
    void SetTitle(const char* Title) override;
    unsigned GetRefreshRate() const override;
    void SetVisible(bool value) override;


    void MakeCurrent();
    void Flush();

    void Initialize(const ContextCreationInfo &ctxifo, RendererFacade *renderer, RenderDevice *device);
    void InitializeWindowLayer(Commands::CommandQueue &q, Frame* frame);
    void Finalize();

    GLFWwindow* GetHandle() {
        return m_Window;
    }

    void Process();

    emath::fvec2 CursorPos();

    void CaptureScreenShot();
private:
    emath::ivec2 m_Size;
    GLFWwindow *m_Window = nullptr;
    RenderDevice *m_Device = nullptr;
    RendererFacade *m_Renderer = nullptr;
    iContextInputHandler *m_InputHandler = nullptr;

    bool m_CharMode : 1;
    bool m_MouseHooked : 1;
    bool m_Focused : 1;
    emath::fvec2 m_LastMousePos;

    bool CreateWindow(ContextCreationInfo ctxifo);

    static bool s_GLFWInitialized;
    static void GLFW_ErrorCallback(int error, const char* description);
    static void GLFW_CloseCallback(GLFWwindow* window);
    static void GLFW_FocusCallback(GLFWwindow* window, int focus);
    static void GLFW_CursorCallback(GLFWwindow *window, double x, double y);
    static void GLFW_MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
    static void GLFW_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void GLFW_CharModeCharCallback(GLFWwindow* window, unsigned int key);
    static void GLFW_CharModeKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void GLFW_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

//static_assert((sizeof(Context) % 16) == 0, "Invalid size!");

} //namespace MoonGlare::Renderer 
