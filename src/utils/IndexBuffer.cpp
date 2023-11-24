#include "IndexBuffer.h"

#include <imgui.h>

void IndexBuffer::renderUI() {

    if(ImGui::TreeNode("Index Buffer Object")) {
        if (ImGui::BeginTable(name.c_str(), 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
        {
            for (size_t i = 0; i < indices.size(); i++)
            {
                int value = (int)indices[i];
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::PushID(i);
                if(ImGui::InputInt("##index", &value)) {
                    indices[i] = (uint16_t)value;
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(), indices.data(), hint);
                }
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::TreePop();
    }
}