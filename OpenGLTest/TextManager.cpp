#include "TextManager.h"

#include "Utility.h"
#include "RessourceManager.h"
#include "Camera.h"
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H 

namespace TextManager {
    std::map<char, Character> characters;
    unsigned int VAO, VBO;

    int TextManager::Init() {
        // Init library
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
        {
            std::cout << "Could not init FreeType Library" << std::endl;
            return -1;
        }

        // Load font
        FT_Face face;
        if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
        {
            std::cout << "Failed to load font" << std::endl;
            return -1;
        }

        // Set font resolution
        FT_Set_Pixel_Sizes(face, 0, 64); // TEST

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

        // Load chars
        for (unsigned char c = 0; c < 128; c++)
        {
            // load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "Failed to load Glyph " << c << std::endl;
                continue;
            }

            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // store character
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                (unsigned int)face->glyph->advance.x
            };

            characters.insert(std::pair<char, Character>(c, character));
        }

        // Free ressources
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        // Load text shader
        RessourceManager::LoadShader("Shaders\\FontVtex.glsl", "Shaders\\FontFrag.glsl", "text");

        // Create VBO for dynamic mesh
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        Utility::GlBindVtexArrayOptimised(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 5, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return 0;
    }

    glm::vec2 RenderText(std::string text, glm::vec3 pos, float scale, text_align align, glm::vec3 color) {
        scale /= 64;

        Shader* shader = &RessourceManager::shaders["text"];
        shader->Use();

        shader->SetUniform("projection", Camera::GetUIProjection());

        glUniform3f(glGetUniformLocation(shader->ID, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        Utility::GlBindVtexArrayOptimised(VAO);

        if (align == left)
        {
            pos.x -= GetRect(text, scale).x * 64;
        }
        else if (align == center)
        {
            pos.x -= GetRect(text, scale).x * 32;
        }

        float currentX = pos.x;
        float currentY = pos.y;

        // iterate through all characters
        for (auto c = text.begin(); c != text.end(); c++)
        {
            DrawChar(*c, &currentX, &currentY, scale, pos);
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        return glm::vec2(currentX - pos.x, pos.y - currentY + scale * 64.f) ;
    }

    void DrawChar(char c, float* currentX, float* currentY, float scale, glm::vec3 pos)
    {
        if (c == '\n')
        {
            // advance cursors
            *currentY -= 64 * scale;
            *currentX = pos.x;
        }
        else
        {
            Character ch = characters[c];

            float xpos = *currentX + ch.bearing.x * scale;
            float ypos = *currentY - (ch.size.y - ch.bearing.y) * scale;
            float w = ch.size.x * scale;
            float h = ch.size.y * scale;

            // update VBO for each character
            float vertices[6][5] = {
                { xpos,     ypos + h, pos.z,   0.0f, 0.0f },
                { xpos,     ypos,     pos.z,   0.0f, 1.0f },
                { xpos + w, ypos,     pos.z,   1.0f, 1.0f },

                { xpos,     ypos + h, pos.z,   0.0f, 0.0f },
                { xpos + w, ypos,     pos.z,   1.0f, 1.0f },
                { xpos + w, ypos + h, pos.z,   1.0f, 0.0f }
            };

            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            // update content of VBO memory
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // advance cursors
            *currentX += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
    }

    glm::vec2 GetRect(std::string text, float size)
    {
        float X = 0;
        float maxX = 0;
        float Y = size * 64 * 64;

        // iterate through all characters
        for (auto c = text.begin(); c != text.end(); c++)
        {
            Character ch = characters[*c];

            if (*c == '\n')
            {
                X = 0;
                Y += size * 64 * 64;
            }
            else
            {
                X += ch.advance * size;
                maxX = X;
            }
        }

        return glm::vec2(maxX, Y) / 64.f / 64.f;
    }
}
