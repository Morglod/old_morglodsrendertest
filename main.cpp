#include <MorglodsRender.hpp>
#include <Threads.hpp>
#include <Types.hpp>

#include <SimpleApp.hpp>

//Do not use hybryd graphics
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SCREEN_CENTER_X (WINDOW_WIDTH  / 2)
#define SCREEN_CENTER_Y (WINDOW_HEIGHT / 2)

//  #include "TestLods.hpp"
//  #include "TestTriangle.hpp"
  #include "TestSponza.hpp"
//  #include "TestUI.hpp"

void* AsyncRun(void* arg) {
    const char* a = (const char*)arg;
    for(int i = 0; i < 10; ++i){
        std::cout << std::string(a);
    }
    return new int(55);
}

void ThreadTest() {
    //THREADS

    class ThreadA : public MR::Thread {
    public:
        void* Run(void* arg) override {
            const char* a = (const char*)arg;
            for(int i = 0; i < 10; ++i){
                std::cout << std::string(a);
            }
            return new int(55);
        }

        ThreadA() : MR::Thread() {}
    };

    ThreadA* thread = new ThreadA();

    std::cout << thread->Start((void*)"Thread");
    std::cout << "_NOTTHREAD_";
    int* ret = (int*)MR::Thread::Join(thread);
    std::cout << std::to_string(*ret);

    //ASYNC

    const int asyncNum = 10000;
    MR::AsyncHandle ah[asyncNum];
    for(int i = 0; i < asyncNum; ++i){
        ah[i] = MR::AsyncCall(AsyncRun, (void*)("ASYNC"+std::to_string(i)).c_str());
    }
    for(int i = 0; i < asyncNum; ++i){
        ah[i].End();
    }
}

int main(){
    //ThreadTest();

    TestSponza app;
    app.Go("MorglodsRender test", WINDOW_WIDTH, WINDOW_HEIGHT);

    return 0;
}
