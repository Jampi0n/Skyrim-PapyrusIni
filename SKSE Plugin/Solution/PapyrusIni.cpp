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
#define PAPYRUS_ARGS "Data\\" + std::string(file.data), std::string(category.data), std::string(name.data)
#define PAPYRUS_ARGS_EX_DEFAULT "Data\\" + std::string(fileDefault.data), std::string(category.data), std::string(name.data)
#define PAPYRUS_ARGS_EX_USER "Data\\" + std::string(fileUser.data), std::string(category.data), std::string(name.data)

#define USE_CACHE true

#define SECTION_KEY_SEP "::"

namespace PapyrusIni {

	class FileReader {
	private:
		std::unordered_map<std::string, std::unique_ptr<std::string>> settings;
		std::string path;
	public:
		FileReader() = delete;
		FileReader(const FileReader&) = delete;
		FileReader(FileReader&&) = delete;
		FileReader& operator=(FileReader&&) = delete;
		FileReader& operator=(FileReader&) = delete;

		FileReader(std::string path) {
			this->path = path;
			CSimpleIniA ini;
			SI_Error rc = ini.LoadFile(path.c_str());
			CSimpleIniA::TNamesDepend sections;
			ini.GetAllSections(sections);
			for (auto& section : sections) {
				CSimpleIniA::TNamesDepend keys;
				ini.GetAllKeys(section.pItem, keys);
				for (auto& key : keys) {
					settings.emplace(std::string(section.pItem) + SECTION_KEY_SEP + key.pItem, std::make_unique<std::string>(ini.GetValue(section.pItem, key.pItem)));
					auto debug = std::string("Create Cache") + ":" + path + "," + section.pItem + "," + key.pItem + " value=" + ini.GetValue(section.pItem, key.pItem);
					_MESSAGE(debug.c_str());
				}
			}
		}


		std::string& GetValue(std::string section, std::string key, std::string& def) {
			const auto pair = section + SECTION_KEY_SEP + key;
			if (settings.find(pair) == settings.end()) {
				return def;
			}
			auto& value = *settings.at(pair).get();
			auto debug = std::string("Read Cache") + ":" + this->path + "," + section + "," + key + " value=" + value;
			_MESSAGE(debug.c_str());
			return value;
		}
	};

	class FileWriter {
	private:
		std::unordered_map<std::string, std::unique_ptr<std::string>> settings;
		std::string path;
	public:
		FileWriter() = delete;
		FileWriter(const FileWriter&) = delete;
		FileWriter(FileWriter&&) = delete;
		FileWriter& operator=(FileWriter&&) = delete;
		FileWriter& operator=(FileWriter&) = delete;

		FileWriter(std::string path) {
			this->path = path;
		}

		void Write(std::string& section, std::string& key, std::string& value) {
			const auto pair = section + SECTION_KEY_SEP + key;
			if (settings.find(pair) != settings.end()) {
				settings.erase(pair);
			}
			auto debug = std::string("Write Cache") + ":" + this->path + "," + section + "," + key + " value=" + value;
			_MESSAGE(debug.c_str());
			settings.emplace(pair, std::make_unique<std::string>(value));
		}

		void Save() {
			CSimpleIniA ini;
			for (auto& [sectionKeyPair, value] : settings) {
				auto section = sectionKeyPair.substr(0, sectionKeyPair.find(SECTION_KEY_SEP));
				auto key = sectionKeyPair.substr(sectionKeyPair.find(SECTION_KEY_SEP) + std::string(SECTION_KEY_SEP).length());
				ini.SetValue(section.c_str(), key.c_str(), value.get()->c_str());
				auto debug = std::string("Save Cache") + ":" + this->path + "," + section + "," + key + " value=" + *value.get();
				_MESSAGE(debug.c_str());
			}
			ini.SaveFile(path.c_str());
		}
	};

	class IniHandler {
	private:
		std::unordered_map <  std::string, std::unique_ptr<FileReader>> fileReaders;
		std::unordered_map <  std::string, std::unique_ptr<FileWriter>> fileWriters;
	public:
		static auto GetInstance()->IniHandler&
		{
			static IniHandler instance;
			return instance;
		}

		/// <summary>
		/// Returns a FileReader for the specified path. If it does not exist, a new one is created.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		/// <returns>A FileReader containing all values of the .ini file.</returns>
		FileReader& GetFileReader(std::string path) {
			auto debug = std::string("GetFileReader") + ":" + path;
			_MESSAGE(debug.c_str());
			if (fileReaders.find(path) == fileReaders.end()) {
				auto debug = std::string("GetFileReader") + ":" + path + "->" + "new";
				_MESSAGE(debug.c_str());
				fileReaders.emplace(path, std::make_unique<FileReader>(path));
			}
			return *fileReaders.at(path).get();
		}

