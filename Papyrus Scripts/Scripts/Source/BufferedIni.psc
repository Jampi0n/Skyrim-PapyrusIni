scriptname BufferedIni

; Buffered Ini provides the same functions as PapyrusIni, but uses a buffer to reduce the number of file accesses.
; Buffered ini operations should be used when multiple write or read operations are used in quick succession.
; On the first operation a buffer will be created.
; Buffered ini operations will then interact with the buffer instead of the file.
; After the last operation, the buffer needs to be written, so that the buffered writes are forwarded to the ini file.

; Performance Benchmarks:
; Performance depends on the size of the file, the number of sections and the number of keys in the section which is used in the operation.

; This test was performed, with 67 sections, each having 30 keys:

; This test was performed, with 1 section, each having 1 key:
; Repeat: 1000
; Operations                                |    seconds
; 1 writes                                  |       0.70
; 1 buffered writes + 1 close buffer        |       1.12
; 1 reads                                   |       0.28
; 1 buffered reads + 1 close buffer         |       1.13

; This test was performed, with 2 sections, each having 2 keys:
; Repeat: 250
; Operations                                |    seconds
; 4 writes                                  |       5.33
; 4 buffered writes + 1 close buffer        |       0.38
; 4 reads                                   |       0.27
; 4 buffered reads + 1 close buffer         |       0.27


; This test was performed, with 4 sections, each having 5 keys:
; Repeat: 50
; Operations                                |    seconds
; 20 writes                                 |       5.03
; 20 buffered writes + 1 close buffer       |       0.12
; 20 reads                                  |       0.22
; 20 buffered reads + 1 close buffer        |       0.07

; This test was performed, with 10 sections, each having 10 keys:
; Repeat: 10
; Operations                                |    seconds
; 100 writes                                |       6.09
; 100 buffered writes + 1 close buffer      |       0.06
; 100 reads                                 |       0.28
; 100 buffered reads + 1 close buffer       |       0.05

; This test was performed, with 10 sections, each having 25 keys:
; Repeat: 4
; Operations                                |    seconds
; 250 writes                                |       6.45
; 250 buffered writes + 1 close buffer      |       0.07
; 250 reads                                 |       0.27
; 250 buffered reads + 1 close buffer       |       0.05

; This test was performed, with 25 sections, each having 40 keys:
; Repeat: 1
; Operations                                |    seconds
; 1000 writes                               |       7.05
; 1000 buffered writes + 1 close buffer     |       0.05
; 1000 reads                                |       0.25
; 1000 buffered reads + 1 close buffer      |       0.08

; This test was performed, with 75 sections, each having 100 keys:
; Repeat: 1
; Operations                                |    seconds
; 1000 writes                               |      53.14
; 1000 buffered writes + 1 close buffer     |       0.41
; 1000 reads                                |       3.04
; 1000 buffered reads + 1 close buffer      |       0.31

; Conclusion:


; Since buffered ini operations interact with the buffer rather then the file, any changes to the file while the buffer is active will not be seen and will be overridden once the buffer is written back to the file.

; Writes the buffered write operations to the file, but keeps the buffer.
; Should be used, if further buffered ini operations are possible, but not guaranteed.
; If further buffered ini operations are guaranteed, the buffer should be written after them.
Function WriteBuffer(string file) Global Native

; Writes the buffered write operations to the file and closes the buffer.
; Closing the buffer frees memory.
; Closing the buffer allows the ini file to be changed and the changes will be visible the next time a buffer is created for it.
; Closing the buffer requires creating a new buffer the next time a buffered ini operation is used on the file.
; Should be used, if no further buffered ini operations are expected.
Function CloseBuffer(string file) Global Native

; Creates a buffer for the file. In general, buffers are automatically created when needed.
; However this function can be used to create a buffer in advance to avoid buffer creation at a more performance critical moment.
Function CreateBuffer(string file) Global Native

; Combining buffered and non-buffered is possible.
; While a buffer exists, non-buffered ini operations will do the followng:
; Read Operation: Reads the value from the buffer instead.
; Write Operation: Writes the value to the file and updates the buffer, so that the buffer will not override it once it is written.


; Example Usages:
; 1. Read all MCM settings on startup from file. Write MCM settings to file as they are modified.
;       Use buffered read operations to read all settings.
;       Use CloseBuffer once you are done.
;       Use non-buffered writes to save MCM settings as they are modified.
;       Settings are changed one by one, so buffered writes are not required.
;   
;       Advantages:
;           Everything is automatic.
;           Changes can be observed immediately in the ini file.
;           Ini file only contains changes -> can use default/custom ini setup.
;
;       Disadvantages:
;           Outside changes to the ini file are only applied once reloading a save.
;
;       Notes:
;           One could add a Reload button to the MCM to manually apply outside changes.
;           This can be implemented by using CloseBuffer in order to force the creation of a new buffer that contains the changes.
;
; 2. Read all MCM settings on startup from file. Write MCM settings to file when closing the MCM.
;       Use buffered read operations to read all settings.
;       Do not close the buffer.
;       Use buffered writes to save MCM settings as they are changed.
;       Use CloseBuffer once you are done to apply the buffered writes.
;
;       Advantages:
;           Everything is automatic.
;
;       Disadvantages:
;           Outside changes to the ini file are only applied once reloading a save.
;           Writing all settings everytime the MCM is closed can be slow, if a lot of settings are used.
;
;       Notes:
;           Using WriteBuffer instead of CloseBuffer would mean outside changes are overridden by the buffer as it is never closed.
;           Additionally, the buffer would be written even if no settings are changed.
;           With CloseBuffer, the buffer only exists after changing at least one settings, so only in then it will be written.
;           
; 3. Read/write multiple settings when importing/exporting MCM settings via buttons in the MCM.
;       Use buffered read and write operations for import and export.
;       Use CloseBuffer once import/export is done. This allows the player to manually edit the ini file before and after import/export.
;
;       Advantages:
;           Player can edit/replace/delete/create the ini file at any moment, as the file is only buffered during export and import.
;
;       Disadvantages:
;           Import/export needs to be done manually by the player.

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
