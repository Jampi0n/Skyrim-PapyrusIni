scriptname PapyrusIni

Function WriteInt(string file, string category, string name, int value) Global Native
Int Function ReadInt(string file, string category, string name, int default) Global Native

Function WriteFloat(string file, string category, string name, float value) Global Native
Float Function ReadFloat(string file, string category, string name, float default) Global Native

Function WriteBool(string file, string category, string name, bool value) Global Native
Bool Function ReadBool(string file, string category, string name, bool default) Global Native

Function WriteString(string file, string category, string name, string value) Global Native
String Function ReadString(string file, string category, string name, string default) Global Native
