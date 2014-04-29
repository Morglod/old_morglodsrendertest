set "buildres=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\BuildedRes\"
set "resdir=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\Resources\"
set "outdata=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\Data\"

copy %buildres%Dragon.momodel %outdata%Dragon.momodel
copy %buildres%Nanosuit.momodel %outdata%Nanosuit.momodel
copy %buildres%nanosuit_sprite.momodel %outdata%nanosuit_sprite.momodel
copy %buildres%shader_default.moshader %outdata%shader_default.moshader
copy %buildres%shader_default_render_to_texture.moshader %outdata%shader_default_render_to_texture.moshader
copy %buildres%shader_default_render_to_texture_discard.moshader %outdata%shader_default_render_to_texture_discard.moshader
copy %buildres%shader_default_screen.moshader %outdata%shader_default_screen.moshader
copy %buildres%shader_parallax.moshader %outdata%shader_parallax.moshader
copy %buildres%Sibenik.momodel %outdata%Sibenik.momodel
copy %buildres%SkyMap.momodel %outdata%SkyMap.momodel
copy %buildres%NormalDispl.momodel %outdata%NormalDispl.momodel

copy %resdir%nano_sprite_forward.png %outdata%nano_sprite_forward.png
copy %resdir%nano_sprite_side.png %outdata%nano_sprite_side.png
copy %resdir%nano_sprite_top.png %outdata%nano_sprite_top.png
copy %resdir%tex\arm_dif.png %outdata%arm_dif.png
copy %resdir%tex\body_dif.png %outdata%body_dif.png
copy %resdir%tex\glass_dif.png %outdata%glass_dif.png
copy %resdir%tex\hand_dif.png %outdata%hand_dif.png
copy %resdir%tex\helmet_dif.png %outdata%helmet_dif.png
copy %resdir%tex\leg_dif.png %outdata%leg_dif.png

pause