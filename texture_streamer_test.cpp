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
#include <Textures/TextureStreamer2D.hpp>
#include <Textures/TextureData.hpp>
#include <Textures/TextureManager.hpp>
#include <Core.hpp>
#include <Utils/Log.hpp>
#include <MachineInfo.hpp>
#include <StateCache.hpp>

#include <mu/TimedCounter.hpp>

#include <fstream>
#include <stack>
#include <iostream>
#include <thread>
#include <mutex>

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

bool init_context(glm::vec2 const& sizes, GLFWwindow*& outMainWindow) {
    glfwSetErrorCallback(glfwError);

    if(!glfwInit()){
        mr::Log::LogString("Failed glfwInit in SimpleApp::Go.", MR_LOG_LEVEL_ERROR);
        return false;
    }

    ///Setup video mode
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vidMode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, vidMode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, vidMode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, vidMode->blueBits);
    //glfwWindowHint(GLFW_SAMPLES, 16);

    ///Main context creation
    GLFWwindow* mainWindow = 0;
    if((mainWindow = glfwCreateWindow(sizes.x, sizes.y, "MorglodsRenderTest", 0, 0)) == 0) {
        mr::Log::LogString("Failed glfw main window creation failed.", MR_LOG_LEVEL_ERROR);
        return false;
    }
    glfwMakeContextCurrent(mainWindow);

    if(!mr::Init()) {
        mr::Log::LogString("Failed render initialization SimpleApp::Go.", MR_LOG_LEVEL_ERROR);
        return false;
    }

    outMainWindow = mainWindow;
    return true;
}

void texture_streamer_test_main(glm::vec2 const& sizes) {
    const glm::vec2 screen_center = glm::vec2(sizes.x / 2.0f, sizes.y / 2.0f);

    mr::Log::Add(LogString);
    GLFWwindow* mainWindow = 0;
    if(!init_context(sizes, mainWindow)) return;

    //GL setup
    glClearColor(0.2f, 0.2, 0.2, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);

    ///Machine info
    mr::machine::PrintInfo();

    ///Scene setup
    mr::ShaderManager* shaderManager = mr::ShaderManager::GetInstance();
    mr::SceneManager sceneManager;

    mr::PerspectiveCamera camera = mr::PerspectiveCamera( mr::Transform::WorldForwardVector() * 2.0f, glm::vec3(0,0,0), 90.0f, sizes.x / sizes.y, 0.1f, 500.0f);
    camera.SetAutoRecalc(true);
    camera.SetPosition(glm::vec3(0.25f, 0.75f, -0.7f));
    camera.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
    camera.SetFarZ(10000.0f);
    sceneManager.SetMainCamera(&camera);

    //Create lights

    sceneManager.CreatePointLight(glm::vec3(0,100,100), glm::vec3(0.9,1,0.8), 100, 800);
    sceneManager.CreatePointLight(glm::vec3(0,100,200), glm::vec3(0.9,1,0.8), 100, 800);
    sceneManager.CreatePointLight(glm::vec3(0,100,300), glm::vec3(0.9,1,0.8), 100, 800);
    sceneManager.CreatePointLight(glm::vec3(0,100,400), glm::vec3(0.9,1,0.8), 100, 800);
    sceneManager.CreatePointLight(glm::vec3(0,100,500), glm::vec3(0.9,1,0.8), 100, 800);
    sceneManager.CreatePointLight(glm::vec3(0,100,600), glm::vec3(1.2,1.2,1.2), 600, 1500);
    sceneManager.CompleteLights();

    //Scene import
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

    //'Progress bar'
    importOptions.assimpProcessCallback = [&import_counter](float percent) -> bool {
        std::cout << "\rImporting";
        for(unsigned char i = 0; i < import_counter; ++i) {
            std::cout << '.';
        }
        ++import_counter;
        if(import_counter > 3) import_counter = 0;
        return true;
    };

    //Scene processing 'Progress bar'
    importOptions.progressCallback = [](mr::SceneLoader::ProgressInfo const& info) {
        std::cout << "\rMeshes (" << info.meshes << "/" << info.totalMeshes << "), Materials (" << info.materials << "/" << info.totalMaterials << ")";
    };

    scene_loader.Import(loadModelSrc, importOptions);

    //Instancing
    {
        unsigned int realInstNum = inst_num * inst_num;
        mr::IGPUBuffer* instGpuBuff = mr::GPUBufferManager::GetInstance().CreateBuffer(mr::IGPUBuffer::Static, sizeof(glm::vec3) * realInstNum);
        if(instGpuBuff == nullptr) {
            std::cout << "Failed create instance gpu buffer." << std::endl;
            return;
        }

        glm::vec3 instPos[realInstNum];
        for(unsigned int ix = 0; ix < inst_num; ++ix) {
            for(unsigned int iy = 0; iy < inst_num; ++iy) {
                instPos[ix*inst_num + iy] = glm::vec3(inst_x_offset * ix, 0.0f, inst_z_offset * iy);
            }
        }
        instGpuBuff->Write(instPos, 0, 0, sizeof(glm::vec3) * realInstNum, nullptr, nullptr);

        instGpuBuff->MakeResident();
        uint64_t instGpuBuffAddress = 0;
        if(!instGpuBuff->GetGPUAddress(instGpuBuffAddress)) {
            std::cout << "!!!! Failed get instances resident ptr !!!!" << std::endl;
            return;
        }
        shaderManager->SetGlobalUniform("MR_VERTEX_INSTANCED_POSITION", mr::IShaderUniformRef::Type::UInt64, &instGpuBuffAddress, true);
    }

    mr::ModelPtr model = scene_loader.GetModel();

    mr::EntityPtr entity = sceneManager.CreateEntity(model);
    sceneManager.GetRootNode()->CreateChild()->AddChild(std::static_pointer_cast<mr::SceneNode>(entity));

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
    glfwTerminate();
}
