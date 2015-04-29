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

#define POINT_LIGHTS_NUM 6

class TestTriangle : public mr::SimpleApp {
public:
    mr::IGeometry* geom;
    /*MR::GPUObjectHandlePtr<MR::IShaderProgram> prog;
    MR::GPUObjectHandlePtr<MR::ITexture> tex;*/
    mr::IShaderProgram* prog;
    mr::ITexture* tex;
    mr::ITexture* tex2;
    mr::ModelPtr model;
    std::vector<mr::MeshPtr> meshes;
    mr::SceneManager sceneManager;
    mr::IVertexAttribute* instAttrib;
    mr::IGPUBuffer* instGpuBuff;
   // mr::IFrameBuffer* frameBuffer;

   int texColorUnit = 0;
   int sphericalTexUnit = 1;

    struct LightDesc {
        glm::vec3 pos, color;
        float innerR, outerR;
        LightDesc(glm::vec3 const& p, glm::vec3 const& c, float iR, float oR) : pos(p), color(c), innerR(iR), outerR(oR) {}
    };

    const int point_lights_num = POINT_LIGHTS_NUM;
    std::vector<LightDesc> pointLights;

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

        {
            mr::Image img;
            std::cout << "Create: " << img.Create(glm::ivec2(500, 500), 3, glm::ivec4(255,255,255,255)) << std::endl;
            img.DrawLine(glm::ivec2(0,0), glm::ivec2(499, 499), glm::ivec4(255,0,0,255));
            img.DrawRect(glm::ivec2(100,100), glm::ivec2(399, 399), glm::ivec4(0,255,0,255), false);
            img.DrawRect(glm::ivec2(200,200), glm::ivec2(299, 299), glm::ivec4(0,0,255,255), true);
            img.Invert();

            mr::IImagePtr img2 = img.Resize(glm::ivec2(300, 300), mr::IImage::NearestNeighborScale);
            img.Lerp(glm::ivec2(50,50), img2.get(), 0.8f);

            mr::ImageSaverSOIL soil;
            mr::IImageSaver::Options options; options.fileType = mr::IImageSaver::Options::BMP;
            std::cout << "Save: " << soil.Save("out.bmp", options, &img) << std::endl;
            std::cout << "Save2: " << soil.Save("out2.bmp", options, img2.get()) << std::endl;
        }

        //Setup shaders

        mr::ShaderManager* shaderManager = mr::ShaderManager::GetInstance();
        prog = shaderManager->DefaultShaderProgram();

        mr::ShaderUniformDesc mvpUniform("MR_MAT_MVP", mr::IShaderUniformRef::Mat4);
        shaderManager->SetGlobalUniform(mvpUniform, camera->GetMVPPtr());

        mr::ShaderUniformDesc texColorUniform("MR_TEX_COLOR", mr::IShaderUniformRef::Sampler2D);
        shaderManager->SetGlobalUniform(texColorUniform, &texColorUnit);

        shaderManager->SetGlobalUniform(mr::ShaderUniformDesc("MR_numPointLights", mr::IShaderUniformRef::Int), &point_lights_num);

        for(int i = 0; i < point_lights_num; i++) {
            const std::string light_index = std::to_string(i);
            pointLights.push_back(LightDesc(glm::vec3(i * 20, 1, 0), glm::vec3(1,1,1), 10, 50));
            mr::ShaderUniformDesc   u_lightPos("MR_pointLights["+light_index+"].pos", mr::IShaderUniformRef::Vec3),
                                    u_lightColor("MR_pointLights["+light_index+"].color", mr::IShaderUniformRef::Vec3),
                                    u_lightIR("MR_pointLights["+light_index+"].innerRange", mr::IShaderUniformRef::Float),
                                    u_lightOR("MR_pointLights["+light_index+"].outerRange", mr::IShaderUniformRef::Float);
            shaderManager->SetGlobalUniform(u_lightPos, &pointLights[i].pos);
            shaderManager->SetGlobalUniform(u_lightColor, &pointLights[i].color);
            shaderManager->SetGlobalUniform(u_lightIR, &pointLights[i].innerR);
            shaderManager->SetGlobalUniform(u_lightOR, &pointLights[i].outerR);
            std::cout << std::to_string(pointLights[i].pos) << std::endl;
            std::cout << std::to_string(pointLights[i].color) << std::endl;
            std::cout << std::to_string(pointLights[i].innerR) << std::endl;
            std::cout << std::to_string(pointLights[i].outerR) << std::endl;
        }

        mr::ShaderUniformDesc texColorUniform2("testSphericalLightMap", mr::IShaderUniformRef::Sampler2D);
        //shaderManager->SetGlobalUniform(texColorUniform2, &sphericalTexUnit);


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

        auto sceneMaterials = scene_loader.GetMaterials();
        for(size_t i = 0; i < sceneMaterials.GetNum(); ++i) {
            sceneMaterials.At(i)->SetShaderProgram(prog);
        }

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
        mr::ShaderManager::GetInstance()->UpdateAllGlobalUniforms();
        sceneManager.Draw();
        fps.Count(delta);
    }

    void Free() {
    }

    TestTriangle() : mr::SimpleApp() {}
    virtual ~TestTriangle() {  }
};

