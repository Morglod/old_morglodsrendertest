class TestTriangle : public MR::SimpleApp {
public:
    MR::IGeometryBuffer* geom;
    MR::Shader* shader;

    const char * VertexShaderCode =
    "#version 330\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "layout(location = 0) in vec3 pos;\n"
    "layout(location = 1) in vec3 color;\n"
    "layout(location = 1) out vec3 colorOut;\n"
    "void main(){\n"
    "gl_Position = vec4(pos, 1);\n"
    "colorOut = color;\n"
    "}\n";

    const char * FragmentShaderCode =
    "#version 330\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "layout(location = 1) in vec3 color;\n"
    "void main(){\n"
    "gl_FragColor = vec4(color, 1);\n"
    "}\n";

    bool Setup() {
        MR::Log::LogString( std::string("Machine info:") +
            std::string("\nVersion: ") + MR::MachineInfo::gl_version_string() +
            std::string("\nGLSL: ") + MR::MachineInfo::gl_version_glsl() +
            std::string("\nOpenGL: ") + std::to_string(MR::MachineInfo::gl_version_major()) + std::string(" ") + std::to_string(MR::MachineInfo::gl_version_minor()) +
            std::string("\nGPU: ") + MR::MachineInfo::gpu_name() + std::string(" from ") + MR::MachineInfo::gpu_vendor_string() +
            std::string("\nMem Total(kb): ") + std::to_string(MR::MachineInfo::total_memory_kb()) + std::string(" Current(kb): ") + std::to_string(MR::MachineInfo::current_memory_kb()) + "\n\n", MR_LOG_LEVEL_INFO);

        MR::SubShader* vsub = new MR::SubShader(std::string(VertexShaderCode), MR::ISubShader::Type::Vertex);
        MR::SubShader* fsub = new MR::SubShader(std::string(FragmentShaderCode), MR::ISubShader::Type::Fragment);

        shader = dynamic_cast<MR::Shader*>(MR::ShaderManager::Instance()->Create("Auto", "FromSubs"));
        shader->AttachSubShader(vsub);
        shader->AttachSubShader(fsub);
        shader->Link();

        float triData[18] {
            -1, -1, 0, //pos
            1, 0, 0, //color
            0, 1, 0,
            0, 1, 0,
            1, -1, 0,
            0, 0, 1
        };

        unsigned int triIData[3] {
            0, 1, 2
        };

        MR::VertexBuffer* vbuffer = new MR::VertexBuffer();
        vbuffer->Buffer(&triData[0], sizeof(float)*18, MR::IGLBuffer::Static+MR::IGLBuffer::Draw, MR::IGLBuffer::ReadOnly);
        vbuffer->SetNum(3);

        MR::IVertexFormat* vformat = new MR::VertexFormatCustom();
        vformat->AddVertexAttribute(new MR::VertexAttributeCustom(3, MR::VertexDataTypeFloat::Instance(), 0));
        vformat->AddVertexAttribute(new MR::VertexAttributeCustom(3, MR::VertexDataTypeFloat::Instance(), 1));

        /*MR::IndexBuffer* ibuffer = new MR::IndexBuffer();
        ibuffer->Buffer(&triIData[0], sizeof(unsigned int)*3, MR::IGLBuffer::Static+MR::IGLBuffer::Draw, MR::IGLBuffer::ReadOnly);
        ibuffer->SetNum(3);

        MR::IIndexFormat* iformat = new MR::IndexFormatCustom(MR::VertexDataTypeUInt::Instance());*/

        geom = new MR::GeometryBuffer(vbuffer, nullptr, vformat, nullptr, MR::IGeometryBuffer::Draw_Triangles);

        glClearColor(0.2f, 0.2, 0.2, 1.0f);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        return true;
    }

    void Input(const float& delta){
    }

    void DrawScene(){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader->Use(&context);
        geom->Draw(&context);
    }

    void Frame(const float& delta) {
        Input(delta);
        DrawScene();

        if(window->IsIconified()) {
            Sleep(1000);
        }
        else if(!window->IsFocused()) {
            Sleep(500);
        }
    }

    void Free() {
        delete geom;
        delete shader;
    }

    TestTriangle() : MR::SimpleApp() {}
    virtual ~TestTriangle() {}
};

