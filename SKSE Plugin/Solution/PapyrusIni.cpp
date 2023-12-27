#include "PapyrusIni.h"

#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <filesystem>

#include <ShlObj.h>
#include <WinBase.h>
#include "SimpleIni.h"


#define DEBUG 0
#define PAPYRUS_FUNCTION StaticFunctionTag* base

constexpr auto BUFFER_SIZE = 32;
constexpr auto SECTION_KEY_SEP = "::";

namespace PapyrusIni {

	class Logger {
	private:
		static void WriteLine(std::string str) {
			auto t = std::time(nullptr);
			std::tm tm;
			localtime_s(&tm, &t);

			std::ostringstream oss;
			oss << std::put_time(&tm, "%d/%m/%Y - %I:%M:%S%p");
			auto time = oss.str();

			_MESSAGE(("[" + time + "] " + str).c_str());
		}
	public:
		static void Error(std::string str) {
			WriteLine("Error: " + str);
		}
		static void Msg(std::string str) {
			WriteLine(str);
		}
		static void DebugMsg(std::string str) {
#if DEBUG > 0
			WriteLine(str);
#endif
		}
	};

	std::string IniAccess(std::string& path, std::string& section, std::string& key) {
		return "{" + path + "}[" + section + "]<" + key + ">";
	}

	class FileHelper {
	public:
		static void CreateParentDir(std::string& iniFile) {
			std::filesystem::path filePath(iniFile);
			std::filesystem::path parentPath = filePath.parent_path();
			std::filesystem::directory_entry directory(parentPath);
			if (!directory.exists()) {
				std::filesystem::create_directories(parentPath);
			}
		}

		static void FileCannotBeLoaded(std::string& iniFile) {
			std::filesystem::path filePath(iniFile);
			std::filesystem::directory_entry iniEntry(filePath);
			if (!iniEntry.exists()) {
				Logger::Msg("File does not exist: " + iniFile);
				Logger::Msg("\tDefault values will be used.");
			}
			else {
				Logger::Error("Failed to parse file: " + iniFile);
				Logger::Error("\tCheck that the file is not protected or corrupted.");
			}
		}

		static void FileCannotBeSaved(std::string& iniFile) {
			Logger::Error("Failed to save file: " + iniFile);
			Logger::Error("\tCheck that the file is not protected or read-only.");
		}
	};

	class IniCache {
	private:
		std::string path;
		CSimpleIniA ini;
		bool modified = false;
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
			Logger::Msg("Load Cache: {" + path + "}");
			SI_Error rc = ini.LoadFile(path.c_str());
			if (rc < 0) {
				FileHelper::FileCannotBeLoaded(path);
				return;
			}
		}

		std::string Read(std::string section, std::string key, std::string& def) {
			auto value = std::string(ini.GetValue(section.c_str(), key.c_str(), def.c_str()));
			Logger::DebugMsg("Read Cache: " + IniAccess(path, section, key) + " value=" + value);
			return value;
		}

		void Write(std::string& section, std::string& key, std::string& value) {
			Logger::DebugMsg("Write Cache: " + IniAccess(path, section, key) + " value=" + value);
			modified = true;
			SI_Error rc = ini.SetValue(section.c_str(), key.c_str(), value.c_str());
			if (rc < 0) {
				Logger::Error("Failed to write buffer: " + path);
				Logger::Error("\tThis is an error with PapyrusIni.Please report the bug.");
				return;
			}
		}

