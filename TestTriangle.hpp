#include "TimedCounter.hpp"

#include <Buffers/Buffers.hpp>
#include <Utils/Serialization.hpp>
#include <Geometry/GeometryBuilder.hpp>
#include <Geometry/GeometryObject.hpp>
#include <Geometry/GeometryFormats.hpp>
#include <Geometry/GeometryManager.hpp>

#include <Models/Models.hpp>
#include <Scene/Node.hpp>
#include <Geometry/Mesh.hpp>
#include <Scene/SceneManager.hpp>
#include <Scene/Entity.hpp>

#include <Textures/TextureObjects.hpp>
#include <Textures/TextureSettings.hpp>

#include <RTT/FrameBufferObjects.hpp>
#include <RTT/RenderBufferObject.hpp>

#include <StateCache.hpp>

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
    std::vector<mr::MeshPtr> meshes;
    mr::SceneManager sceneManager;
    mr::IVertexAttribute* instAttrib;
    mr::IGPUBuffer* instGpuBuff;
   // mr::IFrameBuffer* frameBuffer;

    bool Setup() {
        mr::machine::PrintInfo();

        //Create and set framebuffer
        /*mr::IRenderBuffer * renderBuffer = new mr::RenderBuffer();
        renderBuffer->Create(mr::ITexture::StorageDataFormat::SDF_RGB8, 800, 600, 0);

        frameBuffer = new mr::FrameBuffer();
        frameBuffer->Create();
        frameBuffer->Bind(mr::IFrameBuffer::BindTarget::DrawFramebuffer);
        frameBuffer->SetRenderBufferToColor(renderBuffer, 0);
        std::cout << mr::FrameBuffer::CompletionStatusToString(frameBuffer->CheckCompletion(mr::IFrameBuffer::BindTarget::DrawFramebuffer));*/

        //Load shader
        prog = mr::ShaderProgram::DefaultWithTexture();

        if(prog == nullptr) return false;

        prog->CreateUniform(MR_SHADER_MVP_MAT4, mr::IShaderUniform::Mat4, camera->GetMVPPtr());
        prog->CreateUniform(MR_SHADER_COLOR_TEX, mr::IShaderUniform::Sampler2D, new int(0));

        ///TEST
        std::string loadModelSrc = "";
        std::string loadTexSrc = "";
        bool loadFast = false;
        unsigned int inst_num = 100;

#ifndef DEBUG_BUILD
        std::cout << "Load model: ";
        std::cin >> loadModelSrc;

        std::cout << "Load tex: ";
        std::cin >> loadTexSrc;

        std::cout << "Load fast? (0 or 1): ";
        std::cin >> loadFast;

        std::cout << "Instances num: ";
        std::cin >> inst_num;
#else
        loadModelSrc = "pyr_test.obj";
        loadTexSrc = "mramor6x6.png";
        loadFast = false;
        inst_num = 1;
#endif

        mr::SceneLoader scene_loader;
        scene_loader.Import(loadModelSrc, loadFast);
        geom = scene_loader.GetGeometry().At(0);
        auto sceneMaterials = scene_loader.GetMaterials();
        for(size_t i = 0; i < sceneMaterials.GetNum(); ++i) {
            sceneMaterials.At(i)->SetShaderProgram(prog);
        }

        //Instancing
        instGpuBuff = new mr::GPUBuffer();
        instGpuBuff->Allocate(mr::IGPUBuffer::Static, sizeof(glm::vec3) * inst_num);

        mr::IGPUBuffer::IMappedRangePtr mappedInstGpuBuff = instGpuBuff->Map(0, sizeof(glm::vec3) * inst_num, mr::IGPUBuffer::IMappedRange::Write | mr::IGPUBuffer::IMappedRange::Unsynchronized | mr::IGPUBuffer::IMappedRange::Invalidate);
        if(mappedInstGpuBuff == nullptr) {
            glm::vec3 instPos[inst_num];
            for(unsigned int i = 0; i < inst_num; ++i) {
                instPos[i] = glm::vec3(50.0f * i, 0.0f, 0.0f);
            }
            instGpuBuff->Write(instPos, 0, 0, sizeof(glm::vec3) * inst_num, nullptr, nullptr);
        } else {
            for(unsigned int i = 0; i < inst_num; ++i) {
                glm::vec3* instPos = (glm::vec3*)(mappedInstGpuBuff->Get());
                instPos[i] = glm::vec3(50.0f * i, 0.0f, 0.0f);
            }
            mappedInstGpuBuff->UnMap();
        }

        instAttrib = new mr::VertexAttributeCustom(3, &mr::VertexDataTypeFloat::GetInstance(), 4, 1);
        geom->GetGeometryBuffer()->SetAttribute(instAttrib, instGpuBuff);
        geom->GetDrawParams()->SetInstancesNum(inst_num);
        ///

        camera->SetPosition(glm::vec3(0.25f, 0.75f, -0.7f));
        camera->SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
        camera->SetFarZ(10000.0f);

        //Make texture
        tex = mr::Texture::FromFile(loadTexSrc);

        if(tex) {
            mr::StateCache* stateCache = mr::StateCache::GetDefault();
            mr::ITextureSettings* texSettings = new mr::TextureSettings();
            texSettings->Create();
            tex->SetSettings(texSettings);
            stateCache->BindTexture(tex, 0);
        }

        model = mr::ModelPtr(new mr::Model());
        mr::SubModel* subModel = new mr::SubModel();

        auto sceneMeshes = scene_loader.GetMeshes();
        mr::TStaticArray<mr::MeshWeakPtr> subModelMeshes(sceneMeshes.GetNum());
        for(size_t i = 0; i < sceneMeshes.GetNum(); ++i) {
            meshes.push_back(mr::MeshPtr(sceneMeshes.At(i)));
            subModelMeshes.At(i) = mr::MeshWeakPtr(meshes[meshes.size()-1]);
        }

        subModel->SetMeshes( subModelMeshes );

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
        //const float mouse_speed = 0.0001f;
        const float mouse_speed = 8.0f;

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

            camera->Roll((float)(SCREEN_CENTER_X - (int)mouse_x) * mouse_speed * delta /*/ delta*/);
            camera->Yaw((float)(SCREEN_CENTER_Y - (int)mouse_y) * mouse_speed * delta /*/ delta*/);
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

