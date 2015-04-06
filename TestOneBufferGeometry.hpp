using namespace mr;

class TestOneBufGeom : public SimpleApp {
public:
    float camera_moving_speed = 1.0f;

    double mouseX = 0.0, mouseY = 0.0;
    const float MOUSE_SPEED = 70.0f;

    Geometry *geom1, *geom2;
    VertexFormatCustomFixed* vformat;
    IndexFormatCustom* iformat;

    void LoadData1() {
        const unsigned int num_verts = 3;
        const unsigned int size_of_one_vertex = sizeof(float) * 6;
        const unsigned int size_in_bytes = size_of_one_vertex * num_verts;

        float vdata[] {
            0.0f, 1.0f, 0.0f, //p
            0.0f, 0.0f, 0.0f, //t
            1.0f, 0.0f, 0.0f, //p
            0.0f, 0.0f, 0.0f, //t
            -1.0f, 0.0f, 0.0f, //p
            0.0f, 0.0f, 0.0f //t
        };

        unsigned int idata [] {
            0, 1, 2
        };

        unsigned int vbuf_offset = 0, ibuf_offset = 0;
        IGeometryBuffer* gbuffer = GeometryManager::Instance()->PlaceGeometry(vformat, iformat, &vdata[0], size_in_bytes, &idata[0], sizeof(unsigned int) * 3, GL_STATIC_DRAW, GL_READ_ONLY, GL_TRIANGLES, &vbuf_offset, &ibuf_offset);
        geom1 = new Geometry(gbuffer, ibuf_offset / sizeof(unsigned int), vbuf_offset / size_of_one_vertex, 3, num_verts);
    }

    void LoadData2() {
        const unsigned int num_verts = 3;
        const unsigned int size_of_one_vertex = sizeof(float) * 6;
        const unsigned int size_in_bytes = size_of_one_vertex * num_verts;

        float vdata[] {
            0.0f, -1.0f, 0.0f, //p
            0.0f, 0.0f, 0.0f, //t
            1.0f, 0.0f, 0.0f, //p
            0.0f, 0.0f, 0.0f, //t
            -1.0f, 0.0f, 0.0f, //p
            0.0f, 0.0f, 0.0f //t
        };

        unsigned int idata [] {
            0, 1, 2
        };

        unsigned int vbuf_offset = 0, ibuf_offset = 0;
        IGeometryBuffer* gbuffer = GeometryManager::Instance()->PlaceGeometry(vformat, iformat, &vdata[0], size_in_bytes, &idata[0], sizeof(unsigned int) * 3, GL_STATIC_DRAW, GL_READ_ONLY, GL_TRIANGLES, &vbuf_offset, &ibuf_offset);
        geom2 = new Geometry(gbuffer, ibuf_offset / sizeof(unsigned int), vbuf_offset / size_of_one_vertex, 3, num_verts);
    }

    bool Setup() {
        //Write user's machine info
        MachineInfo::PrintInfo();

        TextureManager::Instance()->SetCompressionMode(ITexture::CompressionMode::ETC2);

        ///TIMER
        Timer<Time::HighResolutionClock, Time::Milliseconds> loading_timer;
        loading_timer.Start();

        //Load
        vformat = new VertexFormatCustomFixed();
        vformat->SetAttributesNum(2);
        vformat->AddVertexAttribute(new VertexAttributePos3F());
        vformat->AddVertexAttribute(new VertexAttributeCustom(3, VertexDataTypeFloat::Instance(), 1));
        vformat->Complete();

        iformat = new IndexFormatCustom(VertexDataTypeUInt::Instance());

        LoadData1();
        LoadData2();

        ///TIMER
        loading_timer.Stop();
        std::cout << "\n\nLoading time: " << loading_timer.TimerTime().count() << std::endl;

        glEnable(GL_DEPTH_TEST);
        //glEnable(GL_CULL_FACE);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

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

    void Frame(const float& delta) {
        Input(delta);
        pipeline->Frame(delta);

        geom1->Draw(sys);
        geom2->Draw(sys);

        if(window->IsIconified()) {
            Sleep(1000);
        }
        else if(!window->IsFocused()) {
            Sleep(500);
        }
    }

    void Free() {  }

    TestOneBufGeom() : SimpleApp() {}
    virtual ~TestOneBufGeom() {}
};
