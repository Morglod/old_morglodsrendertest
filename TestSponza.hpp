class TestSponza : public MR::SimpleApp {
public:
    float camera_moving_speed = 1.0f;

    double mouseX = 0.0, mouseY = 0.0;
    const float MOUSE_SPEED = 70.0f;

    bool Setup() {
        //Write user's machine info
        MR::Log::LogString(
            std::string("Machine info:") +
            std::string("\nVersion: ") + MR::MachineInfo::gl_version_string() +
            std::string("\nGLSL: ") + MR::MachineInfo::gl_version_glsl() +
            std::string("\nOpenGL: ") + std::to_string(MR::MachineInfo::gl_version_major()) + std::string(" ") + std::to_string(MR::MachineInfo::gl_version_minor()) +
            std::string("\nGPU: ") + MR::MachineInfo::gpu_name() + std::string(" from ") + MR::MachineInfo::gpu_vendor_string() +
            std::string("\nMem Total(kb): ") + std::to_string(MR::MachineInfo::total_memory_kb()) + std::string(" Current(kb): ") + std::to_string(MR::MachineInfo::current_memory_kb()) + "\n\n"
        , MR_LOG_LEVEL_INFO);

        MR::Log::LogString("\nVBUM: " + std::to_string(MR::MachineInfo::FeatureNV_GPUPTR()));
        MR::Log::LogString("Direct: " + std::to_string(MR::MachineInfo::IsDirectStateAccessSupported()));

        MR::Model* sponza_model = MR::ModelManager::Instance()->NeedModel("Data/Sponza.momodel");
        for(unsigned short meN = 0; meN < sponza_model->GetLodN(0)->GetMeshesNum(); ++meN){
            if(sponza_model->GetLodN(0)->GetMesh(meN)->GetMaterial() != nullptr){
                camera->AttachToShader( sponza_model->GetLodN(0)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetShader() );
            }
        }

        MR::Entity* sponza_entity = scene.CreateEntity(sponza_model);
        sponza_entity->GetTransformP()->SetScale( new glm::vec3(0.005f, 0.005f, 0.005f) );

        MR::Model* nano_model = MR::ModelManager::Instance()->NeedModel("Data/Nanosuit.momodel");
        for(unsigned short meN = 0; meN < nano_model->GetLodN(0)->GetMeshesNum(); ++meN){
            if(nano_model->GetLodN(0)->GetMesh(meN)->GetMaterial() != nullptr){
                camera->AttachToShader( nano_model->GetLodN(0)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetShader() );
            }
        }

        nano_model->AddLod( MR::ModelManager::Instance()->NeedModel("Data/nanosuit_sprite.momodel")->GetLodN(0) );
        for(unsigned short meN = 0; meN < nano_model->GetLodN(1)->GetMeshesNum(); ++meN){
            if(nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial() != nullptr){
                //nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->SetShader(shader_render_to_texture_discard);
                if(nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetDiffuseTexture() != nullptr){
                    if(nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetDiffuseTexture()->GetSettings() != nullptr){
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetDiffuseTexture()->GetSettings()->SetWrapR(MR::TextureSettings::Wrap::CLAMP);
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetDiffuseTexture()->GetSettings()->SetWrapS(MR::TextureSettings::Wrap::CLAMP);
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetDiffuseTexture()->GetSettings()->SetWrapT(MR::TextureSettings::Wrap::CLAMP);
                    }
                }
                if(nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAmbientTexture() != nullptr){
                    if(nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAmbientTexture()->GetSettings() != nullptr){
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAmbientTexture()->GetSettings()->SetWrapR(MR::TextureSettings::Wrap::CLAMP);
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAmbientTexture()->GetSettings()->SetWrapS(MR::TextureSettings::Wrap::CLAMP);
                        nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->GetAmbientTexture()->GetSettings()->SetWrapT(MR::TextureSettings::Wrap::CLAMP);
                    }
                }
                nano_model->GetLodN(1)->GetMesh(meN)->GetMaterial()->GetPass(0)->SetTwoSided(true);
            }
        }
        nano_model->SetDistStep(8.0f);

        MR::Entity* nano_entity = scene.CreateEntity(nano_model);
        nano_entity->GetTransformP()->SetScale( new glm::vec3(0.1f, 0.1f, 0.1f) );

        glClearColor(0.8f, 0.82f, 0.83f, 1.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

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

    inline void DrawScene(){
        scene.Draw(sys);
    }

    void Frame(const float& delta) {
        Input(delta);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        DrawScene();

        if(window->IsIconified()) {
            Sleep(1000);
        }
        else if(!window->IsFocused()) {
            Sleep(500);
        }
    }

    void Free() {  }

    TestSponza() : SimpleApp() {}
    virtual ~TestSponza() {}
};

