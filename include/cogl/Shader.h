//
// Created by ekin4 on 26/02/2017.
//

#ifndef IDEK_SHADER_H
#define IDEK_SHADER_H

#include "Constants.h"

namespace cogl {
    class Shader {
    public:
        // Store the programID
        GLuint Program = 0;
        // "Load and Compile" Constructors
        Shader() = default;

        explicit Shader(const char *vertex_file_path, const char *fragment_file_path);

        explicit Shader(const char *shader_file_path);

        Shader(std::string vertex_file_path, std::string fragment_file_path);

        explicit Shader(std::string shader_file_path);

        //Move Constructor
        Shader(Shader &&other);

        // Move Assignment
        Shader &operator=(Shader &&other);

        // Disallow Copy Constructor
        Shader(const Shader &) = delete;

        // Disallow Copy Assignment
        Shader &operator=(const Shader &) = delete;

        //Destructor
        ~Shader();

        void release();

        void loadAndCompile(const char *vertex_file_path, const char *fragment_file_path);

		Shader shaderFromString(std::string vertex_shader, std::string fragment_shader);

        // Enable the use of the shader
        void bind() const;

        // Disable shader
        static void unbind() {
            glUseProgram(0);
        }

        // Get uniform location
        GLint getUniformLoc(const char *uniformName) const;

        // Get attribute location
        GLint getAttribLoc(const char *attribName) const;
    };

}
#endif //IDEK_SHADER_H
