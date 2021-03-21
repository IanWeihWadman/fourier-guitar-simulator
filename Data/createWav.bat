@echo off
SETLOCAL

REM %1 is tabs file name which is also song name

SET tabs=tabs\
SET strings=strings\%1\
SET wavs=wavfolder\
SET build=Release
SET simulate=..\guitarSimulation\%build%\guitarSimulation.exe
SET parse=..\TabParser\bin\%build%\TabParser.exe
SET wavify=..\WavMaker\bin\%build%\WavMaker.exe

echo %tabs%
echo %strings%

echo %parse%

%parse% rawSongFiles\lovecats.txt

%simulate% rawSongFiles\lovecats.txt.out


%wavify% 


if exist %tabs%%1 (
    rem file exists
    echo exists
    %parse% %tabs%%1 %strings% 
    %simulate% %strings% %simfolder%
    %wavify% %simfolder%\output %wavs%%1
) else (
    echo %1 not found in %tabs%
)
