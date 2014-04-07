CopyEngineBin.bat

set "binin=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\BIN\"
set "buildin=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\BUILD\"
set "binout=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\"

copy %binin%AntTweakBar.dll %binout%AntTweakBar.dll
copy %binin%glfw3.dll %binout%glfw3.dll
copy %binin%libgcc_s_dw2-1.dll %binout%libgcc_s_dw2-1.dll
copy %binin%"libstdc++-6.dll" %binout%"libstdc++-6.dll"
copy MorglodsRenderTest.exe %binout%MorglodsRenderTest.exe