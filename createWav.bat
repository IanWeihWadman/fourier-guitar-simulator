REM %1 is tabs file name

SET config=Release
SET simulate=guitarSimulation\%config%\guitarSimulation.exe
SET parse=TabParser\bin\%config%\TabParser.exe
SET wavify=WavMaker\bin\%config%\WavMaker.exe

if exist rawSongFiles\%1 (
    %parse% %1
    %simulate% %1
    %wavify% %1
) else (
    echo %1 not found 
)
