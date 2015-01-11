#include "TimedCounter.hpp"

#include <Buffers/Buffers.hpp>
#include <Utils/Serialization.hpp>
#include <Geometry/GeometryBuilder.hpp>
#include <Geometry/GeometryObject.hpp>
#include <Geometry/GeometryManager.hpp>

#include <Models/Models.hpp>
#include <Scene/Node.hpp>
#include <Geometry/Mesh.hpp>
#include <Scene/SceneManager.hpp>
#include <Scene/Entity.hpp>

#include <Textures/TextureObjects.hpp>
#include <Textures/TextureSettings.hpp>

#include <Utils/Pointers.hpp>

#include <Scene/SceneLoader.hpp>

class TestTriangle : public mr::SimpleApp {
public:
    mr::IGeometry* geom;
    /*MR::GPUObjectHandlePtr<MR::IShaderProgram> prog;
    MR::GPUObjectHandlePtr<MR::ITexture> tex;*/
    mr::IShaderProgram* prog;
    mr::ITexture* tex;
    mr::ModelPtr model;
    mr::MeshPtr mesh;
    mr::SceneManager sceneManager;

    bool Setup() {
        mr::machineInfo::PrintInfo();

        ///TEST
        std::string loadModelSrc = "";
        std::cout << "Load model: ";
        std::cin >> loadModelSrc;

        std::string loadTexSrc = "";
        std::cout << "Load tex: ";
        std::cin >> loadTexSrc;

        bool loadFast = false;
        std::cout << "Load fast? (0 or 1): ";
        std::cin >> loadFast;

        mr::SceneLoader scene_loader;
        scene_loader.Import(loadModelSrc, loadFast);
        geom = scene_loader.GetGeometry().At(0);
        ///

        prog = mr::ShaderProgram::DefaultWithTexture();

        if(prog == nullptr) return false;

        prog->CreateUniform(MR_SHADER_MVP_MAT4, mr::IShaderUniform::Mat4, camera->GetMVPPtr());
        prog->CreateUniform(MR_SHADER_COLOR_TEX, mr::IShaderUniform::Sampler2D, new int(0));

        camera->SetPosition(glm::vec3(0.25f, 0.75f, -0.7f));
        camera->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        camera->SetFarZ(10000.0f);

        //Make texture
        tex = mr::Texture::FromFile(loadTexSrc);

        if(tex) {
            mr::ITextureSettings* texSettings = new mr::TextureSettings();
            texSettings->Create();
            tex->SetSettings(texSettings);
            tex->Bind(0);
        }

        model = mr::ModelPtr(new mr::Model());
        mr::SubModel* subModel = new mr::SubModel();

        mesh = mr::MeshPtr(mr::Mesh::Create(
            mr::TStaticArray<mr::IGeometry*> {
                geom
            },
            nullptr
        ));

        subModel->SetMeshes(
            mr::TStaticArray<mr::MeshWeakPtr> {
                mr::MeshWeakPtr(mesh)
            }
        );

        model->SetLods(
            mr::TStaticArray<mr::SubModelPtr> {
                mr::SubModelPtr(subModel)
            }
        );

        mr::EntityPtr entity = sceneManager.CreateEntity(model);
        sceneManager.GetRootNode()->CreateChild()->AddChild(std::static_pointer_cast<mr::SceneNode>(entity));

        glClearColor(0.2f, 0.2, 0.2, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);

        window = context->GetWindow();

        return true;
    }

    GLFWwindow* window;
    double mouse_x = 0.0, mouse_y = 0.0;

    class FpsCounter : public mu::TimedCounter {
    public:
        void Invoke() override {
            printf(("\r"+std::to_string(counter)+" ").c_str());
        }
    };

    FpsCounter fps;

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

        sceneManager.Draw();

        fps.Count(delta);
    }

    void Free() {
    }

    TestTriangle() : mr::SimpleApp() {}
    virtual ~TestTriangle() {  }
};

