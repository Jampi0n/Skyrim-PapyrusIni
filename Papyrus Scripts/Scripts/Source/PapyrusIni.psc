scriptname PapyrusIni

; Parameters:
; string file:
;   filename of the .ini file starting in the Data directory. e.g. "Config\\MyConfigFile.ini"
; string section:
;   section in the ini file. e.g. "MySection"
; string key:
;   key of the ini entry. e.g. "MyInt"
; type value:
;   When writing, this value will be written to the ini file.
; type default:
;   When reading, this value will be returned, if the value could not be read (e.g. ini file does not exist or does not contain the entry)
;
;   For example WriteInt("Config\\MyConfigFile.ini", "MySection", "MyInt", 0) will result in:
;
;       [MySection]
;       MyInt = 0
;

; WriteType:
;   writes to the .ini file and creates it if necesseary

; ReadType:
;   reads from the .ini file and returns a default value if it does not exist

; ReadTypeEx:
;   if fileDefault does not have the ini value, the default value is written to it.
;   Tries to read from fileUser. If the setting does not exist in fileUser, reads from fileDefault instead (always exists).

; HasType:
;   returns if the ini has the value

Int Function GetPluginVersion() Global Native

Function WriteInt(string file, string section, string key, int value) Global Native
Int Function ReadInt(string file, string section, string key, int default) Global Native
Int Function ReadIntEx(string fileDefault, string fileUser, string section, string key, int default) Global Native
Bool Function HasInt(string file, string section, string key) Global Native

Function WriteFloat(string file, string section, string key, float value) Global Native
Float Function ReadFloat(string file, string section, string key, float default) Global Native
Float Function ReadFloatEx(string fileDefault, string fileUser, string section, string key, float default) Global Native
Bool Function HasFloat(string file, string section, string key) Global Native

Function WriteBool(string file, string section, string key, bool value) Global Native
Bool Function ReadBool(string file, string section, string key, bool default) Global Native
Bool Function ReadBoolEx(string fileDefault, string fileUser, string section, string key, bool default) Global Native
Bool Function HasBool(string file, string section, string key) Global Native

Function WriteString(string file, string section, string key, string value) Global Native
String Function ReadString(string file, string section, string key, string default) Global Native
String Function ReadStringEx(string fileDefault, string fileUser, string section, string key, string default) Global Native
Bool Function HasString(string file, string section, string key) Global Native
