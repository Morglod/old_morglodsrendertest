#include "texture_streamer_test.hpp"

#include <Buffers/BufferManager.hpp>
#include <Geometry/GeometryManager.hpp>
#include <Geometry/Mesh.hpp>
#include <Shaders/ShaderManager.hpp>
#include <Models/Models.hpp>
#include <Scene/PerspectiveCamera.hpp>
#include <Scene/SceneLoader.hpp>
#include <Scene/SceneManager.hpp>
#include <Scene/Node.hpp>
#include <Scene/Entity.hpp>
#include <RTT/FrameBuffer.hpp>
#include <RTT/RTTManager.hpp>
#include <Core.hpp>
#include <Utils/Log.hpp>
#include <MachineInfo.hpp>
#include <StateCache.hpp>

#include <mu/TimedCounter.hpp>

#include <fstream>
#include <iostream>

#define GLEW_STATIC
#include <GL\glew.h>

#include <GLFW\glfw3.h>

namespace {

std::ofstream log_file("log.txt");

void LogString(const std::string& s, const int& level) {
    switch(level) {
    case MR_LOG_LEVEL_ERROR:
        std::cout << "Error: ";
        log_file << "Error: ";
        break;
    case MR_LOG_LEVEL_EXCEPTION:
        std::cout << "Exception: ";
        log_file << "Exception: ";
        break;
    case MR_LOG_LEVEL_INFO:
        std::cout << "Info: ";
        log_file << "Info: ";
        break;
    case MR_LOG_LEVEL_WARNING:
        std::cout << "Warning: ";
        log_file << "Warning: ";
        break;
    }
    std::cout << s << std::endl;
    log_file << s << std::endl;
    log_file.flush();
}

void glfwError(int level, const char* desc) {
    mr::Log::LogString("glfw error. level " + std::to_string(level) + ". " + std::string(desc), MR_LOG_LEVEL_ERROR);
}

}

void texture_streamer_test_main(glm::vec2 const& sizes) {
    mr::Log::Add(LogString);
    glfwSetErrorCallback(glfwError);

    if(!glfwInit()){
        mr::Log::LogString("Failed glfwInit in SimpleApp::Go.", MR_LOG_LEVEL_ERROR);
        return;
    }

    ///Setup video mode
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, vidMode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, vidMode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, vidMode->blueBits);
    const glm::vec2 screen_center = glm::vec2(sizes.x / 2.0f, sizes.y / 2.0f);

    //Thread pull
    const size_t threadPullSize = 1;
    GLFWwindow* contextPull[threadPullSize+1]; //0 is default

    //If need multithreaded mode
    GLFWwindow* prevWindow = 0;
    for(size_t i = 0; i < threadPullSize; ++i) {
        contextPull[i+1] = prevWindow = glfwCreateWindow(1, 1, ("MorglodsRenderTest_"+std::to_string(i)).c_str(), 0, prevWindow);
        if(!prevWindow) {
            mr::Log::LogString("Failed glfw second window creation failed.", MR_LOG_LEVEL_ERROR);
            return;
        }
    }

    ///Main context creation
    GLFWwindow* mainWindow = 0;
    if((mainWindow = glfwCreateWindow(sizes.x, sizes.y, "MorglodsRenderTest", 0, prevWindow)) == 0) {
        mr::Log::LogString("Failed glfw main window creation failed.", MR_LOG_LEVEL_ERROR);
        return;
    }
    contextPull[0] = mainWindow;
    glfwMakeContextCurrent(mainWindow);

    if(!mr::Init()) {
        mr::Log::LogString("Failed render initialization SimpleApp::Go.", MR_LOG_LEVEL_ERROR);
        return;
    }

    ///Machine info
    mr::machine::PrintInfo();

    ///Scene setup
    mr::PerspectiveCamera camera = mr::PerspectiveCamera( mr::Transform::WorldForwardVector() * 2.0f, glm::vec3(0,0,0), 90.0f, sizes.x / sizes.y, 0.1f, 500.0f);
    camera.SetAutoRecalc(true);
    camera.SetPosition(glm::vec3(0.25f, 0.75f, -0.7f));
    camera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
    camera.SetFarZ(10000.0f);

    //Setup shaders
    mr::ShaderManager* shaderManager = mr::ShaderManager::GetInstance();

    shaderManager->SetGlobalUniform("MR_MAT_MVP", mr::IShaderUniformRef::Mat4, camera.GetMVPPtr());
    shaderManager->SetGlobalUniform("MR_CAM_POS", mr::IShaderUniformRef::Vec3, camera.GetPositionPtr());

    mr::ShaderUniformMap* shaderUniformMap = shaderManager->DefaultShaderProgram()->GetMap();
    glUniformBlockBinding(shaderManager->DefaultShaderProgram()->GetGPUHandle(), shaderUniformMap->GetUniformBlock("MR_pointLights_block").location, 0);
    glUniformBlockBinding(shaderManager->DefaultShaderProgram()->GetGPUHandle(), shaderUniformMap->GetUniformBlock("MR_Textures_Block").location, 1);

    //Create lights
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

    lightsList.Create(glm::vec3(0,100,100), glm::vec3(0.9,1,0.8), 100, 800);
    lightsList.Create(glm::vec3(0,100,200), glm::vec3(0.9,1,0.8), 100, 800);
    lightsList.Create(glm::vec3(0,100,300), glm::vec3(0.9,1,0.8), 100, 800);
    lightsList.Create(glm::vec3(0,100,400), glm::vec3(0.9,1,0.8), 100, 800);
    lightsList.Create(glm::vec3(0,100,500), glm::vec3(0.9,1,0.8), 100, 800);
    lightsList.Create(glm::vec3(0,100,600), glm::vec3(1.2,1.2,1.2), 600, 1500);

    shaderManager->SetGlobalUniform("MR_numPointLights", mr::IShaderUniformRef::Int, &lightsList.num);

    mr::IGPUBuffer* lightsGpuBuff = mr::GPUBufferManager::GetInstance().CreateBuffer(mr::IGPUBuffer::FastChange, 16777216); //16mb

    mr::ShaderUniformBlockInfo* blockInfo = &(shaderUniformMap->GetUniformBlock("MR_pointLights_block"));
    for(int i = 0; i < lightsList.num; ++i) {
        const std::string uniform_name = "MR_pointLights["+std::to_string(i)+"].";
        lightsGpuBuff->Write(&lightsList.pointLights[i].pos, 0, blockInfo->GetOffset(uniform_name+"pos"), sizeof(glm::vec3), nullptr, nullptr);
        lightsGpuBuff->Write(&lightsList.pointLights[i].color, 0, blockInfo->GetOffset(uniform_name+"color"), sizeof(glm::vec3), nullptr, nullptr);
        lightsGpuBuff->Write(&lightsList.pointLights[i].innerR, 0, blockInfo->GetOffset(uniform_name+"innerRange"), sizeof(float), nullptr, nullptr);
        lightsGpuBuff->Write(&lightsList.pointLights[i].outerR, 0, blockInfo->GetOffset(uniform_name+"outerRange"), sizeof(float), nullptr, nullptr);
    }

    std::string loadModelSrc = "";
    float inst_x_offset = 10.0f;
    float inst_z_offset = 10.0f;
    unsigned int inst_num = 100;

