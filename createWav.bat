SETLOCAL

REM %1 is tabs file name which is also song name

REM SET tabs=Data\rawSongFiles\
REM SET strings=Data\simOut\%1\
REM SET wavs=wavfolder\
SET config=Release
SET simulate=guitarSimulation\%config%\guitarSimulation.exe
SET parse=TabParser\bin\%config%\TabParser.exe
SET wavify=WavMaker\bin\%config%\WavMaker.exe

echo %tabs%
echo %strings%

echo %parse%

REM %parse% Data\rawSongFiles\lovecats.txt

REM %simulate% Data\rawSongFiles\lovecats.txt

if exist rawSongFiles\%1 (
    rem file exists
    echo exists
    %parse% %1
    %simulate% %1
    REM %wavify% %simfolder%\output %wavs%%1
) else (
    echo %1 not found in %tabs%
)
