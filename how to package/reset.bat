@echo off

SET PKGDIR="pkg"

del /Q /F package
del /Q /F %PKGDIR%

mkdir %PKGDIR%
mkdir %PKGDIR%\files
mkdir %PKGDIR%\files-win
mkdir %PKGDIR%\files-posix
mkdir %PKGDIR%\files\scripts
mkdir %PKGDIR%\files-win\scripts
mkdir %PKGDIR%\files-posix\scripts
mkdir %PKGDIR%\xml
mkdir %PKGDIR%\xml-win
mkdir %PKGDIR%\xml-posix
mkdir %PKGDIR%\agent-files

copy plugin_settings.example.txt %PKGDIR%\plugin_settings.txt
copy readme.txt %PKGDIR%\readme.txt

echo "Done."