#include "VertexBuffer.h"

#include <iostream>

template<>
void VertexBuffer<TextureVertex>::renderUI() {
    if(ImGui::TreeNode("Vertex Buffer Object (TextureVertex)")) {
        if (ImGui::BeginTable(name.c_str(), 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing()*7)))
        {
            ImGui::TableSetupColumn("Pos X");
            ImGui::TableSetupColumn("Pos Y");
            ImGui::TableSetupColumn("Pos Z");
            ImGui::TableSetupColumn("Tex U");
            ImGui::TableSetupColumn("Tex V");
            ImGui::TableHeadersRow();

            float * values = &vertices[0].position.x;
            size_t count = vertices.size() * sizeof(TextureVertex) / sizeof(float);
            for (size_t i = 0; i < count; ++i)
            {
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushID(i);
                if(ImGui::InputFloat("##vertex", &values[i])) {
                    glBindBuffer(GL_ARRAY_BUFFER, handle);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(TextureVertex) * vertices.size(), vertices.data(), hint);
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
}

template<>
void VertexBuffer<ColorVertex>::renderUI() {
    if(ImGui::TreeNode("Vertex Buffer Object (ColorVertex)")) {
        if (ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg ))
        {
            ImGui::TableSetupColumn("Position");
            ImGui::TableSetupColumn("Color");
            ImGui::TableHeadersRow();

            bool verticesChanged = false;

            static std::vector<glm::vec4> colors;
            colors.resize(vertices.size());

            size_t id = 0;
            for (size_t i = 0; i < vertices.size(); ++i)
            {
                colors[i].r = vertices[i].color.r / 255.0f;
                colors[i].g = vertices[i].color.g / 255.0f;
                colors[i].b = vertices[i].color.b / 255.0f;
                colors[i].a = vertices[i].color.a / 255.0f;

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushID(id++);
                if(ImGui::InputFloat3("##vertex.pos.x", &vertices[i].position.x)) {
                    verticesChanged = true;
                }
                ImGui::PopID();

                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushID(id++);
                if(ImGui::ColorEdit4("##vertex.col", &colors[i][0])) {
                    vertices[i].color.r = colors[i].r * 255.0f;
                    vertices[i].color.g = colors[i].g * 255.0f;
                    vertices[i].color.b = colors[i].b * 255.0f;
                    vertices[i].color.a = colors[i].a * 255.0f;
                    verticesChanged = true;
                }
                ImGui::PopID();
            }
            ImGui::EndTable();

            if (verticesChanged) {
                glBindBuffer(GL_ARRAY_BUFFER, handle);
                glBufferData(GL_ARRAY_BUFFER, sizeof(ColorVertex) * vertices.size(), vertices.data(), hint);
            }
        }
        ImGui::TreePop();
    }
}