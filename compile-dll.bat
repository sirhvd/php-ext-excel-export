@ECHO OFF

REM setting info box
@ECHO ############################################################################
@ECHO ##                                                                        ##
@ECHO ## Please install MS Visual C++ 2019 Build Tools                          ##
@ECHO ##                                                                        ##
@ECHO ############################################################################

@ECHO.

REM setting PHP version
SET PHPVERSION=7.1.3
SET PHPMAJOR=%PHPVERSION:~0,3%

REM setting full path of current directory to %DIR&
SET DIR=%~dp0
SET DIR=%Dir:~0,-1%

REM check for .\downloads directory
IF NOT EXIST "%DIR%\downloads" (
    @ECHO.
    @ECHO creating .\downloads directory
    MD %DIR%\downloads
)

REM check for .\build-cache directory
IF NOT EXIST "%DIR%\build-cache" (
    @ECHO.
    @ECHO creating .\build-cache directory
    MD %DIR%\build-cache
)

REM check for .\tmp directory
IF NOT EXIST "%DIR%\tmp" (
    @ECHO.
    @ECHO creating .\tmp directory
    MD %DIR%\tmp
)

REM adding current directory and ./downloads to path
SET PATH=%PATH%;%DIR%;%DIR%\downloads;

REM -----------------------------------------------------------
REM --- TOOLS CHECK
REM -----------------------------------------------------------

REM check for wget availability
wget >nul 2>&1
IF %ERRORLEVEL%==9009 (
    REM since wget is not available look if PHP is available and try to download wget from web with PHP
    php -v >nul 2>&1
    IF NOT %ERRORLEVEL%==9009 (
        REM download wget with php
        @ECHO.
        @ECHO loading wget...
        php -r "file_put_contents('%DIR%\downloads\wget.exe',file_get_contents('https://eternallybored.org/misc/wget/current/wget.exe'));"
    )

    REM if wget download with PHP failed try to download with bitsadmin.exe
    IF NOT EXIST "%DIR%\downloads\wget.exe" (
        REM checking for bitsadmin.exe to download wget.exe from web source
        IF NOT EXIST "%SYSTEMROOT%\System32\bitsadmin.exe" (
            @ECHO.
            @ECHO wget.exe not available
            @ECHO failed to download wget.exe automatically
            @ECHO please download wget from https://eternallybored.org/misc/wget/current/wget.exe
            @ECHO manually and put the wget.exe file in .\downloads folder
            @ECHO it is also available from the php-sdk-binary-tools zip archive
            PAUSE
            EXIT
        )

        REM bitsadmin.exe is available but wget.exe is not - so download it from web
        @ECHO.
        @ECHO loading wget for Windows from...
        @ECHO https://eternallybored.org/misc/wget/current/wget.exe
        bitsadmin.exe /transfer "WgetDownload" "https://eternallybored.org/misc/wget/current/wget.exe" "%DIR%\downloads\wget.exe"
    )

    REM if download of wget failed stop script
    IF NOT EXIST "%DIR%\downloads\wget.exe" (
        @ECHO.
        @ECHO loading wget failed. Please re-run script or
        @ECHO install .\downloads\wget.exe manually
        PAUSE
        EXIT
    )
)

SET /P USE_PROXY=Do you want to use proxy for wget? [y/n]

IF /I %USE_PROXY%==Y (
	REM setting proxy for wget
	SETX "http_proxy" "proxy.hcm.fpt.vn:80"
	SETX "https_proxy" "proxy.hcm.fpt.vn:80"
)

REM check for 7-zip cli tool
7za >nul 2>&1
IF %ERRORLEVEL%==9009 (
    @ECHO.
    @ECHO loading 7-zip cli tool from web...
    wget http://downloads.sourceforge.net/sevenzip/7za920.zip -O %DIR%\downloads\7za920.zip -N

    REM if wget download of 7za failed stop script
    IF NOT EXIST "%DIR%\downloads\7za920.zip" (
        @ECHO.
        @ECHO failed to download 7za920.zip - please re-run this script
        PAUSE
        EXIT
    )

    REM if php is available try unpacking 7za with php
    php -v >nul 2>&1
    IF NOT %ERRORLEVEL%==9009 (
        @ECHO.
        @ECHO unpacking 7za.exe...
        php -r "file_put_contents('%DIR%\downloads\7za.exe',file_get_contents('zip://%DIR%/downloads/7za920.zip#7za.exe'));"
    )

    REM if unpacking 7za with PHP failed try to unpacking with unzip
    IF NOT EXIST "%DIR%\downloads\7za.exe" (
        REM check if unzip.exe is available to unpack 7-zip
        unzip >nul 2>&1
        IF %ERRORLEVEL%==9009 (
            REM check for unzip tool in Git\bin
            IF EXIST "%PROGRAMFILES(X86)%\Git\bin\unzip.exe" (
                @ECHO.
                @ECHO copying unzip.exe from Git...
                COPY "%PROGRAMFILES(X86)%\Git\bin\unzip.exe" "%DIR%\downloads\"
            )
        )

        REM unpacking 7za920.zip
        @ECHO.
        @ECHO unpacking 7-zip cli tool...
        CD %DIR%\downloads
        unzip -C 7za920.zip 7za.exe
        CD %DIR%
    )

	REM if unpacking 7za with unzip failed try to unpacking with vbs
    IF NOT EXIST "%DIR%\downloads\7za.exe" (
    	Call :UnZipFile "%DIR%\downloads" "%DIR%\downloads\7za920.zip"
    )
)

