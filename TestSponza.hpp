class TestSponza : public MR::SimpleApp {
public:
    float camera_moving_speed = 1.0f;

    double mouseX = 0.0, mouseY = 0.0;
    const float MOUSE_SPEED = 70.0f;

    MR::ILightSource* light_point = 0;

    MR::Query qTimeElapsed = MR::Query(MR::Query::Target::TimeElapsed);

    bool Setup() {
        //Write user's machine info
        MR::MachineInfo::PrintInfo();

        MR::TextureManager::GetInstance()->SetCompressionMode(MR::ITexture::CompressionMode::ETC2);

        ///TIMER
        qTimeElapsed.Begin();
        MR::Timer<MR::Time::HighResolutionClock, MR::Time::Milliseconds> loading_timer;
        loading_timer.Start();

        MR::Model* sponza_model = MR::ModelManager::GetInstance()->NeedModel("Data/Sponza.momodel");

        MR::Entity* sponza_entity = scene.CreateEntity(sponza_model);
        sponza_entity->GetTransformPtr()->SetScale( new glm::vec3(0.005f, 0.005f, 0.005f) );

        /*for(int i = 0; i < 5; ++i){
            MR::Entity* entI = scene.CreateEntity(sponza_model);
            entI->GetTransformPtr()->SetScale( new glm::vec3(0.005f, 0.005f, 0.005f) );
            entI->GetTransformPtr()->SetPos(glm::vec3(0.0f, 0.0f, 15.0f*i));
        }*/

        ///TIMER
        loading_timer.Stop();
        std::cout << "\n\nLoading time: " << loading_timer.TimerTime().count() << std::endl;
        qTimeElapsed.End();

        uint64_t gpu_time = 0;
        bool gpu_timer = qTimeElapsed.GetResult(&gpu_time);
        MR::Log::LogString("\rTime elapsed: " + std::to_string(gpu_time));

        light_point = MR::LightSource::CreatePointLight(glm::vec3(-3.0f,0.0f,0.0f), glm::vec3(2.1f,2.1f,2.1f), glm::vec3(0.0f, 0.0f, 0.0f), 1.0f, 3.0f, 10.0f);
        scene.AddLight( light_point );

        for(size_t il = 0; il < 3; ++il) {
            scene.AddLight( MR::LightSource::CreatePointLight(glm::vec3(0.0f + 3.0f * (float)il,0.0f,0.0f), glm::vec3(2.1f,2.1f,2.1f), glm::vec3(0.0f, 0.0f, 0.0f), 10.0f / (il+0.1f), (float)il +0.001f, 10.0f) );
        }

        /*scene.AddLight(
            MR::LightSource::CreateDirLight(glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f)), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f))
        );*/

        scene.SetFog(0.5f, 0.9f, glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

        //glClearColor(0.8f, 0.82f, 0.83f, 1.0f);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        return true;
    }

    void Input(const float& delta){
        /*glfwGetMousePo
        context.->GetMousePos(&mouseX, &mouseY);

        if(glfwGetKey(window->GetHandle(), GLFW_KEY_SPACE)) camera_moving_speed = 15.0f;
        else camera_moving_speed = 1.0f;

        if(glfwGetKey(window->GetHandle(), GLFW_KEY_W)) camera->MoveForward( delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_S)) camera->MoveForward( -delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_A)) camera->MoveLeft( delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_D)) camera->MoveLeft( -delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_V)) camera->Move( delta*glm::vec3(0.0f,camera_moving_speed,0.0f) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_C)) camera->Move( -delta*glm::vec3(0.0f,camera_moving_speed,0.0f) );

        if(glfwGetKey(window->GetHandle(), GLFW_KEY_LEFT)) light_point->GetPosP()->x += delta*1.0f; //camera->MoveForward( delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_RIGHT)) light_point->GetPosP()->x -= delta*1.0f; //camera->MoveForward( -delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_UP)) light_point->GetPosP()->z += delta*1.0f; //camera->MoveLeft( delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );
        if(glfwGetKey(window->GetHandle(), GLFW_KEY_DOWN)) light_point->GetPosP()->z -= delta*1.0f; //camera->MoveLeft( -delta*glm::vec3(camera_moving_speed,camera_moving_speed,camera_moving_speed) );

        if(glfwGetMouseButton(window->GetHandle(), GLFW_MOUSE_BUTTON_RIGHT)) {
            window->SetMousePos(SCREEN_CENTER_X, SCREEN_CENTER_Y);

            camera->Roll(((float)(SCREEN_CENTER_X - mouseX))*delta*MOUSE_SPEED);
            camera->Yaw(((float)(SCREEN_CENTER_Y - mouseY))*delta*MOUSE_SPEED);
        }*/
    }

    void Frame(const float& delta) {
        glClear(GL_COLOR_BUFFER_BIT);

        Input(delta);
        pipeline->Frame(delta);

        /*if(window->IsIconified()) {
            Sleep(1000);
        }
        else if(!window->IsFocused()) {
            Sleep(500);
        }*/
    }

    void Free() {  }

    TestSponza() : SimpleApp() {}
    virtual ~TestSponza() {}
};

/*
DEPRECATED PARTS

class NanoModelLoaded : public MR::EventHandle<MR::Resource*> {
        public:
            class NanoSpriteModelLoaded : public MR::EventHandle<MR::Resource*> {
            public:
                MR::Model* nano_model;
                MR::Model* sprite_model;

                void Invoke(MR::EventListener<MR::Resource*>*, MR::Resource* res) override {
                    nano_model->AddLod( sprite_model->GetLodN(0) );
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
                }

                NanoSpriteModelLoaded(MR::Model* nm, MR::Model* sm) : nano_model(nm), sprite_model(sm) {}
            };

            MR::Model* nano_model;
            MR::Model* sprite_model;

            void Invoke(MR::EventListener<MR::Resource*>*, MR::Resource* res) override {
                sprite_model = MR::ModelManager::Instance()->NeedModel("Data/nanosuit_sprite.momodel");
                sprite_model->OnLoad.RegisterHandle(new NanoSpriteModelLoaded(nano_model, sprite_model));
            }
            NanoModelLoaded(MR::Model* nm) : nano_model(nm) {}
        };

        MR::Model* nano_model = MR::ModelManager::Instance()->NeedModel("Data/Nanosuit.momodel");
        nano_model->OnLoad.RegisterHandle(new NanoModelLoaded(nano_model));

        MR::Entity* nano_entity = scene.CreateEntity(nano_model);
        nano_entity->GetTransformP()->SetScale( new glm::vec3(0.1f, 0.1f, 0.1f) );
**/
