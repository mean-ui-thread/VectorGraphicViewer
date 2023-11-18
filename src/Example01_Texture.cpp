#include "Example01_Texture.h"

#include "imgui.h"

#include "ViewerApp.h"

bool Example01_Texture::setup()
{
#ifdef __EMSCRIPTEN__
    Shader defaultVert("assets/default_100_es.vert");
#else
    Shader defaultVert("assets/default_330_core.vert");
#endif

    if (defaultVert.compile() != 0)
    {
        return false;
    }

#ifdef __EMSCRIPTEN__
    Shader defaultFrag("assets/default_100_es.frag");
#else
    Shader defaultFrag("assets/default_330_core.frag");
#endif
    if (defaultFrag.compile() != 0)
    {
        return false;
    }

    defaultProgram = std::make_shared<ShaderProgram>(std::vector<AttributeInfo>({
        {"a_position", AttributeInfo::Float, 3},
        {"a_texCoord0", AttributeInfo::Float, 2},
    }));
    defaultProgram->attach(&defaultVert);
    defaultProgram->attach(&defaultFrag);
    if (defaultProgram->link() != 0)
    {
        return false;
    }

    u_MVP      = defaultProgram->getUniformLocation("u_MVP");
    u_texture0 = defaultProgram->getUniformLocation("u_texture0");

    backgroundTex = std::make_shared<Texture>("assets/background.jpg");
    if (backgroundTex->decode() != 0)
    {
        return false;
    }

    backgroundVBO = std::make_shared<VertexBuffer>();
    backgroundVBO->upload(backgroundVertices, VertexBuffer::Static);


    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    return true;
}

void Example01_Texture::render(const std::shared_ptr<ViewerApp> &app)
{
    float fieldOfViewRad = glm::radians(45.0f);
    float cameraDistance = ((float)app->displayHeight/2.0f) / tan(fieldOfViewRad/2.0f);
    float aspectRatio = (float)app->displayWidth / (float)app->displayHeight;

    projectionMatrix = glm::perspective(fieldOfViewRad, aspectRatio, 0.1f, cameraDistance+512.0f);
    projectionMatrix = glm::rotate(projectionMatrix, glm::radians(180.0f), glm::vec3(1, 0, 0));
    projectionMatrix = glm::translate(projectionMatrix, glm::vec3(-app->displayWidth*0.5f, -app->displayHeight*0.5f, cameraDistance));

    modelMatrix  = glm::translate(glm::identity<glm::mat4>(), position);
    modelMatrix *= glm::yawPitchRoll(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
    modelMatrix  = glm::scale(modelMatrix, scale);
    modelMatrix  = glm::translate(modelMatrix, -anchorPoint);

    // Draw texture
    defaultProgram->bind();
    defaultProgram->setUniform(u_texture0, 0);
    defaultProgram->setUniform(u_MVP, projectionMatrix * modelMatrix);
    backgroundVBO->bind(defaultProgram);
    backgroundTex->bind();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (GLsizei)mikeVertices.size());
    backgroundTex->unbind();
    backgroundVBO->unbind();
    defaultProgram->unbind();
}

void Example01_Texture::renderUI(const std::shared_ptr<ViewerApp> &app) {

    ImGui::SliderFloat("Translate X", &position.x, -app->displayWidth, app->displayWidth);
    ImGui::SliderFloat("Translate Y", &position.y, -app->displayWidth, app->displayWidth);
    ImGui::SliderFloat("Translate Z", &position.z, -8000.0f, 8000.0f);
    ImGui::SliderFloat("Rotate X", &rotation.x, 0, 360);
    ImGui::SliderFloat("Rotate Y", &rotation.y, 0, 360);
    ImGui::SliderFloat("Rotate Z", &rotation.z, 0, 360);
    ImGui::SliderFloat("Scale X", &scale.x, -2, 4);
    ImGui::SliderFloat("Scale Y", &scale.y, -2, 4);
    ImGui::SliderFloat("Center X", &anchorPoint.x, 0, 512);
    ImGui::SliderFloat("Center Y", &anchorPoint.y, 0, 512);
    ImGui::SliderFloat("Center Z", &anchorPoint.z, 0, 512);

}