		void Save() {
			if (modified) {
				Logger::Msg("Save Cache: {" + path + "} -> save");
				FileHelper::CreateParentDir(path);
				SI_Error rc = ini.SaveFile(path.c_str());
				if (rc < 0) {
					FileHelper::FileCannotBeSaved(path);
				}
			}
			else {
				Logger::Msg("Save Cache: {" + path + "} -> no changes");
			}
		}
	};


	class IniHandler {
	private:
		std::unordered_map <  std::string, std::unique_ptr<IniCache>> fileReaders;
	public:
		static auto GetInstance() -> IniHandler&
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
			Logger::DebugMsg("HasIniCache: {" + path + "} -> return " + std::to_string(fileReaders.find(path) != fileReaders.end()));
			return fileReaders.find(path) != fileReaders.end();
		}

		/// <summary>
		/// Returns a IniCache for the specified path. If it does not exist, a new one is created.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		/// <returns>A IniCache containing all values of the .ini file.</returns>
		IniCache& GetIniCache(std::string path) {
			if (fileReaders.find(path) == fileReaders.end()) {
				Logger::DebugMsg("GetIniCache: {" + path + "} -> get new");
				fileReaders.emplace(path, std::make_unique<IniCache>(path));
			}
			else {
				Logger::DebugMsg("GetIniCache: {" + path + "} -> get existing");
			}
			return *fileReaders.at(path).get();
		}

		/// <summary>
		/// Closes the IniCache for the specified path, writing the changes and freeing memory.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		void CloseIniCache(std::string path) {
			if (fileReaders.find(path) != fileReaders.end()) {
				Logger::DebugMsg("CloseIniCache: {" + path + "} -> close existing");
				fileReaders[path].get()->Save();
				fileReaders.erase(path);
			}
			else {
				Logger::DebugMsg("CloseIniCache: {" + path + "} -> does not exist");
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

	std::pair<std::string, std::string> ExtractSettingAndKey(std::string& settingName) {
		auto colonIndex = settingName.find(':');
		if (colonIndex == -1) {
			Logger::Error("Invalid setting name: \"" + settingName + "\"");
			return std::make_pair<std::string, std::string>("", "");
		}
		auto key = settingName.substr(0, colonIndex);
		auto section = settingName.substr(colonIndex + 1);
		return std::pair<std::string, std::string>(section, key);
	}

	void WriteString(std::string& fileName, std::string& settingName, std::string& value, bool cache) {
		auto pair = ExtractSettingAndKey(settingName);
		auto& section = pair.first;
		auto& key = pair.second;
		if (section.compare("") == 0 || key.compare("") == 0) {
			Logger::Msg("No value was written for setting name: \"" + settingName + "\"");
			return;
		}

		Logger::DebugMsg("Write File: " + IniAccess(fileName, section, key) + " value=" + value);
		// write to cache, creating one if it does not exist
		if (cache) {
			IniHandler::GetInstance().GetIniCache(fileName).Write(section, key, value);
		}
		else {
			// write to cache if it exists, but do not create a new one
			if (IniHandler::GetInstance().HasIniCache(fileName)) {
				IniHandler::GetInstance().GetIniCache(fileName).Write(section, key, value);
			}
			// write without cache
			FileHelper::CreateParentDir(fileName);
			if (!WritePrivateProfileStringA(section.c_str(), key.c_str(), value.c_str(), fileName.c_str())) {
				Logger::Msg("Failed to write file: " + fileName);
				Logger::Msg("	 Check that the path is correct and the file is not protected or read-only.");
			}
		}
	}

	void WriteInt(std::string& fileName, std::string& settingName, SInt32 value, bool cache) { WriteString(fileName, settingName, std::to_string(value), cache); }
	void WriteBool(std::string& fileName, std::string& settingName, bool value, bool cache) { WriteString(fileName, settingName, std::string(value ? "1" : "0"), cache); }
	void WriteFloat(std::string& fileName, std::string& settingName, float value, bool cache) { WriteString(fileName, settingName, std::to_string(value), cache); }

	void CloseReader(std::string& fileName) {
		IniHandler::GetInstance().CloseIniCache(fileName);
	}

	std::string ReadString(std::string& fileName, std::string& settingName, std::string& def, bool cache, SInt32 bufferSize) {
		auto pair = ExtractSettingAndKey(settingName);
		auto& section = pair.first;
		auto& key = pair.second;

		if (section.compare("") == 0 || key.compare("") == 0) {
			Logger::Msg("No value was read for setting name: \"" + settingName + "\"");
			return def;
		}

		std::string value;
		// read from cache, creating one if it does not exist
		if (cache) {
			value = IniHandler::GetInstance().GetIniCache(fileName).Read(section, key, def);
		}
		else {
			// read from cache if it exists, but do not create a new one
			if (IniHandler::GetInstance().HasIniCache(fileName)) {
				return ReadString(fileName, settingName, def, true, bufferSize);
			}
			// read without cache
			char* inBuf;

#if LEGENDARY_EDITION
			inBuf = (char*)FormHeap_Allocate(sizeof(char) * (bufferSize + 1));
#else
			inBuf = (char*)Heap_Allocate(sizeof(char) * (bufferSize + 1));
#endif

			if (GetPrivateProfileStringA(section.c_str(), key.c_str(), def.c_str(), inBuf, bufferSize+1, fileName.c_str())) {
				value = std::string(inBuf);
			}
			else {
				FileHelper::FileCannotBeLoaded(fileName);
			}
			Logger::DebugMsg("Read File: " + IniAccess(fileName, section, key) + " value=" + value);
		}
		return value;
	}

	SInt32 ReadInt(std::string& fileName, std::string& settingName, SInt32 def, bool cache) {

		SInt32 value;
		try {
			value = std::stoi(ReadString(fileName, settingName, std::to_string(def), cache, BUFFER_SIZE).c_str());
		}
		catch (std::invalid_argument) {
			value = def;
		}
		return value;
	}

	float ReadFloat(std::string& fileName, std::string& settingName, float def, bool cache) {
		float value = def;
		try {
			value = std::stof(ReadString(fileName, settingName, std::to_string(def), cache, BUFFER_SIZE).c_str());
		}
		catch (std::invalid_argument) {
			value = def;
		}
		return value;
	}



	bool ReadBool(std::string& fileName, std::string& settingName, bool def, bool cache) {
		return ReadInt(fileName, settingName, def ? 1 : 0, cache) == 1;
	}

	bool HasString(std::string& fileName, std::string& settingName, bool cache) {
		auto zero = ReadString(fileName, settingName, std::string("zero"), cache, BUFFER_SIZE);
		auto one = ReadString(fileName, settingName, std::string("one"), cache, BUFFER_SIZE);
		auto result = (zero.compare(std::string("zero")) != 0) || (one.compare(std::string("one")) != 0);
		return result;
	}

	bool HasInt(std::string& fileName, std::string& settingName, bool cache) {
		auto zero = ReadInt(fileName, settingName, 0, cache);
		auto one = ReadInt(fileName, settingName, 1, cache);
		auto result = (zero != 0) || (one != 1);
		return result;
	}

	bool HasFloat(std::string& fileName, std::string& settingName, bool cache) {
		auto zero = ReadFloat(fileName, settingName, 0.0, cache);
		auto one = ReadFloat(fileName, settingName, 1.0, cache);
		auto result = (zero != 0.0) || (one != 1.0);
		return result;
	}

	bool HasBool(std::string& fileName, std::string& settingName, bool cache) {
		bool zero = ReadBool(fileName, settingName, false, cache);
		bool one = ReadBool(fileName, settingName, true, cache);
		auto result = (zero != false) || (one != true);
		return result;
	}
	BSFixedString ToPapyrusString(char* in) {
		return BSFixedString(in);
	}

	BSFixedString ToPapyrusString(std::string in) {
		return BSFixedString(in.c_str());
	}

	std::string ToStdString(BSFixedString in) {
		return std::string(in.data);
	}

	std::string FromPapyrusPath(BSFixedString path) {
		return std::string("Data\\") + path.data;
	}

	void Buffered_CreateBuffer(PAPYRUS_FUNCTION, BSFixedString file) {
		CreateCache(FromPapyrusPath(file));
	}
	void Buffered_WriteBuffer(PAPYRUS_FUNCTION, BSFixedString file) {
		WriteCache(FromPapyrusPath(file));
	}
	void Buffered_CloseBuffer(PAPYRUS_FUNCTION, BSFixedString file) {
		CloseCache(FromPapyrusPath(file));
	}

	SInt32 Papyrus_GetPluginVersion(StaticFunctionTag* base) {
		return PLUGIN_VERSION;
	}

#define DEFINE_FUNCTIONS_PREFIX(Prefix, Type, cType, cache) \
void Prefix##_Write##Type(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString settingName, cType value) { Write##Type(FromPapyrusPath(file), ToStdString(settingName), value, cache);} \
cType Prefix##_Read##Type(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString settingName, cType def) { return Read##Type(FromPapyrusPath(file), ToStdString(settingName) , def, cache);} \
bool Prefix##_Has##Type(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString settingName) { return Has##Type(FromPapyrusPath(file), ToStdString(settingName) , cache);} \
\
cType Prefix##_Read##Type##Ex(PAPYRUS_FUNCTION, BSFixedString fileDefault, BSFixedString fileUser, BSFixedString settingName, cType def) { \
	if(!Has##Type(FromPapyrusPath(fileDefault), ToStdString(settingName), cache)) {\
		Write##Type(FromPapyrusPath(fileDefault), ToStdString(settingName), def, cache); \
	} \
	return Read##Type(FromPapyrusPath(fileUser), ToStdString(settingName), Read##Type(FromPapyrusPath(fileDefault), ToStdString(settingName), def, cache), cache);\
}

