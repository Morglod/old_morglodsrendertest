#include "TimedCounter.hpp"

#include <Buffers/BuffersManager.hpp>
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
#include <Shaders/ShaderManager.hpp>

#include <Macro.hpp>

#include <Images/Image.hpp>
#include <Images/SOILwrapper.hpp>

class TestTriangle : public mr::SimpleApp {
public:
    mr::IGeometry* geom;
    mr::IShaderProgram* prog;
    mr::ITexture* tex;
    mr::ITexture* tex2;
    mr::ModelPtr model;
    std::vector<mr::MeshPtr> meshes;
    mr::SceneManager sceneManager;
    mr::IVertexAttribute* instAttrib;
    mr::IGPUBuffer* instGpuBuff;
    mr::IGPUBuffer* lightsGpuBuff;
    unsigned int lightsGpuBuff_bindindex = 0;

   int texColorUnit = 0;
   int sphericalTexUnit = 1;

    struct LightDesc {
        glm::vec3 pos, color;
        float innerR, outerR;
        LightDesc(glm::vec3 const& p, glm::vec3 const& c, float iR, float oR) : pos(p), color(c), innerR(iR), outerR(oR) {}
    };

    struct LightsList {
        int num = 0;
        std::vector<LightDesc> pointLights;

        inline LightDesc& Create(glm::vec3 const& pos, glm::vec3 const& color, float innerR, float outerR) {
            pointLights.push_back(LightDesc(pos, color, innerR, outerR));
            num++;
            return pointLights[pointLights.size()-1];
        }
    };

    LightsList lightsList;