7za >nul 2>&1
IF %ERRORLEVEL%==9009 (
    @ECHO.
    @ECHO 7za.exe not found - please re-run this script
    PAUSE
    EXIT
)

REM -----------------------------------------------------------
REM --- PHP DEVEL SOURCE PREPARATION
REM -----------------------------------------------------------

IF NOT EXIST "%DIR%\downloads\php-devel-pack-7.1.3-Win32-VC14-x64.zip" (
    @ECHO.
    @ECHO loading php devel source code...
    wget https://windows.php.net/downloads/releases/archives/php-devel-pack-7.1.3-Win32-VC14-x64.zip -O %DIR%\downloads\php-devel-pack-7.1.3-Win32-VC14-x64.zip -N
)

IF NOT EXIST "%DIR%\downloads\php-devel-pack-7.1.3-Win32-VC14-x64.zip" (
    @ECHO.
    @ECHO php devel source code not found in .\downloads please re-run this script
    PAUSE
    EXIT
)

@ECHO.
@ECHO unpacking php-sdk-binary tools...
7za x %DIR%\downloads\php-devel-pack-7.1.3-Win32-VC14-x64.zip -o%DIR%/downloads -y

REM check for .\php-devel-pack-7.1.3-Win32-VC14-x64 directory
IF NOT EXIST "%DIR%\downloads\php-7.1.3-devel-VC14-x64" (
    @ECHO.
    @ECHO php devel source code not found in .\downloads please re-run this script
    PAUSE
    EXIT
)

XCOPY /s /y /i "%DIR%\downloads\php-7.1.3-devel-VC14-x64" "%DIR%\build-cache\php-7.1.3-devel-VC14-x64"

REM -----------------------------------------------------------
REM --- PHP SDK PREPARATION
REM -----------------------------------------------------------

IF NOT EXIST "%DIR%\downloads\php-sdk-binary-tools-php-sdk-2.1.2.zip" (
    @ECHO.
    @ECHO loading php-sdk-binary tools...
    wget https://github.com/Microsoft/php-sdk-binary-tools/archive/php-sdk-2.1.2.zip -O %DIR%\downloads\php-sdk-binary-tools-php-sdk-2.1.2.zip -N
)

IF NOT EXIST "%DIR%\downloads\php-sdk-binary-tools-php-sdk-2.1.2.zip" (
    @ECHO.
    @ECHO php-sdk-binary tools zip file not found in .\build-cache please re-run this script
    PAUSE
    EXIT
)

@ECHO.
@ECHO unpacking php-sdk-binary tools...
7za x %DIR%\downloads\php-sdk-binary-tools-php-sdk-2.1.2.zip -o%DIR%/downloads -y

REM check for .\php-sdk-binary-tools-php-sdk-2.1.2 directory
IF NOT EXIST "%DIR%\downloads\php-sdk-binary-tools-php-sdk-2.1.2" (
    @ECHO.
    @ECHO php-sdk-binary tools zip file not found in .\build-cache please re-run this script
    PAUSE
    EXIT
)

XCOPY /s /y /i "%DIR%\downloads\php-sdk-binary-tools-php-sdk-2.1.2" "%DIR%\build-cache\php-sdk-2.1.2"

REM -----------------------------------------------------------
REM --- EXTENSION DEPENDENCIES PREPARATION
REM -----------------------------------------------------------

IF NOT EXIST "%DIR%\downloads\zlib-1.2.8-vc14-x64.zip" (
    @ECHO.
    @ECHO loading zlib library...
    wget https://windows.php.net/downloads/php-sdk/deps/vc14/x64/zlib-1.2.8-vc14-x64.zip -O %DIR%\downloads\zlib-1.2.8-vc14-x64.zip -N
)

