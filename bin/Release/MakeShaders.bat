cd "C:\Users\VolodyaM\Documents\SharpDevelop Projects\MoConverter\bin\Debug\"
set "SHDIR=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\Resources\Shaders\"
set "BUILDRES=C:\Users\VolodyaM\Documents\CodeBlocksProjects\MorglodsRenderTest\bin\Release\BuildedRes\"
MoConverter.exe type=shader shader_sub_vertex=%SHDIR%shader_default_vert.txt shader_sub_fragment=%SHDIR%shader_default_frag.txt output=%BUILDRES%shader_default.moshader
MoConverter.exe type=shader shader_sub_vertex=%SHDIR%shader_default_render_to_texture_vert.txt shader_sub_fragment=%SHDIR%shader_default_render_to_texture_frag.txt output=%BUILDRES%shader_default_render_to_texture.moshader
MoConverter.exe type=shader shader_sub_vertex=%SHDIR%shader_default_screen_vert.txt shader_sub_fragment=%SHDIR%shader_default_screen_frag.txt output=%BUILDRES%shader_default_screen.moshader
MoConverter.exe type=shader shader_sub_vertex=%SHDIR%shader_default_render_to_texture_vert_discard.txt shader_sub_fragment=%SHDIR%shader_default_render_to_texture_frag_discard.txt output=%BUILDRES%shader_default_render_to_texture_discard.moshader
MoConverter.exe type=shader shader_sub_vertex=%SHDIR%shader_parallax_vs.txt shader_sub_fragment=%SHDIR%shader_parallax_fs.txt output=%BUILDRES%shader_parallax.moshader
