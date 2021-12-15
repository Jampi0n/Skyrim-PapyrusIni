#pragma once

#include "SkyrimEdition.h"
#if LEGENDARY_EDITION
#include "skse/PapyrusNativeFunctions.h"
#else
#include "skse64/PapyrusNativeFunctions.h"
#endif

#define PLUGIN_VERSION 1

namespace PapyrusIni
{
	bool RegisterFuncs(VMClassRegistry* registry);
}
