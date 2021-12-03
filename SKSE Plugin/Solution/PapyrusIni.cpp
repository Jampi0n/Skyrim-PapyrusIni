#include "PapyrusIni.h"

#include <vector>
#include <cmath>
#include <stdexcept>

#include <ShlObj.h>
#include <WinBase.h>

#define DEBUG 0
#define BUFFER_SIZE 128
#define PAPYRUS_FUNCTION StaticFunctionTag* base
#define INI_PARAMS PAPYRUS_FUNCTION, BSFixedString file, BSFixedString category, BSFixedString name
#define INI_PARAMS_EX PAPYRUS_FUNCTION, BSFixedString fileDefault, BSFixedString fileUser, BSFixedString category, BSFixedString name
#define PAPYRUS_ARGS "Data\\" + std::string(file.data), std::string(category.data), std::string(name.data)
#define PAPYRUS_ARGS_EX_DEFAULT "Data\\" + std::string(fileDefault.data), std::string(category.data), std::string(name.data)
#define PAPYRUS_ARGS_EX_USER "Data\\" + std::string(fileUser.data), std::string(category.data), std::string(name.data)
namespace PapyrusIni {


	void WriteString(std::string& fileName, std::string& category, std::string& name, std::string& value) {
#if DEBUG
		auto debug = std::string("WriteString") + ":" + fileName + "," + category + "," + name + " value=" + value;
		_MESSAGE(debug.c_str());
#endif
		WritePrivateProfileStringA(category.c_str(), name.c_str(), value.c_str(), fileName.c_str());
	}

	void WriteInt(std::string& fileName, std::string& category, std::string& name, SInt32 value) {
		WriteString(fileName, category, name, std::to_string(value));
	}

	void WriteBool(std::string& fileName, std::string& category, std::string& name, bool value) {
		WriteString(fileName, category, name, std::string(value ? "1" : "0"));
	}

	void WriteFloat(std::string& fileName, std::string& category, std::string& name, float value) {
		WriteString(fileName, category, name, std::to_string(value));
	}

	std::string ReadString(std::string& fileName, std::string& category, std::string& name, std::string& def) {
		char inBuf[BUFFER_SIZE];
		GetPrivateProfileStringA(category.c_str(), name.c_str(), def.c_str(), inBuf, BUFFER_SIZE, fileName.c_str());
		auto result = std::string(inBuf);
#if DEBUG
		auto debug = std::string("ReadString") + ":" + fileName + "," + category + "," + name + " result=" + result;
		_MESSAGE(debug.c_str());
#endif
		return result;
	}

	SInt32 ReadInt(std::string& fileName, std::string& category, std::string& name, SInt32 def) {
		auto result = GetPrivateProfileIntA(category.c_str(), name.c_str(), def, fileName.c_str());
#if DEBUG
		auto debug = std::string("ReadInt") + ":" + fileName + "," + category + "," + name + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
	}

	float ReadFloat(std::string& fileName, std::string& category, std::string& name, float def) {
		float value = def;
		try {
			value = std::stof(ReadString(fileName, category, name, std::to_string(def)).c_str());
		}
		catch (std::invalid_argument) {
			value = def;
		}
		return value;
	}

	bool ReadBool(std::string& fileName, std::string& category, std::string& name, bool def) {
		return ReadInt(fileName, category, name, def ? 1 : 0) == 1;
	}

	bool HasString(std::string& fileName, std::string& category, std::string& name) {
		auto zero = ReadString(fileName, category, name, std::string("zero"));
		auto one = ReadString(fileName, category, name, std::string("one"));
		auto result = (zero.compare(std::string("zero")) != 0) || (one.compare(std::string("one")) != 0);
#if DEBUG
		auto debug = std::string("HasString") + ":" + fileName + "," + category + "," + name + " zero=" + zero + " one=" + one + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
	}

	bool HasInt(std::string& fileName, std::string& category, std::string& name) {
		auto zero = ReadInt(fileName, category, name, 0);
		auto one = ReadInt(fileName, category, name, 1);
		auto result = (zero != 0) || (one != 1);
#if DEBUG
		auto debug = std::string("HasInt") + ":" + fileName + "," + category + "," + name + " zero=" + std::to_string(zero) + " one=" + std::to_string(one) + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
	}

	bool HasFloat(std::string& fileName, std::string& category, std::string& name) {
		auto zero = ReadFloat(fileName, category, name, 0.0);
		auto one = ReadFloat(fileName, category, name, 1.0);
		auto result = (zero != 0.0) || (one != 1.0);
#if DEBUG
		auto debug = std::string("HasFloat") + ":" + fileName + "," + category + "," + name + " zero=" + std::to_string(zero) + " one=" + std::to_string(one) + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
	}

	bool HasBool(std::string& fileName, std::string& category, std::string& name) {
		bool zero = ReadBool(fileName, category, name, false);
		bool one = ReadBool(fileName, category, name, true);
		auto result = (zero != false) || (one != true);
#if DEBUG
		auto debug = std::string("HasBool") + ":" + fileName + "," + category + "," + name + " zero=" + std::to_string(zero) + " one=" + std::to_string(one) + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
	}


	void Papyrus_WriteInt(INI_PARAMS, SInt32 value) { WriteInt(PAPYRUS_ARGS, value); }
	void Papyrus_WriteFloat(INI_PARAMS, float value) { WriteFloat(PAPYRUS_ARGS, value); }
	void Papyrus_WriteBool(INI_PARAMS, bool value) { WriteBool(PAPYRUS_ARGS, value); }
	void Papyrus_WriteString(INI_PARAMS, BSFixedString value) { WriteString(PAPYRUS_ARGS, std::string(value.data)); }

