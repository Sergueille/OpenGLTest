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
    /// <param name="x">Screen position x, from left</param>
    /// <param name="y">Screen position y, from bottom</param>
    /// <param name="scale">The size of the text in pixels</param>
    /// <param name="color">The color of the text</param>
    void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1, 1, 1));
}

#endif