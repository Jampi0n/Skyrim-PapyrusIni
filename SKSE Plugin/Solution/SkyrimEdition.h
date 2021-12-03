#pragma once

#define COMPATIBLE_VERSION CURRENT_RELEASE_RUNTIME

#if __has_include("skse/PluginAPI.h")
#define LEGENDARY_EDITION 1
#include "skse/PluginAPI.h"
#include "skse/skse_version.h"
#define MOD_NAME "PapyrusIniLE"
#define LOG_FILE "Skyrim\\SKSE\\" MOD_NAME ".log"

#else
#define LEGENDARY_EDITION 0
#include "skse64/PluginAPI.h"
#include "skse64_common/skse_version.h"
#define MOD_NAME "PapyrusIniSE"
#define LOG_FILE "Skyrim Special Edition\\SKSE\\" MOD_NAME ".log"
#if SKSE_VERSION_INTEGER > 2 || (SKSE_VERSION_INTEGER == 2 && SKSE_VERSION_INTEGER_MINOR > 1) || (SKSE_VERSION_INTEGER == 2 && SKSE_VERSION_INTEGER_MINOR == 1 && SKSE_VERSION_INTEGER_BETA >= 2)
#define NEW_VERSION_CHECK 1
#else
#define NEW_VERSION_CHECK 0
#endif
#endif
