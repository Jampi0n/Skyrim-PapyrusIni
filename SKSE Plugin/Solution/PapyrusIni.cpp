#include "PapyrusIni.h"

#include <vector>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <tuple>
#include <memory>

#include <ShlObj.h>
#include <WinBase.h>
#include "SimpleIni.h"

#define DEBUG 0
#define BUFFER_SIZE 128
#define PAPYRUS_FUNCTION StaticFunctionTag* base
#define INI_PARAMS PAPYRUS_FUNCTION, BSFixedString file, BSFixedString category, BSFixedString name
#define INI_PARAMS_EX PAPYRUS_FUNCTION, BSFixedString fileDefault, BSFixedString fileUser, BSFixedString category, BSFixedString name
#define PAPYRUS_ARGS_FILE_NAME "Data\\" + std::string(file.data)
#define PAPYRUS_ARGS "Data\\" + std::string(file.data), std::string(category.data), std::string(name.data)
#define PAPYRUS_ARGS_EX_DEFAULT "Data\\" + std::string(fileDefault.data), std::string(category.data), std::string(name.data)
#define PAPYRUS_ARGS_EX_USER "Data\\" + std::string(fileUser.data), std::string(category.data), std::string(name.data)

#define USE_CACHE true

#define SECTION_KEY_SEP "::"

namespace PapyrusIni {

	void Log(std::string& s) {
		//_MESSAGE(s.c_str());
	}

	std::string IniAccess(std::string& path, std::string& section, std::string& key) {
		return "{" + path + "}[" + section + "]<" + key + ">";
	}

	class IniCache {
	private:
		std::string path;
		CSimpleIniA ini;
	public:
		IniCache() = delete;
		IniCache(const IniCache&) = delete;
		IniCache(IniCache&&) = delete;
		IniCache& operator=(IniCache&&) = delete;
		IniCache& operator=(IniCache&) = delete;

		IniCache(std::string path) {
			this->path = path;
			Load();
		}

		void Load() {
			Log("Load Cache: {" + path + "}");
			SI_Error rc = ini.LoadFile(path.c_str());
		}

		std::string& Read(std::string section, std::string key, std::string& def) {
			auto value = std::string(ini.GetValue(section.c_str(), key.c_str(), def.c_str()));
			Log("Read Cache: " + IniAccess(path, section, key) + " value=" + value);
			return value;
		}

		void Write(std::string& section, std::string& key, std::string& value) {
			Log("Write Cache: " + IniAccess(path, section, key) + " value=" + value);
			ini.SetValue(section.c_str(), key.c_str(), value.c_str());
		}

		void Save() {
			Log("Save Cache: {" + path + "}");
			ini.SaveFile(path.c_str());
		}
	};


	class IniHandler {
	private:
		std::unordered_map <  std::string, std::unique_ptr<IniCache>> fileReaders;
	public:
		static auto GetInstance()->IniHandler&
		{
			static IniHandler instance;
			return instance;
		}

		/// <summary>
		/// Returns true, if a IniCache exists for the specified path.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		/// <returns></returns>
		bool HasIniCache(std::string path) {
			return fileReaders.find(path) != fileReaders.end();
		}

		/// <summary>
		/// Returns a IniCache for the specified path. If it does not exist, a new one is created.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		/// <returns>A IniCache containing all values of the .ini file.</returns>
		IniCache& GetIniCache(std::string path) {
			Log("GetIniCache: {" + path + "}");
			if (fileReaders.find(path) == fileReaders.end()) {
				Log("GetIniCache: {" + path + "}->new");
				fileReaders.emplace(path, std::make_unique<IniCache>(path));
			}
			return *fileReaders.at(path).get();
		}

