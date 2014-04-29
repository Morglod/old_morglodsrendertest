/*
    EVENTS
*/
class ChangeColorEvent : public MR::EventHandle<MR::IUIElement*, const glm::vec2&, const int&, const bool&> {
public:
    void Invoke(MR::EventListener<MR::IUIElement*, const glm::vec2&, const int&, const bool&>* ptr, MR::IUIElement* sender, const glm::vec2& pos, const int& mbutton, const bool& state) override {
        if(state) sender->SetColor(_a);
        else sender->SetColor(_b);
    }

    ChangeColorEvent(const glm::vec4& colorA, const glm::vec4& colorB) : _a(colorA), _b(colorB) {}
protected:
    glm::vec4 _a, _b;
};

/*
    APP CLASS
*/
class TestUI : public MR::SimpleApp {
public:
    MR::UIElement* ui_test_box;

    bool Setup() {
        ui_test_box = new MR::UIElement(MR::UIManager::Instance(), MR::Rect(25.0f, 25.0f, 25.0f, 50.0f));
        ui_test_box->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        ui_test_box->OnMouseHoverDownStateChanged.RegisterHandle(new ChangeColorEvent(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)));
        MR::UIManager::Instance()->Add(ui_test_box);

        glClearColor(0.8f, 0.82f, 0.83f, 1.0f);

        return true;
    }

    void Input(){/*
        //Get mouse pos
        double mousex, mousey;
        window->GetMousePos(&mousex, &mousey);

        //Get in percent
        float px = mousex / (window_width/100.0f), py = mousey / (window_height/100.0f);

        //Is mouse point over ui element
        if(ui_test_box->GetRect().TestPoint(glm::vec2(px, py))) {
            ui_test_box->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

            //If Clicked
            if(glfwGetMouseButton(window->GetHandle(), GLFW_MOUSE_BUTTON_LEFT)){
                window->Close();
            }
        } else {
            ui_test_box->SetColor(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
        }*/
    }

    void Frame(const float& delta) {
        Input();

        MR::UIManager::Instance()->Frame(sys, delta);

        if(window->IsIconified()) {
            Sleep(1000);
        }
        else if(!window->IsFocused()) {
            Sleep(500);
        }
    }

    void Free() {
    }

    TestUI() : SimpleApp() {}
    virtual ~TestUI() {}
};
