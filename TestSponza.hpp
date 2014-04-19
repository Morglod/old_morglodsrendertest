MR::GeometryBuffer* createScreenQuad(){
    return MR::GeometryBuffer::Plane(glm::vec3(2.0f, 2.0f, 0.0f), glm::vec3(0,0,0), MR::IGLBuffer::Static+MR::IGLBuffer::Draw, MR::IGeometryBuffer::Draw_Quads);
}

class TestSponza : public MR::SimpleApp {
public:
    float camera_moving_speed = 1.0f;

    MR::Shader* shader_default;
    MR::Shader* shader_render_to_texture;
    MR::Shader* shader_render_to_texture_discard;
    MR::Shader* shader_default_screen;

    MR::RenderTarget* rtarget;

    MR::CubeMap* env_cubemap;

    double mouseX = 0.0, mouseY = 0.0;
    const float MOUSE_SPEED = 70.0f;

    MR::IGeometry* screen_quad;
    MR::GeometryBuffer* screen_quad_buffer;

    MR::Shader* MakeShader(MR::SubShader* s1, MR::SubShader* s2){
        MR::Shader* shader = dynamic_cast<MR::Shader*>(MR::ShaderManager::Instance()->Create("Auto", "FromSubs"));
        shader->AttachSubShader(s1);
        shader->AttachSubShader(s2);
        shader->Link();
        return shader;
    }

    bool Setup() {
        //Write user's machine info
        MR::Log::LogString( std::string("Machine info:") +
            std::string("\nVersion: ") + MR::MachineInfo::gl_version_string() +
            std::string("\nGLSL: ") + MR::MachineInfo::gl_version_glsl() +
            std::string("\nOpenGL: ") + std::to_string(MR::MachineInfo::gl_version_major()) + std::string(" ") + std::to_string(MR::MachineInfo::gl_version_minor()) +
            std::string("\nGPU: ") + MR::MachineInfo::gpu_name() + std::string(" from ") + MR::MachineInfo::gpu_vendor_string() +
            std::string("\nMem Total(kb): ") + std::to_string(MR::MachineInfo::total_memory_kb()) + std::string(" Current(kb): ") + std::to_string(MR::MachineInfo::current_memory_kb()) + "\n\n", MR_LOG_LEVEL_INFO);

        MR::Log::LogString("\nVBUM: " + std::to_string(MR::MachineInfo::FeatureNV_GPUPTR()));
        MR::Log::LogString("Direct: " + std::to_string(MR::MachineInfo::IsDirectStateAccessSupported()));

        shader_default = MakeShader(MR::SubShader::DefaultFrag(), MR::SubShader::DefaultVert());
        shader_render_to_texture = MakeShader(MR::SubShader::DefaultRTTFrag(), MR::SubShader::DefaultRTTVert());
        shader_render_to_texture_discard = MakeShader(MR::SubShader::DefaultRTTDiscardFrag(), MR::SubShader::DefaultRTTDiscardVert());
        shader_default_screen = MakeShader(MR::SubShader::DefaultScreenFrag(), MR::SubShader::DefaultScreenVert());

        shader_default->CreateUniform(MR_MATERIAL_ALBEDO_TEX, new int(0));
        CameraUniforms(shader_default, "projMatrix", "viewMatrix", "mvp", "", "");

        shader_render_to_texture->CreateUniform(MR_MATERIAL_ALBEDO_TEX, new int(0));
        //shader_render_to_texture->CreateUniform("EnvCubemap", new int(10));
        shader_render_to_texture->CreateUniform(MR_MATERIAL_COLOR_V4, new glm::vec4(1,1,1,1));
        shader_render_to_texture->CreateUniform("screenColor", new int(1));
        shader_render_to_texture->CreateUniform("screenDepth", new int(2));
        CameraUniforms(shader_render_to_texture, "projMatrix", "viewMatrix", "mvp", "", "");

        shader_render_to_texture_discard->CreateUniform(MR_MATERIAL_ALBEDO_TEX, new int(0));
        shader_render_to_texture_discard->CreateUniform(MR_MATERIAL_COLOR_V4, new glm::vec4(1,1,1,1));
        shader_render_to_texture_discard->CreateUniform("colorTexture", new int(1));
        shader_render_to_texture_discard->CreateUniform("depthTexture", new int(2));
        CameraUniforms(shader_render_to_texture_discard, "projMatrix", "viewMatrix", "mvp", "", "");

        shader_default_screen->CreateUniform(MR_MATERIAL_ALBEDO_TEX, new int(1));

        rtarget = new MR::RenderTarget("TestTarget", 1, WINDOW_WIDTH, WINDOW_HEIGHT);
        rtarget->CreateTargetTexture(0, MR::Texture::InternalFormat::RGB, MR::Texture::Format::RGB, MR::Texture::Type::UNSIGNED_BYTE);

        //env_cubemap = new MR::CubeMap(MR::TextureManager::Instance(), "EnvCubemap", "FromMem", 512, 512);

        MR::Model* sponza_model = MR::ModelManager::Instance()->NeedModel("Data/Sponza.momodel");
        for(unsigned short meN = 0; meN < sponza_model->GetLodN(0)->GetMeshesNum(); ++meN){
            if(sponza_model->GetLodN(0)->GetMesh(meN)->GetMaterial() != nullptr){
                sponza_model->GetLodN(0)->GetMesh(meN)->GetMaterial()->GetPass(0)->SetShader(shader_render_to_texture);
            }
        }

        MR::Entity* sponza_entity = scene.CreateEntity(sponza_model);
        sponza_entity->GetTransformP()->SetScale( new glm::vec3(0.005f, 0.005f, 0.005f) );

        MR::Model* nano_model = MR::ModelManager::Instance()->NeedModel("Data/Nanosuit.momodel");
        for(unsigned short meN = 0; meN < nano_model->GetLodN(0)->GetMeshesNum(); ++meN){
            if(nano_model->GetLodN(0)->GetMesh(meN)->GetMaterial() != nullptr){
                nano_model->GetLodN(0)->GetMesh(meN)->GetMaterial()->GetPass(0)->SetShader(shader_render_to_texture);
            }
        }

        nano_model->AddLod( MR::ModelManager::Instance()->NeedModel("Data/nanosuit_sprite.momodel")->GetLodN(0) );
        for(unsigned short meN = 0; meN < nano_model->GetLodN(1)->GetMeshesNum(); ++meN){
            if(nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial() != nullptr){
                nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->SetShader(shader_render_to_texture_discard);
                if(nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAlbedoTexture() != nullptr){
                    if(nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAlbedoTexture()->GetSettings() != nullptr){
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAlbedoTexture()->GetSettings()->SetWrapR(MR::TextureSettings::Wrap::CLAMP);
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAlbedoTexture()->GetSettings()->SetWrapS(MR::TextureSettings::Wrap::CLAMP);
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAlbedoTexture()->GetSettings()->SetWrapT(MR::TextureSettings::Wrap::CLAMP);
                    }
                }
                nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->SetTwoSided(true);
            }
        }
        nano_model->SetDistStep(8.0f);

        MR::Entity* nano_entity = scene.CreateEntity(nano_model);
        nano_entity->GetTransformP()->SetScale( new glm::vec3(0.1f, 0.1f, 0.1f) );

        screen_quad_buffer = createScreenQuad();
        screen_quad = new MR::Geometry(screen_quad_buffer, 0, 4, 4);

        glClearColor(0.8f, 0.82f, 0.83f, 1.0f);

        glEnable(GL_DEPTH_TEST);

        return true;
    }

