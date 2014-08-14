@set THIS_DIR=%~dp0
@cd %THIS_DIR%

@echo KIARA SDK Environment
@path %PATH%;%THIS_DIR%\bin;%THIS_DIR%\lib
@set KIARA_MODULE_PATH=%THIS_DIR%\lib

@kiara-version
@cmd /k