#ifndef DEBUG_BUILD
    std::cout << "Load model: ";
    std::cin >> loadModelSrc;

    std::cout << "Instance X offset (3700 - sponza): ";
    std::cin >> inst_x_offset;

    std::cout << "Instance Z offset (2200 - sponza): ";
    std::cin >> inst_z_offset;

    std::cout << "sqrt(Instances num): ";
    std::cin >> inst_num;
#else
    loadModelSrc = "pyr_test.obj";
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
    mr::IGPUBuffer* instGpuBuff = mr::GPUBufferManager::GetInstance().CreateBuffer(mr::IGPUBuffer::Static, sizeof(glm::vec3) * realInstNum);
    if(instGpuBuff == nullptr) {
        std::cout << "Failed create instance gpu buffer." << std::endl;
        return;
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

    auto geomsAr = scene_loader.GetGeometry();
    for(size_t i = 0; i < geomsAr.GetNum(); ++i) {
        mr::IGeometry* geom = geomsAr.At(i);
        geom->GetDrawParams()->SetInstancesNum(realInstNum);
    }

    instGpuBuff->MakeResident();
    uint64_t instGpuBuffAddress = 0;
    if(!instGpuBuff->GetGPUAddress(instGpuBuffAddress)) {
        std::cout << "!!!! Failed get resident ptr !!!!" << std::endl;
        return;
    }
    shaderManager->SetNVVBUMPointer(4, instGpuBuffAddress);

    mr::ModelPtr model = mr::ModelPtr(new mr::Model());
    mr::SubModel* subModel = new mr::SubModel();

    auto sceneMeshes = scene_loader.GetMeshes();
    mr::TStaticArray<mr::MeshWeakPtr> subModelMeshes(sceneMeshes.GetNum());
    std::vector<mr::MeshPtr> meshes;
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

    mr::SceneManager sceneManager;
    mr::EntityPtr entity = sceneManager.CreateEntity(model);
    sceneManager.GetRootNode()->CreateChild()->AddChild(std::static_pointer_cast<mr::SceneNode>(entity));

    glClearColor(0.2f, 0.2, 0.2, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    std::cout << std::endl << "GPU Buffers mem: " << mr::GPUBufferManager::GetInstance().GetUsedGPUMemory() << std::endl;

    mr::FrameBuffer* frameBuf = mr::RTTManager::GetInstance().CreateFrameBuffer();
    mr::RenderBuffer* renderBufColor = mr::RTTManager::GetInstance().CreateRenderBuffer(mr::Texture::SDF_RGB8, glm::uvec2(800,600), 0);
    mr::RenderBuffer* renderBufDepth = mr::RTTManager::GetInstance().CreateRenderBuffer(mr::Texture::SDF_DEPTH_COMPONENT24, glm::uvec2(800,600), 0);
    frameBuf->AttachColor(renderBufColor, 0);
    frameBuf->AttachDepth(renderBufDepth);

    mr::FrameBuffer::CompletionInfo frameBufInfo = frameBuf->Complete();
    mr::Log::LogString("FrameBuffer status: " + mr::FrameBuffer::CompletionInfoToString(frameBufInfo));

    mr::StateCache::GetDefault()->DrawTo(frameBuf);

    ///FPS
    class FpsCounter : public mu::TimedCounter {
    public:
        void Invoke() override {
            printf(("\r"+std::to_string(counter)+" ").c_str());
        }
    };

    FpsCounter fps;

    ///Main loop
    double mouse_x = 0.0, mouse_y = 0.0;
    float lTime = glfwGetTime();
    float delta = 0.00000001f;

    while(!glfwWindowShouldClose(mainWindow)) {
        ///Frame

        //Camera moves
        float move_speed = 8.0f;
        const float mouse_speed = 0.15f;

        glfwGetCursorPos(mainWindow, &mouse_x, &mouse_y);

        if(glfwGetKey(mainWindow, GLFW_KEY_LEFT_SHIFT)) {
            move_speed += 1000.0f;
        }

        int pos_offset = mr::ShaderManager::GetInstance()->DefaultShaderProgram()->GetMap()->GetUniformBlock("MR_pointLights_block").GetOffset("MR_pointLights[0].pos");
        if(glfwGetKey(mainWindow, GLFW_KEY_R)) {
            lightsList.pointLights[0].pos += glm::vec3(1000.0f * delta, 0, 0);
            lightsGpuBuff->Write(&lightsList.pointLights[0].pos, 0, pos_offset, sizeof(glm::vec3), nullptr, nullptr);
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_F)) {
            lightsList.pointLights[0].pos -= glm::vec3(1000.0f * delta, 0, 0);
            lightsGpuBuff->Write(&lightsList.pointLights[0].pos, 0, pos_offset, sizeof(glm::vec3), nullptr, nullptr);
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_T)) {
            lightsList.pointLights[0].pos += glm::vec3(0, 0, 1000.0f * delta);
            lightsGpuBuff->Write(&lightsList.pointLights[0].pos, 0, pos_offset, sizeof(glm::vec3), nullptr, nullptr);
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_G)) {
            lightsList.pointLights[0].pos -= glm::vec3(0, 0, 1000.0f * delta);
            lightsGpuBuff->Write(&lightsList.pointLights[0].pos, 0, pos_offset, sizeof(glm::vec3), nullptr, nullptr);
        }

        if(glfwGetKey(mainWindow, GLFW_KEY_W)) {
            camera.MoveForward(move_speed * delta);
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_S))  {
            camera.MoveForward(-move_speed * delta);
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_A))  {
            camera.MoveLeft(move_speed * delta);
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_D))  {
            camera.MoveLeft(-move_speed * delta);
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_SPACE)) {
            camera.MoveUp(move_speed * delta);
        }
        if(glfwGetKey(mainWindow, GLFW_KEY_C)) {
            camera.MoveUp(-move_speed * delta);
        }
        if(glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_RIGHT)) {
            glfwSetCursorPos(mainWindow, (double)screen_center.x, (double)screen_center.y);

            camera.Roll((screen_center.x - (float)mouse_x) * mouse_speed);
            camera.Yaw((screen_center.y - (float)mouse_y) * mouse_speed);
        }

        //Draw
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mr::ShaderManager* shaderManager = mr::ShaderManager::GetInstance();
        shaderManager->UpdateAllGlobalUniforms();
        glUseProgram(shaderManager->DefaultShaderProgram()->GetGPUHandle());
        mr::StateCache::GetDefault()->BindUniformBuffer(lightsGpuBuff, 0);
        sceneManager.Draw();

        fps.Count(delta);

        frameBuf->ToScreen(mr::FrameBuffer::ColorBit,
                           glm::lowp_uvec4(0,0,800,600),
                           glm::lowp_uvec4(0,0,800,600),
                           true);

        glfwPollEvents();
        glfwSwapBuffers(mainWindow);

        float nTime = glfwGetTime();
        delta = nTime - lTime;
        lTime = nTime;
    }

    ///Free

    ///Shutdown
    mr::Shutdown();
    glfwMakeContextCurrent(0);
    for(size_t i = 0; i < threadPullSize+1; ++i) {
        glfwDestroyWindow(contextPull[threadPullSize-i]);
    }
    glfwTerminate();
}