		/// <summary>
		/// Closes the IniCache for the specified path, writing the changes and freeing memory.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		void CloseIniCache(std::string path) {
			Log("CloseIniCache: {" + path + "}");
			if (fileReaders.find(path) != fileReaders.end()) {
				Log("CloseIniCache: {" + path + "}->new");
				fileReaders[path].get()->Save();
				fileReaders.erase(path);
			}
		}


	};

	void CreateCache(std::string& fileName) {
		IniHandler::GetInstance().GetIniCache(fileName);
	}

	void WriteCache(std::string& fileName) {
		if (IniHandler::GetInstance().HasIniCache(fileName)) {
			IniHandler::GetInstance().GetIniCache(fileName).Save();
		}
	}

	void CloseCache(std::string& fileName) {
		IniHandler::GetInstance().CloseIniCache(fileName);
	}

	void WriteString(std::string& fileName, std::string& category, std::string& name, std::string& value, bool cache) {
		// write to cache, creating one if it does not exist
		if (cache) {
			IniHandler::GetInstance().GetIniCache(fileName).Write(category, name, value);
		}
		else {
			// write to cache if it exists, but do not create a new one
			if (IniHandler::GetInstance().HasIniCache(fileName)) {
				IniHandler::GetInstance().GetIniCache(fileName).Write(category, name, value);
			}
			// write without cache
			WritePrivateProfileStringA(category.c_str(), name.c_str(), value.c_str(), fileName.c_str());
		}
	}

	void WriteInt(std::string& fileName, std::string& category, std::string& name, SInt32 value, bool cache) {
		WriteString(fileName, category, name, std::to_string(value), cache);
	}

	void WriteBool(std::string& fileName, std::string& category, std::string& name, bool value, bool cache) {
		WriteString(fileName, category, name, std::string(value ? "1" : "0"), cache);
	}

	void WriteFloat(std::string& fileName, std::string& category, std::string& name, float value, bool cache) {
		WriteString(fileName, category, name, std::to_string(value), cache);
	}

	void CloseReader(std::string& fileName) {
		IniHandler::GetInstance().CloseIniCache(fileName);
	}

	std::string ReadString(std::string& fileName, std::string& category, std::string& name, std::string& def, bool cache) {
		std::string value;
		// read from cache, creating one if it does not exist
		if (cache) {
			value = IniHandler::GetInstance().GetIniCache(fileName).Read(category, name, def);
		}
		else {
			// read from cache if it exists, but do not create a new one
			if (IniHandler::GetInstance().HasIniCache(fileName)) {
				return ReadString(fileName, category, name, def, true);
			}
			// read without cache
			char inBuf[BUFFER_SIZE];
			GetPrivateProfileStringA(category.c_str(), name.c_str(), def.c_str(), inBuf, BUFFER_SIZE, fileName.c_str());
			value = std::string(inBuf);
		}
		return value;
	}

	SInt32 ReadInt(std::string& fileName, std::string& category, std::string& name, SInt32 def, bool cache) {
		SInt32 value;
		try {
			value = std::stoi(ReadString(fileName, category, name, std::to_string(def), cache).c_str());
		}
		catch (std::invalid_argument) {
			value = def;
		}
		return value;
	}

	float ReadFloat(std::string& fileName, std::string& category, std::string& name, float def, bool cache) {
		float value = def;
		try {
			value = std::stof(ReadString(fileName, category, name, std::to_string(def), cache).c_str());
		}
		catch (std::invalid_argument) {
			value = def;
		}
		return value;
	}

	bool ReadBool(std::string& fileName, std::string& category, std::string& name, bool def, bool cache) {
		return ReadInt(fileName, category, name, def ? 1 : 0, cache) == 1;
	}

	bool HasString(std::string& fileName, std::string& category, std::string& name, bool cache) {
		auto zero = ReadString(fileName, category, name, std::string("zero"), cache);
		auto one = ReadString(fileName, category, name, std::string("one"), cache);
		auto result = (zero.compare(std::string("zero")) != 0) || (one.compare(std::string("one")) != 0);
		return result;
	}

	bool HasInt(std::string& fileName, std::string& category, std::string& name, bool cache) {
		auto zero = ReadInt(fileName, category, name, 0, cache);
		auto one = ReadInt(fileName, category, name, 1, cache);
		auto result = (zero != 0) || (one != 1);
		return result;
	}

	bool HasFloat(std::string& fileName, std::string& category, std::string& name, bool cache) {
		auto zero = ReadFloat(fileName, category, name, 0.0, cache);
		auto one = ReadFloat(fileName, category, name, 1.0, cache);
		auto result = (zero != 0.0) || (one != 1.0);
		return result;
	}

	bool HasBool(std::string& fileName, std::string& category, std::string& name, bool cache) {
		bool zero = ReadBool(fileName, category, name, false, cache);
		bool one = ReadBool(fileName, category, name, true, cache);
		auto result = (zero != false) || (one != true);
		return result;
	}

	void Papyrus_WriteInt(INI_PARAMS, SInt32 value) { WriteInt(PAPYRUS_ARGS, value, false); }
	void Papyrus_WriteFloat(INI_PARAMS, float value) { WriteFloat(PAPYRUS_ARGS, value, false); }
	void Papyrus_WriteBool(INI_PARAMS, bool value) { WriteBool(PAPYRUS_ARGS, value, false); }
	void Papyrus_WriteString(INI_PARAMS, BSFixedString value) { WriteString(PAPYRUS_ARGS, std::string(value.data), false); }

	SInt32 Papyrus_ReadInt(INI_PARAMS, SInt32 def) { return ReadInt(PAPYRUS_ARGS, def, false); }
	float Papyrus_ReadFloat(INI_PARAMS, float def) { return ReadFloat(PAPYRUS_ARGS, def, false); }
	bool Papyrus_ReadBool(INI_PARAMS, bool def) { return ReadBool(PAPYRUS_ARGS, def, false); }
	BSFixedString Papyrus_ReadString(INI_PARAMS, BSFixedString def) { return BSFixedString(ReadString(PAPYRUS_ARGS, std::string(def.data), false).c_str()); }

	bool Papyrus_HasInt(INI_PARAMS) { return HasInt(PAPYRUS_ARGS, false); }
	bool Papyrus_HasFloat(INI_PARAMS) { return HasFloat(PAPYRUS_ARGS, false); }
	bool Papyrus_HasBool(INI_PARAMS) { return HasBool(PAPYRUS_ARGS, false); }
	bool Papyrus_HasString(INI_PARAMS) { return HasString(PAPYRUS_ARGS, false); }

	SInt32 Papyrus_ReadIntEx(INI_PARAMS_EX, SInt32 def) {
		if (!HasInt(PAPYRUS_ARGS_EX_DEFAULT, false)) {
			WriteInt(PAPYRUS_ARGS_EX_DEFAULT, def, false);
		}
		return ReadInt(PAPYRUS_ARGS_EX_USER, ReadInt(PAPYRUS_ARGS_EX_DEFAULT, def, false), false);
	}
	float Papyrus_ReadFloatEx(INI_PARAMS_EX, float def) {
		if (!HasFloat(PAPYRUS_ARGS_EX_DEFAULT, false)) {
			WriteFloat(PAPYRUS_ARGS_EX_DEFAULT, def, false);
		}
		return ReadFloat(PAPYRUS_ARGS_EX_USER, ReadFloat(PAPYRUS_ARGS_EX_DEFAULT, def, false), false);
	}
	bool Papyrus_ReadBoolEx(INI_PARAMS_EX, bool def) {
		if (!HasBool(PAPYRUS_ARGS_EX_DEFAULT, false)) {
			WriteBool(PAPYRUS_ARGS_EX_DEFAULT, def, false);
		}
		return ReadBool(PAPYRUS_ARGS_EX_USER, ReadBool(PAPYRUS_ARGS_EX_DEFAULT, def, false), false);
	}
	BSFixedString Papyrus_ReadStringEx(INI_PARAMS_EX, BSFixedString def) {
		auto stringDef = std::string(def.data);
		if (!HasString(PAPYRUS_ARGS_EX_DEFAULT, false)) {
			WriteString(PAPYRUS_ARGS_EX_DEFAULT, stringDef, false);
		}
		return BSFixedString(ReadString(PAPYRUS_ARGS_EX_USER, ReadString(PAPYRUS_ARGS_EX_DEFAULT, stringDef, false), false).c_str());
	}

	void Buffered_WriteInt(INI_PARAMS, SInt32 value) { WriteInt(PAPYRUS_ARGS, value, true); }
	void Buffered_WriteFloat(INI_PARAMS, float value) { WriteFloat(PAPYRUS_ARGS, value, true); }
	void Buffered_WriteBool(INI_PARAMS, bool value) { WriteBool(PAPYRUS_ARGS, value, true); }
	void Buffered_WriteString(INI_PARAMS, BSFixedString value) { WriteString(PAPYRUS_ARGS, std::string(value.data), true); }

	SInt32 Buffered_ReadInt(INI_PARAMS, SInt32 def) { return ReadInt(PAPYRUS_ARGS, def, true); }
	float Buffered_ReadFloat(INI_PARAMS, float def) { return ReadFloat(PAPYRUS_ARGS, def, true); }
	bool Buffered_ReadBool(INI_PARAMS, bool def) { return ReadBool(PAPYRUS_ARGS, def, true); }
	BSFixedString Buffered_ReadString(INI_PARAMS, BSFixedString def) { return BSFixedString(ReadString(PAPYRUS_ARGS, std::string(def.data), true).c_str()); }

	bool Buffered_HasInt(INI_PARAMS) { return HasInt(PAPYRUS_ARGS, true); }
	bool Buffered_HasFloat(INI_PARAMS) { return HasFloat(PAPYRUS_ARGS, true); }
	bool Buffered_HasBool(INI_PARAMS) { return HasBool(PAPYRUS_ARGS, true); }
	bool Buffered_HasString(INI_PARAMS) { return HasString(PAPYRUS_ARGS, true); }

	SInt32 Buffered_ReadIntEx(INI_PARAMS_EX, SInt32 def) {
		if (!HasInt(PAPYRUS_ARGS_EX_DEFAULT, true)) {
			WriteInt(PAPYRUS_ARGS_EX_DEFAULT, def, true);
		}
		return ReadInt(PAPYRUS_ARGS_EX_USER, ReadInt(PAPYRUS_ARGS_EX_DEFAULT, def, true), true);
	}
	float Buffered_ReadFloatEx(INI_PARAMS_EX, float def) {
		if (!HasFloat(PAPYRUS_ARGS_EX_DEFAULT, true)) {
			WriteFloat(PAPYRUS_ARGS_EX_DEFAULT, def, true);
		}
		return ReadFloat(PAPYRUS_ARGS_EX_USER, ReadFloat(PAPYRUS_ARGS_EX_DEFAULT, def, true), true);
	}
	bool Buffered_ReadBoolEx(INI_PARAMS_EX, bool def) {
		if (!HasBool(PAPYRUS_ARGS_EX_DEFAULT, true)) {
			WriteBool(PAPYRUS_ARGS_EX_DEFAULT, def, true);
		}
		return ReadBool(PAPYRUS_ARGS_EX_USER, ReadBool(PAPYRUS_ARGS_EX_DEFAULT, def, true), true);
	}
	BSFixedString Buffered_ReadStringEx(INI_PARAMS_EX, BSFixedString def) {
		auto stringDef = std::string(def.data);
		if (!HasString(PAPYRUS_ARGS_EX_DEFAULT, true)) {
			WriteString(PAPYRUS_ARGS_EX_DEFAULT, stringDef, true);
		}
		return BSFixedString(ReadString(PAPYRUS_ARGS_EX_USER, ReadString(PAPYRUS_ARGS_EX_DEFAULT, stringDef, true), true).c_str());
	}
	
	void Buffered_CreateBuffer(StaticFunctionTag* base, BSFixedString file) {
		CreateCache(PAPYRUS_ARGS_FILE_NAME);
	}
	void Buffered_WriteBuffer(StaticFunctionTag* base, BSFixedString file) {
		WriteCache(PAPYRUS_ARGS_FILE_NAME);
	}
	void Buffered_CloseBuffer(StaticFunctionTag* base, BSFixedString file) {
		CloseCache(PAPYRUS_ARGS_FILE_NAME);
	}

	SInt32 Papyrus_GetPluginVersion(StaticFunctionTag* base) {
		return PLUGIN_VERSION;
	}

	bool RegisterFuncs(VMClassRegistry* registry) {

		// Special Functions

		// Non-Buffered

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, SInt32>("GetPluginVersion", "PapyrusIni", Papyrus_GetPluginVersion, registry));
		registry->SetFunctionFlags("PapyrusIni", "GetPluginVersion", VMClassRegistry::kFunctionFlag_NoWait);

		// Buffered

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, BSFixedString>("CreateBuffer", "BufferedIni", Buffered_CreateBuffer, registry));
		registry->SetFunctionFlags("BufferedIni", "CreateBuffer", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, BSFixedString>("WriteBuffer", "BufferedIni", Buffered_WriteBuffer, registry));
		registry->SetFunctionFlags("BufferedIni", "WriteBuffer", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, BSFixedString>("CloseBuffer", "BufferedIni", Buffered_CloseBuffer, registry));
		registry->SetFunctionFlags("BufferedIni", "CloseBuffer", VMClassRegistry::kFunctionFlag_NoWait);

		// Standard API

		// Non-Buffered

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

		// Buffered

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, SInt32>("WriteInt", "BufferedIni", Buffered_WriteInt, registry));
		registry->SetFunctionFlags("BufferedIni", "WriteInt", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, float>("WriteFloat", "BufferedIni", Buffered_WriteFloat, registry));
		registry->SetFunctionFlags("BufferedIni", "WriteFloat", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, bool>("WriteBool", "BufferedIni", Buffered_WriteBool, registry));
		registry->SetFunctionFlags("BufferedIni", "WriteBool", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("WriteString", "BufferedIni", Buffered_WriteString, registry));
		registry->SetFunctionFlags("BufferedIni", "WriteString", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, SInt32, BSFixedString, BSFixedString, BSFixedString, SInt32>("ReadInt", "BufferedIni", Buffered_ReadInt, registry));
		registry->SetFunctionFlags("BufferedIni", "ReadInt", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, float, BSFixedString, BSFixedString, BSFixedString, float>("ReadFloat", "BufferedIni", Buffered_ReadFloat, registry));
		registry->SetFunctionFlags("BufferedIni", "ReadFloat", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString, bool>("ReadBool", "BufferedIni", Buffered_ReadBool, registry));
		registry->SetFunctionFlags("BufferedIni", "ReadBool", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction4 <StaticFunctionTag, BSFixedString, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("ReadString", "BufferedIni", Buffered_ReadString, registry));
		registry->SetFunctionFlags("BufferedIni", "ReadString", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("HasInt", "BufferedIni", Buffered_HasInt, registry));
		registry->SetFunctionFlags("BufferedIni", "HasInt", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("HasFloat", "BufferedIni", Buffered_HasFloat, registry));
		registry->SetFunctionFlags("BufferedIni", "HasFloat", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("HasBool", "BufferedIni", Buffered_HasBool, registry));
		registry->SetFunctionFlags("BufferedIni", "HasBool", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("HasString", "BufferedIni", Buffered_HasString, registry));
		registry->SetFunctionFlags("BufferedIni", "HasString", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, SInt32, BSFixedString, BSFixedString, BSFixedString, BSFixedString, SInt32>("ReadIntEx", "BufferedIni", Buffered_ReadIntEx, registry));
		registry->SetFunctionFlags("BufferedIni", "ReadIntEx", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, float, BSFixedString, BSFixedString, BSFixedString, BSFixedString, float>("ReadFloatEx", "BufferedIni", Buffered_ReadFloatEx, registry));
		registry->SetFunctionFlags("BufferedIni", "ReadFloatEx", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString, BSFixedString, bool>("ReadBoolEx", "BufferedIni", Buffered_ReadBoolEx, registry));
		registry->SetFunctionFlags("BufferedIni", "ReadBoolEx", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, BSFixedString, BSFixedString, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("ReadStringEx", "BufferedIni", Buffered_ReadStringEx, registry));
		registry->SetFunctionFlags("BufferedIni", "ReadStringEx", VMClassRegistry::kFunctionFlag_NoWait);

		return true;
	}
}
