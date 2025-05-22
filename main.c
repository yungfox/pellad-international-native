#include <stdio.h>
#include <wchar.h>
#include <stdbool.h>

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <windows.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include "include/glad/glad.h"

#define RGFW_IMPLEMENTATION
#define RGFW_NO_IOKIT
#include "include/RGFW.h"

typedef struct {
    float x, y;
} Vec2f;

typedef struct {
    int x, y;
} Vec2i;

typedef struct {
    float x, y, z, w;
} Vec4f;

#define CHARS_DEFAULT_CAPACITY 64
#define OPENGL_ERROR_MESSAGE_BUF_SIZE 512
#define FONT_LOAD_FLAGS FT_LOAD_RENDER | FT_LOAD_TARGET_(FT_RENDER_MODE_SDF)
#define WIDTH 800
#define HEIGHT 600
#define BG_COLOR_BASE 23.0
#define BACKGROUND_COLOR BG_COLOR_BASE / 255.0, BG_COLOR_BASE / 255.0, BG_COLOR_BASE / 255.0, 1.0
#define FOREGROUND_COLOR (Vec4f){ 0.8, 0.8, 0.8, 1.0 }
#define INTERVAL_BETWEEN_CHARACTERS_MS 150
#define IDLE_TIME_MS 2000

typedef struct {
    GLuint texture;
    Vec2i size;
    Vec2i bearing;
    uint32_t advance;
} Glyph;

typedef struct {
    wchar_t code;
    Glyph glyph;
} Character;

typedef struct {
    Character items[CHARS_DEFAULT_CAPACITY];
    size_t count;
} Characters;

typedef struct {
    const wchar_t* text;
    Vec2i size;
    Characters* map;
} Text;

const wchar_t* text_latin = L"PELLAD";
const wchar_t* text_cyrillic = L"ПЕЛЛАД";
const wchar_t* text_katakana = L"ペラド";
// TODO: support arabic text rendering
// const wchar_t* text_arabic = L"بيلاد";

Characters druk_wide_chars = {0};
Characters dela_gothic_one_chars = {0};

GLuint vao;
GLuint vbo;
GLuint program;
GLint color_uniform_location;

char* 
read_file_as_string(const char* file_path)
{
    FILE* f = fopen(file_path, "r");
    if (f == NULL)
        goto error;

    long previous_position = ftell(f);
    if (previous_position < 0)
        goto error;

    if (fseek(f, 0, SEEK_END) < 0)
        goto error;

    long file_size = ftell(f);
    if (file_size < 0)
        goto error;

    if (fseek(f, previous_position, SEEK_SET) < 0)
        goto error;

    char* buffer = (char*)malloc(file_size + 1);
    if (buffer == NULL)
        goto error;

    memset((void*)buffer, 0, sizeof(buffer[0]) * file_size + 1);

    fread(buffer, file_size, 1, f);
    if (ferror(f))
        goto error;
    
    if (f)
        fclose(f);

    return buffer;

error:
    if (f)
        fclose(f);
    fprintf(stderr, "error reading file %s: %s\n", file_path, strerror(errno));
    return NULL;
}

int
characters_get_index_of_code(Characters* map, wchar_t code)
{
    for (size_t i = 0; i < map->count; ++i) {
        if (map->items[i].code == code)
            return (int)i;
    }

    return -1;
}

