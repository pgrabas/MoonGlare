/*
  * Generated by cppsrc.sh
  * On 2015-06-28 11:37:08.64
  * by Immethis
*/
/*--END OF HEADER BLOCK--*/
#include <pch.h>
#include <nfMoonGlare.h>
#include <Engine/Modules/iModule.h>
#include <Foundation/MoonGlareInsider.h>
#include <Core/Scripts/ScriptEngine.h>

#include <Foundation/InterfaceMap.h>
#include <Foundation/Settings.h>

#define _WIN32_WINNT 0x0502
#include <boost/asio.hpp>

namespace MoonGlare::Modules {

using namespace Debug::InsiderApi;

struct RemoteConsoleModule : public iModule {
    RemoteConsoleModule(World *world) : iModule(world) { }

    ~RemoteConsoleModule() {
        m_Running = false;
        m_ioservice.stop();
        if (m_Thread.joinable())
            m_Thread.join();
    }

    virtual std::string GetName() const { return "RemoteConsole"; };

    virtual void OnPostInit() {
        auto stt = GetWorld()->GetInterface<Settings>();
        if (stt) {
            enabled = stt->GetBool("RemoteConsole.Enabled", enabled);
            port = (uint16_t)stt->GetInt("RemoteConsole.Port", port);
        }

        if (enabled) {
            m_Thread = std::thread(&RemoteConsoleModule::ThreadEntry, this);
        }
    }
private:
    using udp = boost::asio::ip::udp;

    bool enabled = false;
    uint16_t port = Debug::InsiderApi::Configuration::recon_Port;

    std::thread m_Thread;
    bool m_Running;
    boost::asio::io_service m_ioservice;

    void ThreadEntry() {
        ::OrbitLogger::ThreadInfo::SetName("RECO");
        AddLog(Info, "RemoteConsole Thread started");

        char buffer[Debug::InsiderApi::Configuration::MaxMessageSize];
        auto *header = reinterpret_cast<MessageHeader*>(buffer);

        udp::socket sock(m_ioservice, udp::endpoint(udp::v4(), port));

        AddLog(Hint, "Remote console initialized");
        while (m_Running) {
            try {
                if (sock.available() <= 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }
                udp::endpoint remote_endpoint;
                boost::system::error_code error;
                sock.receive_from(boost::asio::buffer(buffer, sizeof(buffer) - 1), remote_endpoint, 0, error);

                if (error && error != boost::asio::error::message_size)
                    continue;

                switch (header->MessageType) {
                case MessageTypes::ExecuteCode: {
                    AddLogf(Info, "Received lua command. Size: %d bytes. Data: %s ", header->PayloadSize, header->PayLoad);
                    MoonGlare::Core::GetScriptEngine()->ExecuteCode((char*)header->PayLoad, header->PayloadSize - 1, "RemoteConsole");
                    break;
                }
                default:
                    AddLogf(Info, "Unknown command. Size: %d bytes, type: %d ", header->PayloadSize, header->MessageType);
                }
            }
            catch (...) {
                __debugbreak();
            }
        }
    }
};

#ifdef DEBUG_SCRIPTAPI
ModuleClassRegister::Register<RemoteConsoleModule> RemoteConsoleModuleReg("RemoteConsoleModule");
#endif

} //namespace MoonGlare::Modules
