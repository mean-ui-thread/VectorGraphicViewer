
#include "ShaderProgram.h"

#include <imgui.h>

ShaderProgram::ShaderProgram(const std::string &name, const std::vector<AttributeInfo> &attributes) :
    AbstractGPUObject(name),
    m_attributes(attributes)
{
    m_attributeLocations.resize(attributes.size(), -1);
    m_attributeOffsets.resize(attributes.size(), 0);
    m_handle = glCreateProgram();
}

ShaderProgram::~ShaderProgram()
{
    if (m_handle)
    {
        glDeleteProgram(m_handle);
        m_handle = 0;
    }
}

bool ShaderProgram::attach(const Shader &shader)
{
    GLuint shaderHandle;

    if (shader.ext == "vert" || shader.ext == "vsh")
    {
        shaderHandle = glCreateShader(GL_VERTEX_SHADER);
    }
    else if (shader.ext == "frag" || shader.ext == "fsh")
    {
        shaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
    }

    const char * rawSourceCode = shader.source.c_str();

    glShaderSource(shaderHandle, 1, &rawSourceCode, NULL);
    glCompileShader(shaderHandle);

    GLint compileStatus;
    glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileStatus);

    if (!compileStatus)
    {
        GLchar infoLog[1024];
        glGetShaderInfoLog(shaderHandle, sizeof(infoLog), NULL, infoLog);
        SDL_LogCritical(0, "Could not compile %s : %s", shader.filePath.c_str(), infoLog);
        return false;
    }

    glAttachShader(m_handle, shaderHandle);
    glDeleteShader(shaderHandle);

    m_shaders.push_back(shader);

    return true;
}

bool ShaderProgram::link()
{
    glLinkProgram(m_handle);

    GLint linkStatus;
    glGetProgramiv(m_handle, GL_LINK_STATUS, &linkStatus);
    if (!linkStatus)
    {
        GLchar infoLog[1024];
        glGetProgramInfoLog(m_handle, sizeof(infoLog), NULL, infoLog);
        SDL_LogCritical(0, "Could not link shader program:\n%s", infoLog);
        return false;
    }

    m_vertexSize = 0;
    for(size_t i = 0; i < m_attributes.size(); ++i)
    {
        m_attributeLocations[i] = glGetAttribLocation(m_handle, m_attributes[i].name.c_str());
        if (m_attributeLocations[i] == -1)
        {
            SDL_LogCritical(0, "Could not find attribute named \"%s\" in shader program.", m_attributes[i].name.c_str());
            return -1;
        }

        m_attributeOffsets[i] = m_vertexSize;
        m_vertexSize += m_attributes[i].size();
    }


    return true;
}

GLint ShaderProgram::getUniformLocation(const char *uniformName)
{
    GLint location = glGetUniformLocation(m_handle, uniformName);
    if (location == -1)
    {
        SDL_LogCritical(0, "Could not find uniform named \"%s\" in shader program.", uniformName);
    }
    return location;
}

size_t ShaderProgram::getMemoryUsage() const {
    GLint binarySize = 0;
    glGetProgramiv(m_handle, GL_PROGRAM_BINARY_LENGTH, &binarySize);
    return binarySize;
}

void ShaderProgram::renderUI() {
    for (size_t i = 0; i < m_shaders.size(); ++i) {

        Shader &shader = m_shaders[i];

        if(ImGui::TreeNode(shader.filePath.c_str())) {
            if (ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg| ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(0);
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Type");
                if (shader.ext == "vert" || shader.ext == "vsh")
                {
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    char title[] = "Vertex Shader";
                    ImGui::InputText("##vert", title, strlen(title), ImGuiInputTextFlags_ReadOnly);
                }
                else if (shader.ext == "frag" || shader.ext == "fsh")
                {
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    char title[] = "Fragment Shader";
                    ImGui::InputText("##frag", title, strlen(title), ImGuiInputTextFlags_ReadOnly);
                }
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(0);
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Source");

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputTextMultiline("##source", (char*)shader.source.c_str(), shader.source.size(), ImVec2(-FLT_MIN, 0), ImGuiInputTextFlags_ReadOnly);

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }

    }

}