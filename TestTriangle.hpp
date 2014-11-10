
#include <Buffers/Buffers.hpp>
#include <Utils/Serialization.hpp>
#include <Geometry/GeometryBuilder.hpp>
#include <Geometry/GeometryObject.hpp>
#include <Geometry/GeometryManager.hpp>

#include <Textures/TextureObjects.hpp>
#include <Textures/TextureSettings.hpp>

#include <Utils/Pointers.hpp>

#include <Geometry/GeometryLoader.hpp>

class TestTriangle : public MR::SimpleApp {
public:
    MR::IGeometry* geom;
    /*MR::GPUObjectHandlePtr<MR::IShaderProgram> prog;
    MR::GPUObjectHandlePtr<MR::ITexture> tex;*/
    MR::IShaderProgram* prog;
    MR::ITexture* tex;

    bool Setup() {
        MR::MachineInfo::PrintInfo();

        ///TEST
        MR::GeometryLoader geom_loader;
        geom_loader.Import("pyr_test.obj");
        geom = geom_loader.GetGeometry().At(0);
        ///

        prog = MR::ShaderProgram::DefaultWithTexture();

        if(prog == nullptr) return false;

        prog->CreateUniform(MR_SHADER_MVP_MAT4, MR::IShaderUniform::Mat4, camera->GetMVPPtr());
        prog->CreateUniform("MainTex", MR::IShaderUniform::Sampler2D, new int(0));

        /*MR::IMaterialPass* matPass = mat->CreatePass(MR::MaterialFlag::Default());
        matPass->SetShaderProgram(prog);*/

        camera->SetPosition(glm::vec3(0.25f, 0.75f, -0.7f));
        camera->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        camera->SetFarZ(10000.0f);

        //Make texture
        tex = MR::Texture::FromFile("grid_test_tex.png");

        MR::ITextureSettings* texSettings = new MR::TextureSettings();
        texSettings->Create();
        tex->SetSettings(texSettings);
        tex->Bind(0);

        glClearColor(0.2f, 0.2, 0.2, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);

        window = context.GetMainWindow();

        return true;
    }

    GLFWwindow* window;
    double mouse_x = 0.0, mouse_y = 0.0;
    unsigned short _frames_this_second = 0;
    double _current_time = 0.0;
    void Frame(const float& delta) {
        //Camera moves
        const float move_speed = 8.0f;
        const float mouse_speed = 70.0f;

        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        if(glfwGetKey(window, GLFW_KEY_W)) {
            camera->MoveForward(move_speed * delta);
        }
        if(glfwGetKey(window, GLFW_KEY_S))  {
            camera->MoveForward(-move_speed * delta);
        }
        if(glfwGetKey(window, GLFW_KEY_A))  {
            camera->MoveLeft(move_speed * delta);
        }
        if(glfwGetKey(window, GLFW_KEY_D))  {
            camera->MoveLeft(-move_speed * delta);
        }
        if(glfwGetKey(window, GLFW_KEY_SPACE)) {
            camera->MoveUp(move_speed * delta);
        }
        if(glfwGetKey(window, GLFW_KEY_C)) {
            camera->MoveUp(-move_speed * delta);
        }
        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)) {
            glfwSetCursorPos(window, (double)SCREEN_CENTER_X, (double)SCREEN_CENTER_Y);

            camera->Roll((SCREEN_CENTER_X - (int)mouse_x) * mouse_speed * delta);
            camera->Yaw((SCREEN_CENTER_Y - (int)mouse_y) * mouse_speed * delta);
        }

        //Draw
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        prog->Use();
        geom->Draw();

        _current_time += delta;
        ++_frames_this_second;
        if(_current_time >= 1.0) {
            printf(("\r"+std::to_string(_frames_this_second)).c_str());
            _current_time = 0.0;
            _frames_this_second = 0;
        }
    }

    void Free() {
        /*delete geom[0];
        delete geom[1];
        delete geom[2];
        delete geom[3];
        delete geom[4];
        //delete mat;
        delete mesh;
        prog->Destroy();
        delete prog;*/
    }

    TestTriangle() : MR::SimpleApp() {}
    virtual ~TestTriangle() {  }
};

