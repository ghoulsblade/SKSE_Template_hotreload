@echo off
set SOURCE_DLL=%1
set DEST_DLL=%2
copy /Y "%SOURCE_DLL%" "%DEST_DLL%"
if errorlevel 1 (
    echo Warning: Could not overwrite %DEST_DLL%. It might be in use.
)
