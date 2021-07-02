// Based on https://github.com/xanderdunn/skaar/releases/tag/plugin3

#include <ShlObj.h>

#include "SkyrimEdition.h"

static PluginHandle	g_pluginHandle = kPluginHandle_Invalid;
static SKSEPapyrusInterface* g_papyrus = NULL;

extern "C" {

	bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info) {	// Called by SKSE to learn about this plugin and check that it's safe to load it
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\" LOG_FILE);
		gLog.SetPrintLevel(IDebugLog::kLevel_Error);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);


		_MESSAGE("Trying to load " MOD_NAME "...");

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = MOD_NAME;
		info->version = 1;

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");

			return false;
		}
		else if (skse->runtimeVersion < MINIMUM_VERSION || skse->runtimeVersion > MAXIMUM_VERSION)
		{
			_MESSAGE("unsupported runtime version %08X", skse->runtimeVersion);

			return false;
		}

		// ### do not do anything else in this callback
		// ### only fill out PluginInfo and return true/false

		// supported runtime version
		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface* skse) {	// Called by SKSE to load this plugin
		_MESSAGE(MOD_NAME " loaded");

		g_papyrus = (SKSEPapyrusInterface*)skse->QueryInterface(kInterface_Papyrus);

		//Check if the function registration was a success...
		//bool btest = g_papyrus->Register(RegisterFuncs);
		//
		//if (btest) {
		//	_MESSAGE("Papyrus functions registered");
		//}

		return true;
	}

};

