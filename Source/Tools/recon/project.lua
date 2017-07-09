require "premake-qt/qt"
local qt = premake.extensions.qt

group "Tools"
    project "recon"
        kind "WindowedApp"
        
        qt.enable()
        qtpath(MoonGlare.GetBuildSetting({name = "qtPath", group="Qt"}))
    --	qtheaderpath(settings.qt.headers)"d:/Programowanie/Qt/5.5/msvc2013/"
        qtprefix "Qt5"
        qtmodules { "core", "gui", "widgets", "network" }
    --	qtlibsuffix ""

        SetPCH { hdr = "Tools/recon/pch.h", src = "pch.cpp", }	
        
        defines { 
            "_BUILDING_TOOL_", 
            "_BUILDING_RECON_", 
        }
        includedirs {
            "../Shared/Qt/",
        }
        files { 
            "*",
            "../Shared/**.rc",
            "../Shared/Qt/lua/*",
        }	
    