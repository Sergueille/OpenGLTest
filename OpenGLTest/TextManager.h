#ifndef TEXT_MANAGER
#define TEXT_MANAGER

#include <string>
#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include <map>
#include FT_FREETYPE_H 

namespace TextManager{
    enum text_align {left, center, right};

    struct Character {
        unsigned int textureID = -1; // ID handle of the glyph texture
        glm::ivec2 size; // Size of glyph
        glm::ivec2 bearing; // Offset from baseline to left/top of glyph
        unsigned int advance = 0; // Offset to advance to next glyph
    };

    extern std::map<char, Character> characters;
    extern unsigned int VAO, VBO;

    /// <summary>
    /// Init FreeType library and precompute glyphs bitmaps
    /// </summary>
	int Init();

    /// <summary>
    /// Render text on screen
    /// </summary>
    /// <param name="text">The text to display</param>
    /// <param name="x">Screen position, from bottom left</param>
    /// <param name="scale">The size of the text in pixels</param>
    /// <param name="color">The color of the text</param>
    /// <returns>The size of the text area</returns>
    glm::vec2 RenderText(std::string text, glm::vec3 pos, float scale, text_align align = right, glm::vec3 color = glm::vec3(1, 1, 1));

    /// <summary>
    /// INTERNAL - Draw a char
    /// </summary>
    /// <param name="c"></param>
    /// <param name="currentX"></param>
    /// <param name="currentY"></param>
    /// <param name="scale"></param>
    /// <param name="pos"></param>
    void DrawChar(char c, float* currentX, float* currentY, float scale, glm::vec3 pos);

    /// <summary>
    /// Get the size of the text
    /// </summary>
    glm::vec2 GetRect(std::string text, float size);
}

#endif