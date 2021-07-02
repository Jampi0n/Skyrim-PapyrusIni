#include "PapyrusIni.h"

#include <vector>
#include <cmath>
#include <stdexcept>

#if LEGENDARY_EDITION
#include "skse/GameReferences.h"
#include "skse/GameObjects.h"
#include "skse/PapyrusSpell.h"
#else
#include "skse64/GameReferences.h"
#include "skse64/GameObjects.h"
#include "skse64/PapyrusSpell.h"
#endif

#include <ShlObj.h>
#include <WinBase.h>

#define BUFFER_SIZE 128
#define PAPYRUS_FUNCTION StaticFunctionTag* base
#define INI_PARAMS PAPYRUS_FUNCTION, BSFixedString file, BSFixedString category, BSFixedString name
#define FILE ("Data\\" + std::string(file.data)).c_str()

namespace PapyrusIni {

	void WriteInt(INI_PARAMS, SInt32 value) {
		WritePrivateProfileStringA(category.data, name.data, std::to_string(value).c_str(), FILE);
	}

	SInt32 ReadInt(INI_PARAMS, SInt32 def) {
		_MESSAGE("ReadInt 1");
		return GetPrivateProfileIntA(category.data, name.data, def, FILE);
	}

	void WriteFloat(INI_PARAMS, float value) {
		WritePrivateProfileStringA(category.data, name.data, std::to_string(value).c_str(), FILE);
	}

	float ReadFloat(INI_PARAMS, float def) {
		char inBuf[BUFFER_SIZE];
		GetPrivateProfileStringA(category.data, name.data, std::to_string(def).c_str(), inBuf, BUFFER_SIZE, FILE);
		float value = def;
		try {
			value = std::stof(inBuf);
		}
		catch (std::invalid_argument) {
			value = def;
		}
		return value;
	}

	void WriteString(INI_PARAMS, BSFixedString value) {
		WritePrivateProfileStringA(category.data, name.data, ("\"" + std::string(value.data) + "\"").c_str(), FILE);
	}

	BSFixedString ReadString(INI_PARAMS, BSFixedString def) {
		char inBuf[BUFFER_SIZE];
		GetPrivateProfileStringA(category.data, name.data, def.data, inBuf, BUFFER_SIZE, FILE);
		return BSFixedString(inBuf);
	}

	void WriteBool(INI_PARAMS, bool value) {
		WritePrivateProfileStringA(category.data, name.data, value ? "1" : "0", FILE);
	}

	bool ReadBool(INI_PARAMS, bool def) {
		return GetPrivateProfileIntA(category.data, name.data, def ? 1 : 0, FILE) == 1;
	}


	bool RegisterFuncs(VMClassRegistry* registry) {
		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, SInt32>("WriteInt", "PapyrusIni", WriteInt, registry));
		registry->SetFunctionFlags("PapyrusIni", "WriteInt", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, float>("WriteFloat", "PapyrusIni", WriteFloat, registry));
		registry->SetFunctionFlags("PapyrusIni", "WriteFloat", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, bool>("WriteBool", "PapyrusIni", WriteBool, registry));
		registry->SetFunctionFlags("PapyrusIni", "WriteBool", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("WriteString", "PapyrusIni", WriteString, registry));
		registry->SetFunctionFlags("PapyrusIni", "WriteString", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, SInt32, BSFixedString, BSFixedString, BSFixedString, SInt32>("ReadInt", "PapyrusIni", ReadInt, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadInt", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, float, BSFixedString, BSFixedString, BSFixedString, float>("ReadFloat", "PapyrusIni", ReadFloat, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadFloat", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString, bool>("ReadBool", "PapyrusIni", ReadBool, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadBool", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, BSFixedString, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("ReadString", "PapyrusIni", ReadString, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadString", VMClassRegistry::kFunctionFlag_NoWait);

		return true;
	}
}