IF NOT EXIST "%DIR%\downloads\zlib-1.2.8-vc14-x64.zip" (
    @ECHO.
    @ECHO zlib library not found in .\downloads please re-run this script
    PAUSE
    EXIT
)

@ECHO.
@ECHO unpacking zlib library...
7za x %DIR%\downloads\zlib-1.2.8-vc14-x64.zip -o%DIR%\build-cache\zlib -y

XCOPY /s /y /i "%DIR%\build-cache\zlib" "%DIR%\build-cache\libxlsxwriter"

REM -----------------------------------------------------------
REM --- COMPILING libxlsxwriter EXTENSION 
REM -----------------------------------------------------------

IF NOT EXIST "%DIR%\downloads\zlib-src.zip" (
    @ECHO.
    @ECHO loading zlib source...
    wget https://github.com/madler/zlib/archive/master.zip -O %DIR%\downloads\zlib-src.zip -N
)

IF NOT EXIST "%DIR%\downloads\zlib-src.zip" (
    @ECHO.
    @ECHO zlib source not found in .\downloads please re-run this script
    PAUSE
    EXIT
)

@ECHO.
@ECHO unpacking zlib source...
7za x %DIR%\downloads\zlib-src.zip -o%DIR%/downloads -y

IF NOT EXIST "%DIR%\downloads\zlib-master" (
    @ECHO.
    @ECHO zlib source not found in .\downloads please re-run this script
    PAUSE
    EXIT
)

XCOPY /s /y /i "%DIR%\downloads\zlib-master" "%DIR%\tmp\zlib"

IF NOT EXIST "%DIR%\tmp\zlib" (
    @ECHO.
    @ECHO zlib source not found in .\tmp please re-run this script
    PAUSE
    EXIT
)

IF NOT EXIST "%DIR%\downloads\libxlsxwriter-src.zip" (
    @ECHO.
    @ECHO loading libxlsxwriter source...
    wget https://github.com/jmcnamara/libxlsxwriter/archive/master.zip -O %DIR%\downloads\libxlsxwriter-src.zip -N
)

IF NOT EXIST "%DIR%\downloads\libxlsxwriter-src.zip" (
    @ECHO.
    @ECHO libxlsxwriter source not found in .\downloads please re-run this script
    PAUSE
    EXIT
)

@ECHO.
@ECHO unpacking libxlsxwriter source...
7za x %DIR%\downloads\libxlsxwriter-src.zip -o%DIR%/downloads -y

IF NOT EXIST "%DIR%\downloads\libxlsxwriter-master" (
    @ECHO.
    @ECHO libxlsxwriter source not found in .\downloads please re-run this script
    PAUSE
    EXIT
)

XCOPY /s /y /i "%DIR%\downloads\libxlsxwriter-master" "%DIR%\tmp\libxlsxwriter"

IF NOT EXIST "%DIR%\tmp\libxlsxwriter" (
    @ECHO.
    @ECHO libxlsxwriter source not found in .\tmp please re-run this script
    PAUSE
    EXIT
)

REM check for .tmp\install_dir directory
IF NOT EXIST "%DIR%\tmp\install_dir" (
    @ECHO.
    @ECHO creating .tmp\install_dir directory
    MD %DIR%\tmp\install_dir
)

IF NOT EXIST "%DIR%\downloads\php_xlswriter_src.zip" (
    @ECHO.
    @ECHO loading php_xlswriter source...
    wget https://github.com/sirhvd/php-ext-excel-export/archive/master.zip -O %DIR%\downloads\php_xlswriter_src.zip -N
)

IF NOT EXIST "%DIR%\downloads\php_xlswriter_src.zip" (
    @ECHO.
    @ECHO php_xlswriter source not found in .\downloads please re-run this script
    PAUSE
    EXIT
)

@ECHO.
@ECHO unpacking php_xlswriter source...
7za x %DIR%\downloads\php_xlswriter_src.zip -o%DIR%/downloads -y

XCOPY /s /y /i "%DIR%\downloads\php-ext-excel-export-master" "%DIR%\xlswriter"

REM check for .\xlsxwriter directory
IF NOT EXIST "%DIR%\xlswriter" (
    @ECHO.
    @ECHO php_xlswriter source not found in .\ please re-run this script
    PAUSE
    EXIT
)

REM run step1 & step2 in native tools command prompt
REM cd %DIR%
REM step1-compile-deps.bat
REM step2-compile-deps.bat
PAUSE

@ECHO SET DIR=%%~dp0> step1-compile-deps.bat
@ECHO SET DIR=%%Dir:~0,-1%%>> step1-compile-deps.bat