#define DEFINE_FUNCTIONS_PREFIX_STRING(Prefix, cache) \
void Prefix##_WriteString(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString settingName, BSFixedString value) { WriteString(FromPapyrusPath(file), ToStdString(settingName), ToStdString(value), cache);} \
BSFixedString Prefix##_ReadString(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString settingName, BSFixedString def, SInt32 bufferSize) { return ToPapyrusString(ReadString(FromPapyrusPath(file), ToStdString(settingName) , ToStdString(def), cache, bufferSize));} \
bool Prefix##_HasString(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString settingName) { return HasString(FromPapyrusPath(file), ToStdString(settingName) , cache);} \
\
BSFixedString Prefix##_ReadString##Ex(PAPYRUS_FUNCTION, BSFixedString fileDefault, BSFixedString fileUser, BSFixedString settingName, BSFixedString def, SInt32 bufferSize) { \
	if(!HasString(FromPapyrusPath(fileDefault), ToStdString(settingName), cache)) {\
		WriteString(FromPapyrusPath(fileDefault), ToStdString(settingName), ToStdString(def), cache); \
	} \
	return ToPapyrusString( ReadString(FromPapyrusPath(fileUser), ToStdString(settingName), ReadString(FromPapyrusPath(fileDefault), ToStdString(settingName), ToStdString(def), cache, bufferSize), cache, bufferSize));\
}



