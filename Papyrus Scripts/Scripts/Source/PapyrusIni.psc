scriptname PapyrusIni

; file: fileName of the .ini file starting in the Data directory. e.g. "Config\\MyConfigFile.ini"
; category: category in the ini file. e.g. "MyCategory"
; name: name of the ini entry. e.g. "MyInt"
; WriteInt("Config\\MyConfigFile.ini", "MyCategory", "MyInt", 0) will result in:

; [MyCategory]
; MyInt = 0

; WriteType:
;   writes to the .ini file and creates it if necesseary

; ReadType:
;   reads from the .ini file and returns a default value if it does not exist

; ReadTypeEx:
;   if fileDefault does not have the ini value, the default value is written to it.
;   Tries to read from fileUser. If the setting does not exist in fileUser, reads from fileDefault instead (always exists).

; HasType:
;   returns if the ini has the value

Function WriteInt(string file, string category, string name, int value) Global Native
Int Function ReadInt(string file, string category, string name, int default) Global Native
Int Function ReadIntEx(string fileDefault, string fileUser, string category, string name, int default) Global Native
Bool Function HasInt(string file, string category, string name) Global Native

Function WriteFloat(string file, string category, string name, float value) Global Native
Float Function ReadFloat(string file, string category, string name, float default) Global Native
Float Function ReadFloatEx(string fileDefault, string fileUser, string category, string name, float default) Global Native
Bool Function HasFloat(string file, string category, string name) Global Native

Function WriteBool(string file, string category, string name, bool value) Global Native
Bool Function ReadBool(string file, string category, string name, bool default) Global Native
Bool Function ReadBoolEx(string fileDefault, string fileUser, string category, string name, bool default) Global Native
Bool Function HasBool(string file, string category, string name) Global Native

Function WriteString(string file, string category, string name, string value) Global Native
String Function ReadString(string file, string category, string name, string default) Global Native
String Function ReadStringEx(string fileDefault, string fileUser, string category, string name, string default) Global Native
Bool Function HasString(string file, string category, string name) Global Native
