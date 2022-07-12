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

    extern std::map<int, Character> characters;
    extern std::map<int, Character> charactersMono;
    extern unsigned int VAO, VBO;

    /// <summary>
    /// Init FreeType library and precompute glyphs bitmaps
    /// </summary>
	int Init();

    void LoadFont(FT_Library lib, const char* fontName, std::map<int, Character>* map);

    /// <summary>
    /// Render text on screen
    /// </summary>
    /// <param name="text">The text to display</param>
    /// <param name="x">Screen position, from bottom left</param>
    /// <param name="scale">The size of the text in pixels</param>
    /// <param name="color">The color of the text</param>
    /// <returns>The size of the text area</returns>
    glm::vec2 RenderText(std::string text, glm::vec3 pos, float scale, text_align align, glm::vec4 color, bool mono = false);
    glm::vec2 RenderText(std::string text, glm::vec3 pos, float scale, text_align align, glm::vec3 color, bool mono = false);
    glm::vec2 RenderText(std::string text, glm::vec3 pos, float scale, text_align align = right);

    /// <summary>
    /// INTERNAL - Draw a char
    /// </summary>
    /// <param name="c"></param>
    /// <param name="currentX"></param>
    /// <param name="currentY"></param>
    /// <param name="scale"></param>
    /// <param name="pos"></param>
    void DrawChar(int c, float* currentX, float* currentY, float scale, glm::vec3 pos, bool mono = false);

    /// <summary>
    /// Get the size of the text
    /// </summary>
    glm::vec2 GetRect(std::string text, float size, bool mono = false);
}

#endif