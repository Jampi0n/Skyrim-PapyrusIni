#include "PapyrusIni.h"

#include <vector>
#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <tuple>
#include <memory>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <sstream>

#include <ShlObj.h>
#include <WinBase.h>
#include "SimpleIni.h"


#define DEBUG 0
#define PAPYRUS_FUNCTION StaticFunctionTag* base

constexpr auto BUFFER_SIZE = 128;
constexpr auto SECTION_KEY_SEP = "::";

namespace PapyrusIni {

	class Logger {
	private:
		static void WriteLine(std::string str) {
			auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);

			std::ostringstream oss;
			oss << std::put_time(&tm, "%d/%m/%Y - %I:%M:%S%p");
			auto time = oss.str();

			_MESSAGE(("[" + time + "] " + str).c_str());
		}
	public:
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
		}

		std::string& Read(std::string section, std::string key, std::string& def) {
			auto value = std::string(ini.GetValue(section.c_str(), key.c_str(), def.c_str()));
			Logger::DebugMsg("Read Cache: " + IniAccess(path, section, key) + " value=" + value);
			return value;
		}

		void Write(std::string& section, std::string& key, std::string& value) {
			Logger::DebugMsg("Write Cache: " + IniAccess(path, section, key) + " value=" + value);
			modified = true;
			ini.SetValue(section.c_str(), key.c_str(), value.c_str());
		}

		void Save() {
			Logger::Msg("Save Cache: {" + path + "}");
			if (modified) {
				ini.SaveFile(path.c_str());
			}
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
			Logger::DebugMsg("GetIniCache: {" + path + "}");
			if (fileReaders.find(path) == fileReaders.end()) {
				Logger::DebugMsg("GetIniCache: {" + path + "}->new");
				fileReaders.emplace(path, std::make_unique<IniCache>(path));
			}
			return *fileReaders.at(path).get();
		}

		/// <summary>
		/// Closes the IniCache for the specified path, writing the changes and freeing memory.
		/// </summary>
		/// <param name="path">Path to the .ini file.</param>
		void CloseIniCache(std::string path) {
			Logger::DebugMsg("CloseIniCache: {" + path + "}");
			if (fileReaders.find(path) != fileReaders.end()) {
				Logger::DebugMsg("CloseIniCache: {" + path + "}->new");
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
	BSFixedString ToPapyrusString(char* in) {
		return BSFixedString(in);
	}

	BSFixedString ToPapyrusString(std::string in) {
		return ToPapyrusString(in.c_str());
	}
	
	std::string ToStdString(BSFixedString in) {
		return std::string(in.data);
	}
	
	void Buffered_CreateBuffer(PAPYRUS_FUNCTION, BSFixedString file) {
		CreateCache(ToStdString(file));
	}
	void Buffered_WriteBuffer(PAPYRUS_FUNCTION, BSFixedString file) {
		WriteCache(ToStdString(file));
	}
	void Buffered_CloseBuffer(PAPYRUS_FUNCTION, BSFixedString file) {
		CloseCache(ToStdString(file));
	}

	SInt32 Papyrus_GetPluginVersion(StaticFunctionTag* base) {
		return PLUGIN_VERSION;
	}


#define DEFINE_FUNCTIONS_PREFIX(Prefix, Type, cType, cache, in, out) \
void Prefix##_Write##Type(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString section, BSFixedString key, cType value) { Write##Type(ToStdString(file), ToStdString(section), ToStdString(key), in(value), cache);} \
cType Prefix##_Read##Type(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString section, BSFixedString key, cType def) { return out(Read##Type(ToStdString(file), ToStdString(section), ToStdString(key), in(def), cache));} \
bool Prefix##_Has##Type(PAPYRUS_FUNCTION, BSFixedString file, BSFixedString section, BSFixedString key) { return Has##Type(ToStdString(file), ToStdString(section), ToStdString(key), cache);} \
\
cType Prefix##_Read##Type##Ex(PAPYRUS_FUNCTION, BSFixedString fileDefault, BSFixedString fileUser, BSFixedString section, BSFixedString key, cType def) { \
	if(!Has##Type(ToStdString(fileDefault), ToStdString(section), ToStdString(key), cache)) {\
		Write##Type(ToStdString(fileDefault), ToStdString(section), ToStdString(key), in(def), cache); \
	} \
	return out(Read##Type(ToStdString(fileUser), ToStdString(section), ToStdString(key), Read##Type(ToStdString(fileDefault), ToStdString(section), ToStdString(key), in(def), cache), cache));\
}

#define DEFINE_FUNCTIONS(Type, cType, in, out) \
DEFINE_FUNCTIONS_PREFIX(Papyrus, Type, cType, false, in, out) \
DEFINE_FUNCTIONS_PREFIX(Buffered, Type, cType, true, in, out)

DEFINE_FUNCTIONS(Int, SInt32, , )
DEFINE_FUNCTIONS(Float, float, , )
DEFINE_FUNCTIONS(Bool, bool, , )
DEFINE_FUNCTIONS(String, BSFixedString, ToStdString, ToPapyrusString)

#define REGISTER_WRITE(Prefix, Type, cType) registry->RegisterFunction( \
new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, cType>("Write" #Type, "PapyrusIni", Papyrus##_Write##Type, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Write" #Type, VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction4 <StaticFunctionTag, void, BSFixedString, BSFixedString, BSFixedString, cType>("Write" #Type, "BufferedIni", Buffered##_Write##Type, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Write" #Type, VMClassRegistry::kFunctionFlag_NoWait)

#define REGISTER_READ(Prefix, Type, cType) registry->RegisterFunction( \
new NativeFunction4 <StaticFunctionTag, cType, BSFixedString, BSFixedString, BSFixedString, cType>("Read" #Type, "PapyrusIni", Papyrus##_Read##Type, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Read" #Type, VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction4 <StaticFunctionTag, cType, BSFixedString, BSFixedString, BSFixedString, cType>("Read" #Type, "BufferedIni", Buffered##_Read##Type, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Read" #Type, VMClassRegistry::kFunctionFlag_NoWait)

#define REGISTER_HAS(Prefix, Type, cType) registry->RegisterFunction( \
new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("Has" #Type, "PapyrusIni", Papyrus##_Has##Type, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Has" #Type, VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction3 <StaticFunctionTag, bool, BSFixedString, BSFixedString, BSFixedString>("Has" #Type, "BufferedIni", Buffered##_Has##Type, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Has" #Type, VMClassRegistry::kFunctionFlag_NoWait)

#define REGISTER_READ_EX(Prefix, Type, cType) registry->RegisterFunction( \
new NativeFunction5 <StaticFunctionTag, cType, BSFixedString, BSFixedString, BSFixedString, BSFixedString, cType>("Read" #Type "Ex", "PapyrusIni", Papyrus##_Read##Type##Ex, registry)); \
	registry->SetFunctionFlags("PapyrusIni", "Read" #Type "Ex", VMClassRegistry::kFunctionFlag_NoWait); \
registry->RegisterFunction( \
new NativeFunction5 <StaticFunctionTag, cType, BSFixedString, BSFixedString, BSFixedString, BSFixedString, cType>("Read" #Type "Ex", "BufferedIni", Buffered##_Read##Type##Ex, registry)); \
	registry->SetFunctionFlags("BufferedIni", "Read" #Type "Ex", VMClassRegistry::kFunctionFlag_NoWait)

#define REGISTER_ALL(Prefix, Type, cType) \
REGISTER_WRITE(Prefix, Type, cType); \
REGISTER_READ(Prefix, Type, cType);\
REGISTER_HAS(Prefix, Type, cType);\
REGISTER_READ_EX(Prefix, Type, cType)


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
		REGISTER_ALL(Papyrus, String, BSFixedString);

		return true;
	}
}
