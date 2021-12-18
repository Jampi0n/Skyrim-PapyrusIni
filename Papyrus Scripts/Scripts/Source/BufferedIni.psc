scriptname BufferedIni

; When to use Buffered ini operations compared to normal ini operations?

;   Buffered read operations always make sense, if you read at least 5 to 10 settings before closing the game (or using CloseBuffer).
;   The file will only be read once and future read operations will use the buffer.
;   Buffered reads are so fast, that you do not need to store the values in papyrus. You can simply read them from the buffer whenever you need them.

;   Buffered write operations only make sense, if you write at least 5 to 10 settings at the same time.
;   Since the game could be closed at any moment, the buffer needs to be written to the file after every sequence of buffered writes (using WriteBuffer or CloseBuffer).
;   For smaller sequences or individual writes you should use non-buffered writes instead.
;   You can combine buffered writes and non-buffered writes. When writing at least 5 to 10 settings at the same time use buffered writes, otherwise use non-buffered ones.

; Outside changes to the ini file:

;   Outside changes are any changes to the ini file not performed by this library. That includes editing the ini, deleting it or creating it (If the ini did not exist while the buffer was created, the buffer will still exist, but will not contain any values.).
;   While a buffer exists, all reads (both buffered and non-buffered reads) and buffered writes interact with the buffer instead of the file.
;   For reads, this means outside changes are not visible and the buffer contains the old values from when the buffer was created.
;   Writing the buffer will override the file with the data from the buffer, overriding any outside changes.
;
;   If outside changes need to be possible, the buffer should be closed after every sequence of buffered reads or buffered writes.
;   In that case, buffered reads should also only be used, if you read at least 5 to 10 settings at the same time.

; Notes

; Since the ReadEx functions also write default values to the default ini file if they do not exist, you also need to write the buffer after using them.


; Writes the buffered write operations to the file, but keeps the buffer open.
; Should be used, if further buffered ini operations are possible, but not guaranteed.
; If further buffered ini operations are guaranteed, the buffer should be written after them.
; This function should be used sparingly. If you find yourself using this function after every other write operation, consider using non-buffered writes.
Function WriteBuffer(string file) Global Native

; Writes the buffered write operations to the file and closes the buffer.
; Closing the buffer allows the ini file to be changed from the outside and the changes will be visible the next time a buffer is created for it.
; Closing the buffer requires creating a new buffer the next time a buffered ini operation is used on the file.
; Should only be used, if no further buffered ini operations are expected or if outside changes to the ini file need to be allowed.
; This function should be used sparingly. If you find yourself using this function after every other write operation, consider using non-buffered writes.
Function CloseBuffer(string file) Global Native

; Creates a buffer for the file. In general, buffers are automatically created when needed.
; However this function can be used to create a buffer in advance to avoid buffer creation at a more performance critical moment.
; This is only needed, if you are dealing with very large ini files (thousands of entries).
Function CreateBuffer(string file) Global Native

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
