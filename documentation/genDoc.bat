@echo off

REM ====================================================================================
REM Batch file for generating
REM
REM Author  : Vladimir Marchenko
REM Date    : 4th April 2019
REM Version : 1.0
REM Company : ARM 
REM
REM 
REM Command syntax: genDoc.bat
REM
REM  Version: 1.0 Initial Version.
REM ====================================================================================

SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

REM -- Delete previous generated HTML files ---------------------
  ECHO.
  ECHO Delete previous generated HTML files

REM -- Remove generated doxygen files ---------------------
IF EXIST .\html (RMDIR /S /Q .\html)

REM -- Generate New HTML Files ---------------------
  ECHO.
  ECHO Generate New HTML Files

doxygen IoT_Socket.dxy

REM -- Copy search style sheet ---------------------
ECHO Copy search style sheets
copy /Y Doxygen_Templates\search.css html\search\

:END
  ECHO.
REM done