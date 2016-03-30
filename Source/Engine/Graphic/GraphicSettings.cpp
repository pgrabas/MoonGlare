/*
  * Generated by cppsrc.sh
  * On 2015-08-07 15:45:53,58
  * by Paweu
*/
/*--END OF HEADER BLOCK--*/

#include <pch.h>
#include <MoonGlare.h>
#include "GraphicSettings.h"

namespace Graphic {

struct GraphicSettingsInitializer {
	GraphicSettingsInitializer() {
		::Settings.RegisterDynamicSetting<Settings_t::BufferedSettingManipulator<GraphicSettings::FullScreen>>("Graphic.FullScreen", true, SettingsGroup::None);
		::Settings.RegisterDynamicSetting<Settings_t::BufferedSettingManipulator<GraphicSettings::Monitor>>("Graphic.Monitor", true, SettingsGroup::None);
		::Settings.RegisterDynamicSetting<Settings_t::BufferedSettingManipulator<GraphicSettings::Width>>("Graphic.Width", true, SettingsGroup::None);
		::Settings.RegisterDynamicSetting<Settings_t::BufferedSettingManipulator<GraphicSettings::Height>>("Graphic.Height", true, SettingsGroup::None);
	}
};

static GraphicSettingsInitializer _Initializer;

} //namespace Graphic 