#define DEFINE_FUNCTIONS(Type, cType) \
DEFINE_FUNCTIONS_PREFIX(Papyrus, Type, cType, false) \
DEFINE_FUNCTIONS_PREFIX(Buffered, Type, cType, true)

#define DEFINE_FUNCTIONS_STRING() \
DEFINE_FUNCTIONS_PREFIX_STRING(Papyrus, false) \
DEFINE_FUNCTIONS_PREFIX_STRING(Buffered, true)


	DEFINE_FUNCTIONS(Int, SInt32)
		DEFINE_FUNCTIONS(Float, float)
		DEFINE_FUNCTIONS(Bool, bool)
		DEFINE_FUNCTIONS_STRING()



#define REGISTER_WRITE(Prefix, Type, cType) registry->RegisterFunction( \
new NativeFunction3 <StaticFunctionTag, void, BSFixedString, BSFixedString, cType>("Write" #Type, "PapyrusIni", Papyrus##_Write##Type, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Write" #Type, VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction3 <StaticFunctionTag, void, BSFixedString, BSFixedString, cType>("Write" #Type, "BufferedIni", Buffered##_Write##Type, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Write" #Type, VMClassRegistry::kFunctionFlag_NoWait)

#define REGISTER_READ(Prefix, Type, cType) registry->RegisterFunction( \
new NativeFunction3 <StaticFunctionTag, cType, BSFixedString, BSFixedString, cType>("Read" #Type, "PapyrusIni", Papyrus##_Read##Type, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Read" #Type, VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction3 <StaticFunctionTag, cType, BSFixedString, BSFixedString, cType>("Read" #Type, "BufferedIni", Buffered##_Read##Type, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Read" #Type, VMClassRegistry::kFunctionFlag_NoWait)

#define REGISTER_HAS(Prefix, Type, cType) registry->RegisterFunction( \
new NativeFunction2 <StaticFunctionTag, bool, BSFixedString, BSFixedString>("Has" #Type, "PapyrusIni", Papyrus##_Has##Type, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Has" #Type, VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction2 <StaticFunctionTag, bool, BSFixedString, BSFixedString>("Has" #Type, "BufferedIni", Buffered##_Has##Type, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Has" #Type, VMClassRegistry::kFunctionFlag_NoWait)

#define REGISTER_READ_EX(Prefix, Type, cType) registry->RegisterFunction( \
new NativeFunction4 <StaticFunctionTag, cType, BSFixedString, BSFixedString, BSFixedString, cType>("Read" #Type "Ex", "PapyrusIni", Papyrus##_Read##Type##Ex, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Read" #Type "Ex", VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction4 <StaticFunctionTag, cType, BSFixedString, BSFixedString, BSFixedString, cType>("Read" #Type "Ex", "BufferedIni", Buffered##_Read##Type##Ex, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Read" #Type "Ex", VMClassRegistry::kFunctionFlag_NoWait)

#define REGISTER_ALL(Prefix, Type, cType) \
REGISTER_WRITE(Prefix, Type, cType); \
REGISTER_READ(Prefix, Type, cType);\
REGISTER_HAS(Prefix, Type, cType);\
REGISTER_READ_EX(Prefix, Type, cType)

#define REGISTER_ALL_STRING(Prefix,  Type, cType) \
REGISTER_WRITE(Prefix, Type, cType); \
REGISTER_HAS(Prefix, Type, cType); \
registry->RegisterFunction( \
new NativeFunction5 <StaticFunctionTag, cType, BSFixedString, BSFixedString, BSFixedString, cType, SInt32>("Read" #Type "Ex", "PapyrusIni", Papyrus##_Read##Type##Ex, registry)); \
registry->SetFunctionFlags("PapyrusIni", "Read" #Type "Ex", VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction5 <StaticFunctionTag, cType, BSFixedString, BSFixedString, BSFixedString, cType, SInt32>("Read" #Type "Ex", "BufferedIni", Buffered##_Read##Type##Ex, registry)); \
registry->SetFunctionFlags("BufferedIni", "Read" #Type "Ex", VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction4 <StaticFunctionTag, cType, BSFixedString, BSFixedString, cType, SInt32>("Read" #Type, "PapyrusIni", Papyrus##_Read##Type, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Read" #Type, VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction4 <StaticFunctionTag, cType, BSFixedString, BSFixedString, cType, SInt32>("Read" #Type, "BufferedIni", Buffered##_Read##Type, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Read" #Type, VMClassRegistry::kFunctionFlag_NoWait)


		bool RegisterFuncs(VMClassRegistry* registry) {

		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, SInt32>("GetPluginVersion", "PapyrusIni", Papyrus_GetPluginVersion, registry));
		registry->SetFunctionFlags("PapyrusIni", "GetPluginVersion", VMClassRegistry::kFunctionFlag_NoWait);


		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, BSFixedString>("CreateBuffer", "BufferedIni", Buffered_CreateBuffer, registry));
		registry->SetFunctionFlags("BufferedIni", "CreateBuffer", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, BSFixedString>("WriteBuffer", "BufferedIni", Buffered_WriteBuffer, registry));
		registry->SetFunctionFlags("BufferedIni", "WriteBuffer", VMClassRegistry::kFunctionFlag_NoWait);

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, BSFixedString>("CloseBuffer", "BufferedIni", Buffered_CloseBuffer, registry));
		registry->SetFunctionFlags("BufferedIni", "CloseBuffer", VMClassRegistry::kFunctionFlag_NoWait);

		REGISTER_ALL(Papyrus, Int, SInt32);
		REGISTER_ALL(Papyrus, Float, float);
		REGISTER_ALL(Papyrus, Bool, bool);

		REGISTER_ALL_STRING(Papyrus, String, BSFixedString);

		return true;
	}
}