		/// <summary>
		/// Closes the FileReader for the specified path, freeing memory.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		void CloseFileReader(std::string path) {
			auto debug = std::string("CloseFileReader") + ":" + path;
			_MESSAGE(debug.c_str());
			if (fileReaders.find(path) != fileReaders.end()) {
				auto debug = std::string("CloseFileReader") + ":" + path + "->" + "close";
				_MESSAGE(debug.c_str());
				fileReaders.erase(path);
			}
		}

		/// <summary>
		/// Returns a FileWriter for the specified path. If it does not exist, a new one is created.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		/// <returns>A FileWriter that can write to  the .ini file.</returns>
		FileWriter& GetFileWriter(std::string path) {
			auto debug = std::string("GetFileWriter") + ":" + path;
			_MESSAGE(debug.c_str());
			if (fileWriters.find(path) == fileWriters.end()) {
				auto debug = std::string("GetFileWriter") + ":" + path + "->" + "new";
				_MESSAGE(debug.c_str());
				fileWriters.emplace(path, std::make_unique<FileWriter>(path));
			}
			return *fileWriters.at(path).get();
		}

		/// <summary>
		/// Closes the FileWriter for the specified path, saving the values to the file and freeing memory.
		/// </summary>
		/// <param name="path"></param>
		void CloseFileWriter(std::string path) {
			auto debug = std::string("CloseFileWriter") + ":" + path;
			_MESSAGE(debug.c_str());
			if (fileWriters.find(path) != fileWriters.end()) {
				auto debug = std::string("CloseFileWriter") + ":" + path + "->" + "close";
				_MESSAGE(debug.c_str());
				fileWriters[path].get()->Save();
				fileWriters.erase(path);
			}
		}
	};

	void CloseWriter(std::string& fileName) {
		IniHandler::GetInstance().CloseFileWriter(fileName);
	}

	void WriteString(std::string& fileName, std::string& category, std::string& name, std::string& value, bool cache) {
#if DEBUG
		auto debug = std::string("WriteString") + ":" + fileName + "," + category + "," + name + " value=" + value;
		_MESSAGE(debug.c_str());
#endif
		if (cache) {
			IniHandler::GetInstance().GetFileWriter(fileName).Write(category, name, value);
			IniHandler::GetInstance().CloseFileWriter(fileName);
		}
		else {
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
		IniHandler::GetInstance().CloseFileReader(fileName);
	}

	std::string ReadString(std::string& fileName, std::string& category, std::string& name, std::string& def, bool cache) {
		std::string value;
		if (cache) {
			value = IniHandler::GetInstance().GetFileReader(fileName).GetValue(category, name, def);
		}
		else {
			char inBuf[BUFFER_SIZE];
			GetPrivateProfileStringA(category.c_str(), name.c_str(), def.c_str(), inBuf, BUFFER_SIZE, fileName.c_str());
			value = std::string(inBuf);
		}
#if DEBUG
		auto debug = std::string("ReadString") + ":" + fileName + "," + category + "," + name + " result=" + value;
		_MESSAGE(debug.c_str());
#endif
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

#if DEBUG
		auto debug = std::string("ReadInt") + ":" + fileName + "," + category + "," + name + " result=" + std::to_string(value);
		_MESSAGE(debug.c_str());
#endif
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
#if DEBUG
		auto debug = std::string("HasString") + ":" + fileName + "," + category + "," + name + " zero=" + zero + " one=" + one + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
}

	bool HasInt(std::string& fileName, std::string& category, std::string& name, bool cache) {
		auto zero = ReadInt(fileName, category, name, 0, cache);
		auto one = ReadInt(fileName, category, name, 1, cache);
		auto result = (zero != 0) || (one != 1);
#if DEBUG
		auto debug = std::string("HasInt") + ":" + fileName + "," + category + "," + name + " zero=" + std::to_string(zero) + " one=" + std::to_string(one) + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
	}

	bool HasFloat(std::string& fileName, std::string& category, std::string& name, bool cache) {
		auto zero = ReadFloat(fileName, category, name, 0.0, cache);
		auto one = ReadFloat(fileName, category, name, 1.0, cache);
		auto result = (zero != 0.0) || (one != 1.0);
#if DEBUG
		auto debug = std::string("HasFloat") + ":" + fileName + "," + category + "," + name + " zero=" + std::to_string(zero) + " one=" + std::to_string(one) + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
	}

	bool HasBool(std::string& fileName, std::string& category, std::string& name, bool cache) {
		bool zero = ReadBool(fileName, category, name, false, cache);
		bool one = ReadBool(fileName, category, name, true, cache);
		auto result = (zero != false) || (one != true);
#if DEBUG
		auto debug = std::string("HasBool") + ":" + fileName + "," + category + "," + name + " zero=" + std::to_string(zero) + " one=" + std::to_string(one) + " result=" + std::to_string(result);
		_MESSAGE(debug.c_str());
#endif
		return result;
	}


	void Papyrus_WriteInt(INI_PARAMS, SInt32 value) { WriteInt(PAPYRUS_ARGS, value, USE_CACHE); }
	void Papyrus_WriteFloat(INI_PARAMS, float value) { WriteFloat(PAPYRUS_ARGS, value, USE_CACHE); }
	void Papyrus_WriteBool(INI_PARAMS, bool value) { WriteBool(PAPYRUS_ARGS, value, USE_CACHE); }
	void Papyrus_WriteString(INI_PARAMS, BSFixedString value) { WriteString(PAPYRUS_ARGS, std::string(value.data), USE_CACHE); }

	SInt32 Papyrus_ReadInt(INI_PARAMS, SInt32 def) { return ReadInt(PAPYRUS_ARGS, def, USE_CACHE); }
	float Papyrus_ReadFloat(INI_PARAMS, float def) { return ReadFloat(PAPYRUS_ARGS, def, USE_CACHE); }
	bool Papyrus_ReadBool(INI_PARAMS, bool def) { return ReadBool(PAPYRUS_ARGS, def, USE_CACHE); }
	BSFixedString Papyrus_ReadString(INI_PARAMS, BSFixedString def) { return BSFixedString(ReadString(PAPYRUS_ARGS, std::string(def.data), USE_CACHE).c_str()); }

	bool Papyrus_HasInt(INI_PARAMS) { return HasInt(PAPYRUS_ARGS, USE_CACHE); }
	bool Papyrus_HasFloat(INI_PARAMS) { return HasFloat(PAPYRUS_ARGS, USE_CACHE); }
	bool Papyrus_HasBool(INI_PARAMS) { return HasBool(PAPYRUS_ARGS, USE_CACHE); }
	bool Papyrus_HasString(INI_PARAMS) { return HasString(PAPYRUS_ARGS, USE_CACHE); }

	SInt32 Papyrus_ReadIntEx(INI_PARAMS_EX, SInt32 def) {
		if (!HasInt(PAPYRUS_ARGS_EX_DEFAULT, USE_CACHE)) {
			WriteInt(PAPYRUS_ARGS_EX_DEFAULT, def, USE_CACHE);
		}
		return ReadInt(PAPYRUS_ARGS_EX_USER, ReadInt(PAPYRUS_ARGS_EX_DEFAULT, def, USE_CACHE), USE_CACHE);
	}
	float Papyrus_ReadFloatEx(INI_PARAMS_EX, float def) {
		if (!HasFloat(PAPYRUS_ARGS_EX_DEFAULT, USE_CACHE)) {
			WriteFloat(PAPYRUS_ARGS_EX_DEFAULT, def, USE_CACHE);
		}
		return ReadFloat(PAPYRUS_ARGS_EX_USER, ReadFloat(PAPYRUS_ARGS_EX_DEFAULT, def, USE_CACHE), USE_CACHE);
	}
	bool Papyrus_ReadBoolEx(INI_PARAMS_EX, bool def) {
		if (!HasBool(PAPYRUS_ARGS_EX_DEFAULT, USE_CACHE)) {
			WriteBool(PAPYRUS_ARGS_EX_DEFAULT, def, USE_CACHE);
		}
		return ReadBool(PAPYRUS_ARGS_EX_USER, ReadBool(PAPYRUS_ARGS_EX_DEFAULT, def, USE_CACHE), USE_CACHE);
	}
	BSFixedString Papyrus_ReadStringEx(INI_PARAMS_EX, BSFixedString def) {
		auto stringDef = std::string(def.data);
		if (!HasString(PAPYRUS_ARGS_EX_DEFAULT, USE_CACHE)) {
			WriteString(PAPYRUS_ARGS_EX_DEFAULT, stringDef, USE_CACHE);
		}
		return BSFixedString(ReadString(PAPYRUS_ARGS_EX_USER, ReadString(PAPYRUS_ARGS_EX_DEFAULT, stringDef, USE_CACHE), USE_CACHE).c_str());
	}

	SInt32 Papyrus_GetPluginVersion(StaticFunctionTag* base) {
		return PLUGIN_VERSION;
	}

	bool RegisterFuncs(VMClassRegistry* registry) {
		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, SInt32>("GetPluginVersion", "PapyrusIni", Papyrus_GetPluginVersion, registry));
		registry->SetFunctionFlags("PapyrusIni", "GetPluginVersion", VMClassRegistry::kFunctionFlag_NoWait);

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
