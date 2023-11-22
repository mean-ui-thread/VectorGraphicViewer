#include "VertexBuffer.h"


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