@ECHO CD %%DIR%%\tmp\zlib>> step1-compile-deps.bat
@ECHO MKDIR build>> step1-compile-deps.bat
@ECHO CD build>> step1-compile-deps.bat
@ECHO cmake .. -G "Visual Studio 14 Win64" -DCMAKE_INSTALL_PREFIX:PATH="%%DIR%%/tmp/install_dir/zlib">> step1-compile-deps.bat
@ECHO cmake --build . --config Release --target install>> step1-compile-deps.bat

@ECHO CD %%DIR%%\tmp\libxlsxwriter>> step1-compile-deps.bat
@ECHO MKDIR build>> step1-compile-deps.bat
@ECHO CD build>> step1-compile-deps.bat
@ECHO cmake .. -G "Visual Studio 14 Win64" -DCMAKE_INSTALL_PREFIX:PATH="%%DIR%%/tmp/install_dir/libxlsxwriter" -DZLIB_ROOT:STRING="%%DIR%%/tmp/install_dir/zlib">> step1-compile-deps.bat
@ECHO cmake --build . --config Release --target install>> step1-compile-deps.bat
@ECHO CD %%DIR%%>> step1-compile-deps.bat
@ECHO PAUSE>> step1-compile-deps.bat

@ECHO @ECHO OFF> step2-compile-final.bat
@ECHO SET DIR=%%~dp0>> step2-compile-final.bat
@ECHO SET DIR=%%Dir:~0,-1%%>> step2-compile-final.bat
@ECHO IF NOT EXIST "%%DIR%%\tmp\install_dir\libxlsxwriter" ^(>> step2-compile-final.bat
@ECHO @ECHO.>> step2-compile-final.bat
@ECHO @ECHO libxlsxwriter compilied not found - please re-run step1 script>> step2-compile-final.bat
@ECHO PAUSE>> step2-compile-final.bat
@ECHO EXIT>> step2-compile-final.bat
@ECHO ^)>> step2-compile-final.bat
@ECHO SETX PATH "%%DIR%%\build-cache\php-7.1.3-devel-VC14-x64">> step2-compile-final.bat
@ECHO XCOPY /s /y "%%DIR%%\tmp\install_dir\libxlsxwriter\include\*.*" "%%DIR%%\build-cache\libxlsxwriter/include">> step2-compile-final.bat
@ECHO XCOPY /s /y "%%DIR%%\tmp\install_dir\libxlsxwriter\lib\x64\Release\*.*" "%%DIR%%\build-cache\libxlsxwriter\lib">> step2-compile-final.bat
@ECHO @ECHO CD %%DIR%%\xlswriter^> task.bat>> step2-compile-final.bat
@ECHO @ECHO call phpize 2^>^&1^>^> task.bat>> step2-compile-final.bat
@ECHO @ECHO @ECHO OFF^>^> task.bat>> step2-compile-final.bat
@ECHO @ECHO @ECHO var PHP_SECURITY_FLAGS = 'yes';^^^>^^^> configure.js^>^> task.bat>> step2-compile-final.bat
@ECHO @ECHO call configure --with-xlswriter --with-extra-libs=%%DIR%%\build-cache\libxlsxwriter\lib --with-extra-includes=%%DIR%%\build-cache\libxlsxwriter\include --enable-debug-pack 2^>^&1^>^> task.bat>> step2-compile-final.bat
@ECHO @ECHO nmake /nologo 2^>^&1^>^> task.bat>> step2-compile-final.bat
@ECHO @ECHO PAUSE^>^> task.bat>> step2-compile-final.bat
@ECHO CD %%DIR%%\xlswriter>> step2-compile-final.bat
@ECHO %%DIR%%\build-cache\php-sdk-2.1.2\phpsdk-vc14-x64.bat -t %%DIR%%\task.bat>> step2-compile-final.bat
@ECHO CD %%DIR%%>> step2-compile-final.bat
@ECHO PAUSE>> step2-compile-final.bat

PAUSE
EXIT

:UnZipFile <ExtractTo> <newzipfile>
set vbs="%temp%\_.vbs"
if exist %vbs% del /f /q %vbs%
>%vbs%  echo Set fso = CreateObject("Scripting.FileSystemObject")
>>%vbs% echo If NOT fso.FolderExists(%1) Then
>>%vbs% echo fso.CreateFolder(%1)
>>%vbs% echo End If
>>%vbs% echo set objShell = CreateObject("Shell.Application")
>>%vbs% echo set FilesInZip=objShell.NameSpace(%2).items
>>%vbs% echo call objShell.NameSpace(%1).CopyHere(FilesInZip, 16)
>>%vbs% echo Set fso = Nothing
>>%vbs% echo Set objShell = Nothing
cscript //nologo %vbs%
if exist %vbs% del /f /q %vbs%