    bool Setup() {
        mr::machine::PrintInfo();

        //Setup shaders

        mr::ShaderManager* shaderManager = mr::ShaderManager::GetInstance();

        shaderManager->SetGlobalUniform("MR_MAT_MVP", mr::IShaderUniformRef::Mat4, camera->GetMVPPtr());
        shaderManager->SetGlobalUniform("MR_TEX_COLOR", mr::IShaderUniformRef::Sampler2D, &texColorUnit);

        //Create lights

        lightsList.Create(glm::vec3(0,100,100), glm::vec3(0.9,1,0.8), 100, 800);

        for(int i = 0; i < 20; ++i) {
            lightsList.Create(glm::vec3(i*30,0.1f,0), glm::vec3(0.9,1,0.8), 1, 30);
        }

        shaderManager->SetGlobalUniform("MR_numPointLights", mr::IShaderUniformRef::Int, &lightsList.num);

        lightsGpuBuff = mr::GPUBuffersManager::GetInstance().CreateBuffer(mr::IGPUBuffer::FastChange, 16777216); //16mb

        mr::ShaderUniformMap* shaderUniformMap = shaderManager->DefaultShaderProgram()->GetMap();
        mr::ShaderUniformBlockInfo* blockInfo = &(shaderUniformMap->GetUniformBlock("MR_pointLights_block"));
        lightsGpuBuff_bindindex = blockInfo->location;

        for(int i = 0; i < lightsList.num; ++i) {
            const std::string uniform_name = "MR_pointLights["+std::to_string(i)+"].";
            lightsGpuBuff->Write(&lightsList.pointLights[i].pos, 0, blockInfo->GetOffset(uniform_name+"pos"), sizeof(glm::vec3), nullptr, nullptr);
            lightsGpuBuff->Write(&lightsList.pointLights[i].color, 0, blockInfo->GetOffset(uniform_name+"color"), sizeof(glm::vec3), nullptr, nullptr);
            lightsGpuBuff->Write(&lightsList.pointLights[i].innerR, 0, blockInfo->GetOffset(uniform_name+"innerRange"), sizeof(float), nullptr, nullptr);
            lightsGpuBuff->Write(&lightsList.pointLights[i].outerR, 0, blockInfo->GetOffset(uniform_name+"outerRange"), sizeof(float), nullptr, nullptr);
        }

        ///TEST
        std::string loadModelSrc = "";
        std::string loadTexSrc = "";
        float inst_x_offset = 10.0f;
        float inst_z_offset = 10.0f;
        unsigned int inst_num = 100;

#ifndef DEBUG_BUILD
        std::cout << "Load model: ";
        std::cin >> loadModelSrc;

        std::cout << "Load tex: ";
        std::cin >> loadTexSrc;

        std::cout << "Instance X offset (3700 - sponza): ";
        std::cin >> inst_x_offset;

        std::cout << "Instance Z offset (2200 - sponza): ";
        std::cin >> inst_z_offset;

        std::cout << "sqrt(Instances num): ";
        std::cin >> inst_num;
#else
        loadModelSrc = "pyr_test.obj";
        loadTexSrc = "mramor6x6.png";
        inst_x_offset = 10.0f;
        inst_z_offset = 10.0f;
        inst_num = 2;
#endif

        mr::SceneLoader scene_loader;
        mr::SceneLoader::ImportOptions importOptions;
        unsigned char import_counter = 0;
        importOptions.assimpProcessCallback = [&import_counter](float percent) -> bool {
            std::cout << "\rImporting";
            for(unsigned char i = 0; i < import_counter; ++i) {
                std::cout << '.';
            }
            ++import_counter;
            if(import_counter > 3) import_counter = 0;
            return true;
        };
        importOptions.progressCallback = [](mr::SceneLoader::ProgressInfo const& info) {
            std::cout << "\rMeshes (" << info.meshes << "/" << info.totalMeshes << "), Materials (" << info.materials << "/" << info.totalMaterials << ")";
        };
        scene_loader.Import(loadModelSrc, importOptions);

        //Instancing
        unsigned int realInstNum = inst_num * inst_num;
        instGpuBuff = mr::GPUBuffersManager::GetInstance().CreateBuffer(mr::IGPUBuffer::Static, sizeof(glm::vec3) * realInstNum);
        if(instGpuBuff == nullptr) {
            std::cout << "Failed create instance gpu buffer." << std::endl;
            return false;
        }

        mr::IGPUBuffer::IMappedRangePtr mappedInstGpuBuff = instGpuBuff->Map(0, sizeof(glm::vec3) * realInstNum, mr::IGPUBuffer::IMappedRange::Write | mr::IGPUBuffer::IMappedRange::Unsynchronized | mr::IGPUBuffer::IMappedRange::Invalidate);
        if(mappedInstGpuBuff == nullptr) {
            glm::vec3 instPos[realInstNum];
            for(unsigned int ix = 0; ix < inst_num; ++ix) {
                for(unsigned int iy = 0; iy < inst_num; ++iy) {
                    instPos[ix*inst_num + iy] = glm::vec3(inst_x_offset * ix, 0.0f, inst_z_offset * iy);
                }
            }
            instGpuBuff->Write(instPos, 0, 0, sizeof(glm::vec3) * realInstNum, nullptr, nullptr);
        } else {
            glm::vec3* instPos = (glm::vec3*)(mappedInstGpuBuff->Get());
            for(unsigned int ix = 0; ix < inst_num; ++ix) {
                for(unsigned int iy = 0; iy < inst_num; ++iy) {
                    instPos[ix*inst_num + iy] = glm::vec3(inst_x_offset * ix, 0.0f, inst_z_offset * iy);
                }
            }
            mappedInstGpuBuff->UnMap();
        }

        instAttrib = new mr::VertexAttributeCustom(3, &mr::VertexDataTypeFloat::GetInstance(), 4, 1);

        auto geomsAr = scene_loader.GetGeometry();
        for(size_t i = 0; i < geomsAr.GetNum(); ++i) {
            mr::IGeometry* geom = geomsAr.At(i);
            geom->GetGeometryBuffer()->SetAttribute(instAttrib, instGpuBuff);
            geom->GetDrawParams()->SetInstancesNum(realInstNum);
        }
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

        tex2 = mr::Texture::FromFile("material8.png");

        if(tex2) {
            mr::StateCache* stateCache = mr::StateCache::GetDefault();
            mr::ITextureSettings* texSettings = new mr::TextureSettings();
            texSettings->Create();
            tex2->SetSettings(texSettings);
            stateCache->BindTexture(tex2, 1);
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

        std::cout << std::endl << "GPU Buffers mem: " << mr::GPUBuffersManager::GetInstance().GetUsedGPUMemory() << std::endl;

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
        float move_speed = 8.0f;
        const float mouse_speed = 0.15f;

        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
            move_speed += 1000.0f;
        }

        int pos_offset = mr::ShaderManager::GetInstance()->DefaultShaderProgram()->GetMap()->GetUniformBlock("MR_pointLights_block").GetOffset("MR_pointLights[0].pos");
        if(glfwGetKey(window, GLFW_KEY_R)) {
            lightsList.pointLights[0].pos += glm::vec3(1000.0f * delta, 0, 0);
            lightsGpuBuff->Write(&lightsList.pointLights[0].pos, 0, pos_offset, sizeof(glm::vec3), nullptr, nullptr);
        }
        if(glfwGetKey(window, GLFW_KEY_F)) {
            lightsList.pointLights[0].pos -= glm::vec3(1000.0f * delta, 0, 0);
            lightsGpuBuff->Write(&lightsList.pointLights[0].pos, 0, pos_offset, sizeof(glm::vec3), nullptr, nullptr);
        }

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

            camera->Roll((float)(SCREEN_CENTER_X - (int)mouse_x) * mouse_speed);
            camera->Yaw((float)(SCREEN_CENTER_Y - (int)mouse_y) * mouse_speed);
        }

        //Draw
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mr::ShaderManager* shaderManager = mr::ShaderManager::GetInstance();
        shaderManager->UpdateAllGlobalUniforms();
        glUseProgram(shaderManager->DefaultShaderProgram()->GetGPUHandle());
        glUniformBlockBinding(shaderManager->DefaultShaderProgram()->GetGPUHandle(), lightsGpuBuff_bindindex, 0);
        mr::StateCache::GetDefault()->BindUniformBuffer(lightsGpuBuff, 0);
        sceneManager.Draw();

        fps.Count(delta);
    }

    void Free() {
    }

    TestTriangle() : mr::SimpleApp() {}
    virtual ~TestTriangle() {  }
};

