/*#include "ShaderEditor.hpp"
#include <MachineInfo.hpp>
#include <Geometry/GeometryInterfaces.hpp>

bool ShaderEditor::Setup() {
    MR::MachineInfo::PrintInfo();


}

void ShaderEditor::Frame(const float& delta) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(ShaderProgram) ShaderProgram->Use();
    if(Geometry) Geometry->Draw();
}

void ShaderEditor::Free() {
    if(ShaderProgram) delete ShaderProgram;
    if(Geometry) delete Geometry;

    ShaderProgram = nullptr;
    Geometry = nullptr;
}

ShaderEditor::ShaderEditor() : Geometry(nullptr), ShaderProgram(nullptr) {
}

ShaderEditor::~ShaderEditor() {
}*/
