@echo off

SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

REM ====================================================================================
REM Batch file for generating Documentation
REM
REM Author  : V Marchenko
REM Date    : 30. October 2019
REM Version : 0.1
REM Company : Arm Limited
REM
REM Command syntax: gen_doc.bat
REM ====================================================================================

REM ====================================================================================
REM Generates documentation 
REM ====================================================================================
pushd .\documentation
CALL genDoc.bat
popd
 
