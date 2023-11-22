#include "Texture.h"

#include <cmath>

#include <glm/vec4.hpp>
#include <imgui.h>
#include <implot.h>
#include <SDL2/SDL_image.h>

Texture::Texture(const std::string &filePath) : AbstractGPUObject(filePath), filePath(filePath)
{
    glGenTextures(1, &handle);
}

Texture::~Texture()
{
    glDeleteTextures(1, &handle);
    handle = 0;
}

int Texture::decode()
{
    SDL_Surface* surface = IMG_Load(filePath.c_str());
    if(surface == NULL)
    {
        SDL_LogCritical(0, "Unable to load image %s: %s", filePath.c_str(), IMG_GetError());
        return -1;
    }

    SDL_Surface *surfaceRGBA = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ABGR8888, 0);
    if (surfaceRGBA == NULL)
    {
        SDL_LogCritical(0, "Unable to load convert %s to RGBA: %s", filePath.c_str(), SDL_GetError());
        SDL_FreeSurface(surface);
        return -1;
    }

    width = surfaceRGBA->w;
    height = surfaceRGBA->h;

    // pre-multiply alpha before uploading to the GPU.
    SDL_LockSurface(surfaceRGBA);
    for (auto y = 0; y < height; ++y) {
        for (auto x = 0; x < width; ++x) {
            glm::u8vec4 &pixel = reinterpret_cast<glm::u8vec4 *>(surfaceRGBA->pixels)[y * width + x];
            pixel.r = (uint8_t)(pixel.r * pixel.a / 255.0f);
            pixel.g = (uint8_t)(pixel.g * pixel.a / 255.0f);
            pixel.b = (uint8_t)(pixel.b * pixel.a / 255.0f);
        }
    }
    SDL_UnlockSurface(surfaceRGBA);

    bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaceRGBA->w, surfaceRGBA->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surfaceRGBA->pixels);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    SDL_FreeSurface(surfaceRGBA);
    SDL_FreeSurface(surface);

    return 0;
}

void Texture::setFiltering(Filtering filtering) {
    bind(0);
    if (this->filtering != filtering) {

        switch(filtering) {
            case NoFiltering:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                break;
            case BilinearFiltering:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                break;
            case TrilinearFiltering:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                break;
            default:
                break;
        }
        this->filtering = filtering;
    }
}

size_t Texture::getMemoryUsage() const {

    size_t result = 0;

    int length = std::max(width, height);
    int lengthPOT = std::pow(2, std::ceil(std::log(length)/log(2)));

    while(lengthPOT > 0) {
        result += lengthPOT * lengthPOT * 4; // RGBA
        lengthPOT /= 2;
    }

    return result;
}

void Texture::renderUI() {
    if(ImGui::TreeNode("Texture")) {
        if (ImGui::BeginTable(name.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {

            char title[32];

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(50);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Type");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            snprintf(title, sizeof(title), "2D Texture");
            ImGui::InputText("##type", title, sizeof(title), ImGuiInputTextFlags_ReadOnly);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(50);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Width");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            snprintf(title, sizeof(title), "%d", width);
            ImGui::InputText("##width", title, sizeof(title), ImGuiInputTextFlags_ReadOnly);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(50);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Height");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            snprintf(title, sizeof(title), "%d", height);
            ImGui::InputText("##height", title, sizeof(title), ImGuiInputTextFlags_ReadOnly);
 
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(50);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Format");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            snprintf(title, sizeof(title), "RGBA");
            ImGui::InputText("##format", title, sizeof(title), ImGuiInputTextFlags_ReadOnly);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(50);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Mipmap");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            snprintf(title, sizeof(title), "Generated");
            ImGui::InputText("##mipmap", title, sizeof(title), ImGuiInputTextFlags_ReadOnly);

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(50);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Filtering");

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
            Filtering newFiltering = filtering;
            const char* items[] = { "No Filtering", "Bi-linear Filtering", "Tri-linear Filtering"  };
            if (ImGui::Combo("##filtering", (int*)&newFiltering, items, IM_ARRAYSIZE(items))) {
                setFiltering(newFiltering);
            }

            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(50);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Pixels");

            static ImVec2 bmin(0,0);
            static ImVec2 bmax(1.0,1.0);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(512);
            if (ImPlot::BeginPlot("##imagePlot")) {
                ImPlot::PlotImage("##image", (ImTextureID)(size_t)handle, bmin, bmax);
                ImPlot::EndPlot();
            }

            ImGui::EndTable();
        }
        ImGui::TreePop();
    }

}