    void Input(const float& delta){
        window->GetMousePos(&mouseX, &mouseY);

        if(glfwGetKey(window->GetHandle(), GLFW_KEY_SPACE)) camera_moving_speed = 15.0f;
        else camera_moving_speed = 1.0f;

        if(glfwGetKey(window->GetHandle(), GLFW_KEY_W)) camera->MoveForward( delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_S)) camera->MoveForward( -delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_A)) camera->MoveLeft( delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_D)) camera->MoveLeft( -delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_V)) camera->Move( delta*glm::vec3(0.0f,camera_moving_speed,0.0f) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_C)) camera->Move( -delta*glm::vec3(0.0f,camera_moving_speed,0.0f) );

        if(glfwGetMouseButton(window->GetHandle(), GLFW_MOUSE_BUTTON_RIGHT)) {
            window->SetMousePos(SCREEN_CENTER_X, SCREEN_CENTER_Y);

            camera->Roll(((float)(SCREEN_CENTER_X - mouseX))*delta*MOUSE_SPEED);
            camera->Yaw(((float)(SCREEN_CENTER_Y - mouseY))*delta*MOUSE_SPEED);
        }
    }

    /*void DrawToCubeMap(){
        context->BindTexture(MR::Texture::Target::CubeMap, env_cubemap->GetGLTexture(), 1);
        env_cubemap->GetRenderTarget()->Bind(context);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        DrawScene();

        env_cubemap->GetRenderTarget()->Unbind();
        window->ResetViewport(context);
    }*/

    void DrawToRenderTarget(){
        //Bind RenderTarget ColorBuffer as TextureUnit 1
        context->BindTexture((unsigned int)MR::ITexture::Target::Base2D, rtarget->GetTargetTexture(0), 1);

        //Bind RenderTarget DepthBuffer as TextureUnit 2
        context->BindTexture((unsigned int)MR::ITexture::Target::Base2D, rtarget->GetTargetTexture(1), 2);

        //Bind RenderTarget and clear it
        context->BindRenderTarget(rtarget);

        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        DrawScene();

        //Return to normal state
        context->UnBindRenderTarget();
        window->ResetViewport(context);
    }

    void DrawFromRenderTarget(const float& delta){
        glClear(GL_DEPTH_BUFFER_BIT);

        glDisable(GL_CULL_FACE);
        context->UseShader(shader_default_screen);
        context->DrawGeometry(screen_quad);

        MR::UIManager::Instance()->Draw(context, delta);
        glEnable(GL_CULL_FACE);
    }

    void DrawScene(){
        scene.Draw(context);
    }

    void Frame(const float& delta) {
        Input(delta);

        //DrawToCubeMap();
        DrawToRenderTarget();
        DrawFromRenderTarget(delta);

        if(window->IsIconified()) {
            Sleep(1000);
        }
        else if(!window->IsFocused()) {
            Sleep(500);
        }
    }

    void Free() {
        delete screen_quad;
        delete screen_quad_buffer;
        delete rtarget;
        //delete env_cubemap;
    }

    TestSponza() : SimpleApp() {}
    virtual ~TestSponza() {}
};
