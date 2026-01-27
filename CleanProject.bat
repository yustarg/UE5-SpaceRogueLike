@echo off
echo ========================================
echo 清理 VibeCoding 项目临时文件
echo ========================================
echo.
echo 这将删除以下文件夹：
echo - Binaries
echo - Intermediate
echo - DerivedDataCache
echo - Saved
echo.
echo 这些文件夹会在重新编译时自动生成。
echo.
pause

echo.
echo 正在删除 Binaries...
if exist Binaries rmdir /s /q Binaries

echo 正在删除 Intermediate...
if exist Intermediate rmdir /s /q Intermediate

echo 正在删除 DerivedDataCache...
if exist DerivedDataCache rmdir /s /q DerivedDataCache

echo 正在删除 Saved...
if exist Saved rmdir /s /q Saved

echo.
echo ========================================
echo 清理完成！
echo ========================================
echo.
echo 下次打开项目时，这些文件会自动重新生成。
echo.
pause