void
characters_fill_map(Characters* map, FT_Face face, const wchar_t* text)
{
    for (size_t i = 0; i < wcslen(text); ++i) {
        if (characters_get_index_of_code(map, text[i]) >= 0)
            continue;
        
        FT_Error error = FT_Load_Char(face, text[i], FONT_LOAD_FLAGS);
        if (error) {
            fprintf(stderr, "failed to load glyph with code %zu: %s\n", i, FT_Error_String(error));
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0,
                     GL_RED,
                     GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        map->items[map->count] = (Character){
            .code = text[i],
            .glyph =  (Glyph){
                .texture = texture,
                .size = (Vec2i){ .x = face->glyph->bitmap.width, .y = face->glyph->bitmap.rows},
                .bearing = (Vec2i){ .x = face->glyph->bitmap_left, .y = face->glyph->bitmap_top},
                .advance = face->glyph->advance.x
            }
        };

        map->count++;
    }
}

Vec2i
measure_text(Text* t)
{
    Vec2i res = {0};

    for (size_t i = 0; i < wcslen(t->text); ++i) {
        int idx = characters_get_index_of_code(t->map, t->text[i]);
        if (idx < 0)
            continue;
        Glyph ch = t->map->items[idx].glyph;

        res.x += ch.size.x;
        if (res.y < ch.size.y)
            res.y = ch.size.y;
    }

    return res;
}

void 
render_text(Text* t, size_t count, Vec2f pos, float scale, Vec4f color)
{
    glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);

    for (size_t i = 0; i < count; ++i) {
        int idx = characters_get_index_of_code(t->map, t->text[i]);
        if (idx < 0)
            continue;
        Glyph ch = t->map->items[idx].glyph;

        float xpos = pos.x + ch.bearing.x * scale;
        float ypos = pos.y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        pos.x += (ch.advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Text
text_new(Characters* map, const wchar_t* text)
{
    Text new = (Text){
        .map = map,
        .text = text,
    };
    new.size = measure_text(&new);

    return new;
}

int
main(void)
{   
    const char* frag_file_path = "./shaders/texture.frag";
    const char* vert_file_path = "./shaders/texture.vert";

    char* frag_src = read_file_as_string(frag_file_path);
    if (frag_src == NULL)
        return 1;

    char* vert_src = read_file_as_string(vert_file_path);
    if (vert_src == NULL)
        return 1;

    RGFW_setGLVersion(RGFW_glCore, 3, 3);

    RGFW_window* window = RGFW_createWindow("PELLAD INTERNATIONAL", RGFW_RECT(WIDTH, HEIGHT, WIDTH, HEIGHT), RGFW_windowAllowDND | RGFW_windowCenter);
    if (window == NULL) {
        fprintf(stderr, "failed to create RGFW window\n");
        return 1;
    }
    RGFW_window_makeCurrent(window);

    if (gladLoadGL() == 0) {
        fprintf(stderr, "failed to initialize GLAD\n");
        return 1;
    }

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLchar message[OPENGL_ERROR_MESSAGE_BUF_SIZE];

    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, (const char**)&frag_src, NULL);
    glCompileShader(frag_shader);

    GLint frag_shader_compiled = 0;
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_shader_compiled);

    if (!frag_shader_compiled) {
        glGetShaderInfoLog(frag_shader, sizeof(message), NULL, message);
        fprintf(stderr, "failed to compile fragment shader: %s\n", message);
        return 1;
    }
    
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, (const char**)&vert_src, NULL);
    glCompileShader(vert_shader);
    
    GLint vert_shader_compiled = 0;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_shader_compiled);
    
    if (!vert_shader_compiled) {
        glGetShaderInfoLog(vert_shader, sizeof(message), NULL, message);
        fprintf(stderr, "failed to compile vertex shader: %s\n", message);
        return 1;
    }
    
    program = glCreateProgram();
    
    glAttachShader(program, frag_shader);
    glAttachShader(program, vert_shader);
    glLinkProgram(program);
    
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        glGetProgramInfoLog(program, sizeof(message), NULL, message);
        fprintf(stderr, "failed to link program: %s\n", message);
        return 1;
    }

    glDeleteShader(frag_shader);
    glDeleteShader(vert_shader);

    glUseProgram(program);

    const char* druk_wide_path = "./fonts/Druk-Wide-Cy-Web-Medium-Regular.ttf";
    const char* dela_gothic_one_path = "./fonts/DelaGothicOne-Regular.ttf";

    FT_Library library = {0};
    FT_Error error = FT_Init_FreeType(&library);
    if (error) {
        fprintf(stderr, "failed to initialize freetype2\n");
        return 1;
    }

    FT_Face druk_wide_face, dela_gothic_one_face;
    error = FT_New_Face(library, druk_wide_path, 0, &druk_wide_face);
    if (error) {
        fprintf(stderr, "failed to load face: %s\n", FT_Error_String(error));
        return 1;
    }
    error = FT_New_Face(library, dela_gothic_one_path, 0, &dela_gothic_one_face);
    if (error) {
        fprintf(stderr, "failed to load face: %s\n", FT_Error_String(error));
        return 1;
    }

    FT_UInt pixel_size = 96;
    error = FT_Set_Pixel_Sizes(druk_wide_face, 0, pixel_size);
    if (error) {
        fprintf(stderr, "failed to set pixel size: %s\n", FT_Error_String(error));
        return 1;
    }
    error = FT_Set_Pixel_Sizes(dela_gothic_one_face, 0, pixel_size);
    if (error) {
        fprintf(stderr, "failed to set pixel size: %s\n", FT_Error_String(error));
        return 1;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    characters_fill_map(&druk_wide_chars, druk_wide_face, text_latin);
    characters_fill_map(&druk_wide_chars, druk_wide_face, text_cyrillic);
    characters_fill_map(&dela_gothic_one_chars, dela_gothic_one_face, text_katakana);

    Text latin = text_new(&druk_wide_chars, text_latin);
    Text cyrillic = text_new(&druk_wide_chars, text_cyrillic);
    Text katakana = text_new(&dela_gothic_one_chars, text_katakana);

    Text texts[] = { latin, cyrillic, katakana };
    size_t texts_count = sizeof(texts) / sizeof(texts[0]);

    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glViewport(0, 0, WIDTH, HEIGHT);

    size_t text_index = 0;
    uint64_t now = RGFW_getTimeNS() / 1e6;
    uint64_t previous_digit = now;
    uint64_t time_to_wait_start = 0;
    size_t length = 0;
    bool reached = false;
    bool go_back = false;
    
    while (RGFW_window_shouldClose(window) == 0) {
        RGFW_window_checkEvent(window);
        
        glClearColor(BACKGROUND_COLOR);
        glClear(GL_COLOR_BUFFER_BIT);

        Text* selected = &texts[text_index];

        now = RGFW_getTimeNS() / 1e6;
        if (length < wcslen(selected->text)) {
            if (now - previous_digit >= INTERVAL_BETWEEN_CHARACTERS_MS) {
                previous_digit = now;
                if (go_back) {
                    if (length > 0) {
                        length--;
                    } else {
                        go_back = false;
                        if (text_index < texts_count - 1) {
                            text_index++;
                        } else {
                            text_index = 0;
                        }
                    }
                } else {
                    length++;
                }
            }
        } else {
            if (!reached) {
                time_to_wait_start = now;
                reached = true;
            }

            if (now - time_to_wait_start >= IDLE_TIME_MS) {
                go_back = true;
                reached = false;
                previous_digit = now;
                length--;
            }
        }
        
        Vec2f text_position = (Vec2f){
            .x = -(selected->size.x / 2.0),
            .y = -(selected->size.y / 2.0),
        };
        render_text(selected, length, text_position, 1.0, FOREGROUND_COLOR);
        
        RGFW_window_swapBuffers(window);
        RGFW_window_checkFPS(window, 60);
    }

    FT_Done_Face(druk_wide_face);
    FT_Done_FreeType(library);
    
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);

    RGFW_window_close(window);
    
    return 0;
}
