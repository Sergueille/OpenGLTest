#include "TextManager.h"

#include "RessourceManager.h"
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
        FT_Set_Pixel_Sizes(face, 0, 64);

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

            // now store character for later use
            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                face->glyph->advance.x
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
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
        scale /= 64;

        Shader* shader = &RessourceManager::shaders["text"];
        shader->Use();

        shader->SetUniform("projection", glm::ortho(0.0f, 800.0f, 0.0f, 600.0f));

        glUniform3f(glGetUniformLocation(shader->ID, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        float currentX = x;
        float currentY = y;

        // iterate through all characters
        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            if (*c == '\n') 
            {
                // advance cursors
                currentY -= 64 * scale;
                currentX = x;
            }
            else
            {
                Character ch = characters[*c];

                float xpos = currentX + ch.bearing.x * scale;
                float ypos = currentY - (ch.size.y - ch.bearing.y) * scale;
                float w = ch.size.x * scale;
                float h = ch.size.y * scale;

                // update VBO for each character
                float vertices[6][4] = {
                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos,     ypos,       0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },

                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 0.0f }
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
                currentX += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
            }
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
