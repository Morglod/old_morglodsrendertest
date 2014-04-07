cd "C:\Users\VolodyaM\Documents\SharpDevelop Projects\MoConverter\bin\Debug\"
set "MDIR=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\Resources\Models\"
set "BUILDRES=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\BuildedRes\"
MoConverter type=model input=%MDIR%dragon.obj output=%BUILDRES%Dragon.momodel model_flip_uvs=true
MoConverter type=model input=%MDIR%SkyMap.obj output=%BUILDRES%SkyMap.momodel model_flip_uvs=true
MoConverter type=model input=%MDIR%NanosuitObj.obj output=%BUILDRES%Nanosuit.momodel model_flip_uvs=true model_maximum=true
MoConverter type=model input=%MDIR%sibenik_blender.obj output=%BUILDRES%Sibenik.momodel model_flip_uvs=true
MoConverter type=model input=%MDIR%nanosuit_sprite.obj output=%BUILDRES%nanosuit_sprite.momodel model_flip_uvs=true model_maximum=true