	SInt32 Papyrus_ReadInt(INI_PARAMS, SInt32 def) { return ReadInt(PAPYRUS_ARGS, def); }
	float Papyrus_ReadFloat(INI_PARAMS, float def) { return ReadFloat(PAPYRUS_ARGS, def); }
	bool Papyrus_ReadBool(INI_PARAMS, bool def) { return ReadBool(PAPYRUS_ARGS, def); }
	BSFixedString Papyrus_ReadString(INI_PARAMS, BSFixedString def) { return BSFixedString(ReadString(PAPYRUS_ARGS, std::string(def.data)).c_str()); }

	bool Papyrus_HasInt(INI_PARAMS) { return HasInt(PAPYRUS_ARGS); }
	bool Papyrus_HasFloat(INI_PARAMS) { return HasFloat(PAPYRUS_ARGS); }
	bool Papyrus_HasBool(INI_PARAMS) { return HasBool(PAPYRUS_ARGS); }
	bool Papyrus_HasString(INI_PARAMS) { return HasString(PAPYRUS_ARGS); }

	SInt32 Papyrus_ReadIntEx(INI_PARAMS_EX, SInt32 def) {
		if (!HasInt(PAPYRUS_ARGS_EX_DEFAULT)) {
			WriteInt(PAPYRUS_ARGS_EX_DEFAULT, def);
		}
		return ReadInt(PAPYRUS_ARGS_EX_USER, ReadInt(PAPYRUS_ARGS_EX_DEFAULT, def));
	}
	float Papyrus_ReadFloatEx(INI_PARAMS_EX, float def) {
		if (!HasFloat(PAPYRUS_ARGS_EX_DEFAULT)) {
			WriteFloat(PAPYRUS_ARGS_EX_DEFAULT, def);
		}
		return ReadFloat(PAPYRUS_ARGS_EX_USER, ReadFloat(PAPYRUS_ARGS_EX_DEFAULT, def));
	}
	bool Papyrus_ReadBoolEx(INI_PARAMS_EX, bool def) {
		if (!HasBool(PAPYRUS_ARGS_EX_DEFAULT)) {
			WriteBool(PAPYRUS_ARGS_EX_DEFAULT, def);
		}
		return ReadBool(PAPYRUS_ARGS_EX_USER, ReadBool(PAPYRUS_ARGS_EX_DEFAULT, def));
	}
	BSFixedString Papyrus_ReadStringEx(INI_PARAMS_EX, BSFixedString def) {
		auto stringDef = std::string(def);
		if (!HasString(PAPYRUS_ARGS_EX_DEFAULT)) {
			WriteString(PAPYRUS_ARGS_EX_DEFAULT, stringDef);
		}
		return BSFixedString(ReadString(PAPYRUS_ARGS_EX_USER, ReadString(PAPYRUS_ARGS_EX_DEFAULT, stringDef)).c_str());
	}

	bool RegisterFuncs(VMClassRegistry* registry) {
		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, SInt32>("WriteInt", "PapyrusIni", Papyrus_WriteInt, registry));
		registry->SetFunctionFlags("PapyrusIni", "WriteInt", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, float>("WriteFloat", "PapyrusIni", Papyrus_WriteFloat, registry));
		registry->SetFunctionFlags("PapyrusIni", "WriteFloat", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, bool>("WriteBool", "PapyrusIni", Papyrus_WriteBool, registry));
		registry->SetFunctionFlags("PapyrusIni", "WriteBool", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("WriteString", "PapyrusIni", Papyrus_WriteString, registry));
		registry->SetFunctionFlags("PapyrusIni", "WriteString", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, SInt32, BSFixedString, BSFixedString, BSFixedString, SInt32>("ReadInt", "PapyrusIni", Papyrus_ReadInt, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadInt", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, float, BSFixedString, BSFixedString, BSFixedString, float>("ReadFloat", "PapyrusIni", Papyrus_ReadFloat, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadFloat", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString, bool>("ReadBool", "PapyrusIni", Papyrus_ReadBool, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadBool", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, BSFixedString, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("ReadString", "PapyrusIni", Papyrus_ReadString, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadString", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("HasInt", "PapyrusIni", Papyrus_HasInt, registry));
		registry->SetFunctionFlags("PapyrusIni", "HasInt", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("HasFloat", "PapyrusIni", Papyrus_HasFloat, registry));
		registry->SetFunctionFlags("PapyrusIni", "HasFloat", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("HasBool", "PapyrusIni", Papyrus_HasBool, registry));
		registry->SetFunctionFlags("PapyrusIni", "HasBool", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("HasString", "PapyrusIni", Papyrus_HasString, registry));
		registry->SetFunctionFlags("PapyrusIni", "HasString", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, SInt32, BSFixedString, BSFixedString, BSFixedString, BSFixedString, SInt32>("ReadIntEx", "PapyrusIni", Papyrus_ReadIntEx, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadIntEx", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, float, BSFixedString, BSFixedString, BSFixedString, BSFixedString, float>("ReadFloatEx", "PapyrusIni", Papyrus_ReadFloatEx, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadFloatEx", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString, BSFixedString, bool>("ReadBoolEx", "PapyrusIni", Papyrus_ReadBoolEx, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadBoolEx", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, BSFixedString, BSFixedString, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("ReadStringEx", "PapyrusIni", Papyrus_ReadStringEx, registry));
		registry->SetFunctionFlags("PapyrusIni", "ReadStringEx", VMClassRegistry::kFunctionFlag_NoWait);


		return true;
	}
}
