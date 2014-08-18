class TestTriangle : public MR::SimpleApp {
public:
    MR::IGeometry* geom;
    MR::IShaderProgram* prog;
    MR::Material* mat;
    MR::IMesh* mesh;

    bool Setup() {
        MR::MachineInfo::PrintInfo();

        mat = new MR::Material();
        geom = MR::Geometry::MakeBox(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0,0,0.0f));
        mesh = new MR::Mesh(MR::StaticArray<MR::IGeometry*>(&geom, 1), mat);

        MR::ShaderBuilder::Params reqParams;
        reqParams.features.light = false;
        reqParams.customFragmentFuncName = "CustomFragment";
        reqParams.customFragmentCode =
        "vec4 CustomFragment(in vec4 a) {"
        "   vec3 vpos = GetLocalPos().xyz;"
        "   return vec4(normalize(vec3(floor(vpos.x*10.0f), floor(vpos.y*10.0f), floor(vpos.z*10.0f))), 1.0f);"
        "}";

        prog = MR::ShaderBuilder::Need(reqParams);
        prog->CreateUniform(MR_SHADER_MVP_MAT4, MR::IShaderUniform::SUT_Mat4, camera->GetMVPPtr());

        MR::IMaterialPass* matPass = mat->CreatePass(MR::MaterialFlag::Default());
        matPass->SetShaderProgram(prog);

        camera->SetPosition(glm::vec3(-0.5f, -0.5f, -0.5f));
        camera->SetRotation(glm::vec3(45.0f, 30.0f, 0.0f));

        glClearColor(0.2f, 0.2, 0.2, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);

        return true;
    }

    void Frame(const float& delta) {
        //Camera moves
        const float move_speed = 1.0f;
        const float mouse_speed = 70.0f;
        static double mouse_x = 0.0, mouse_y = 0.0;
        glfwGetCursorPos(context.GetMainWindow(), &mouse_x, &mouse_y);

        if(glfwGetKey(context.GetMainWindow(), GLFW_KEY_W)) {
            camera->MoveForward(move_speed*delta);
        }
        if(glfwGetKey(context.GetMainWindow(), GLFW_KEY_S))  {
            camera->MoveForward(-move_speed*delta);
        }
        if(glfwGetKey(context.GetMainWindow(), GLFW_KEY_A))  {
            camera->MoveLeft(move_speed*delta);
        }
        if(glfwGetKey(context.GetMainWindow(), GLFW_KEY_D))  {
            camera->MoveLeft(-move_speed*delta);
        }
        if(glfwGetKey(context.GetMainWindow(), GLFW_KEY_SPACE)) {
            camera->MoveUp(move_speed*delta);
        }
        if(glfwGetKey(context.GetMainWindow(), GLFW_KEY_C)) {
            camera->MoveUp(-move_speed*delta);
        }
        if(glfwGetMouseButton(context.GetMainWindow(), GLFW_MOUSE_BUTTON_RIGHT)) {
            glfwSetCursorPos(context.GetMainWindow(), (double)SCREEN_CENTER_X, (double)SCREEN_CENTER_Y);

            camera->Roll((SCREEN_CENTER_X - (int)mouse_x) * delta * mouse_speed);
            camera->Yaw((SCREEN_CENTER_Y - (int)mouse_y) * delta * mouse_speed);
        }

        //Draw
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        mesh->Draw();
    }

    void Free() {
        delete geom;
        delete mat;
        delete mesh;
        delete prog;
    }

    TestTriangle() : MR::SimpleApp() {}
    virtual ~TestTriangle() {}
};

