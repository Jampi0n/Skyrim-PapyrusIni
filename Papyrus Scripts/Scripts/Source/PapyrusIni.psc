scriptname PapyrusIni

; Parameters:
; string file:
;   filename of the .ini file starting in the Data directory. e.g. "Config\\MyConfigFile.ini"
; settingName:
;   section and key in the ini file separated by a colon. e.g. "MyInt:MySection"
; type value:
;   When writing, this value will be written to the ini file.
; type default:
;   When reading, this value will be returned, if the value could not be read (e.g. ini file does not exist or does not contain the entry)
;
;   For example WriteInt("Config\\MyConfigFile.ini", "MyInt:MySection", 0) will result in:
;
;       [MySection]
;       MyInt = 0
;

; WriteType:
;   Writes to the .ini file and creates it if necesseary.

; ReadType:
;   Reads from the .ini file and returns a default value if it does not exist, has the wrong type or is inaccessible for another reason (permissions for example).

; ReadTypeEx:
;   Tries to read from fileUser. If the setting does not exist in fileUser, reads from fileDefault.
;   If fileDefault does not have the ini value, the default value is written to it and the default value is returned.

; HasType:
;   Returns if the ini has the value of the correct type. Returns false, if the file does not exist or is inaccessible for another reason (permissions for example).
;   If the value exists, but has the wrong type it also returns false.

Int Function GetPluginVersion() Global Native

Function WriteInt(string file, string settingName, int value) Global Native
Function WriteFloat(string file, string settingName, float value) Global Native
Function WriteBool(string file, string settingName, bool value) Global Native
Function WriteString(string file, string settingName, string value) Global Native

Int Function ReadInt(string file, string settingName, int default) Global Native
Float Function ReadFloat(string file, string settingName, float default) Global Native
Bool Function ReadBool(string file, string settingName, bool default) Global Native
String Function ReadString(string file, string settingName, string default) Global Native

Bool Function HasInt(string file, string settingName) Global Native
Bool Function HasFloat(string file, string settingName) Global Native
Bool Function HasBool(string file, string settingName) Global Native
Bool Function HasString(string file, string settingName) Global Native

Int Function ReadIntEx(string fileDefault, string fileUser, string settingName, int default) Global Native
Float Function ReadFloatEx(string fileDefault, string fileUser, string settingName, float default) Global Native
Bool Function ReadBoolEx(string fileDefault, string fileUser, string settingName, bool default) Global Native
String Function ReadStringEx(string fileDefault, string fileUser, string settingName, string default) Global Native

