#ifndef BP_SHADER_H_
#define BP_SHADER_H_

#include <glad/glad.h>
#include <string>

namespace bp
{
    /*
     * Wrapper class for an OpenGL shader.
     */
    class shader
    {
    public:
        /*
         * Constructor with shader type.
         */
        shader(GLenum type) :
            m_type(type),
            m_handle(0),
            m_have_src(false),
            m_compiled(false)
        {
        }

        /*
         * Constructor with source code and shader type.
         */
        shader(GLenum type, const std::string& src) :
            shader(type)
        {
            m_src = src;
            m_have_src = true;
        }

        /*
         * Destructor
         */
        ~shader()
        {
            if(m_compiled) glDeleteShader(m_handle);
        }

        /*
         * Compile the shader.
         */
        void compile();

        /*
         * Set the source code for the shader.
         */
        void src(const std::string& src)
        {
            m_src = src;
        }

        /*
         * Load shader source code from a file.
         */
        void load_src(const std::string& file);

        /*
         * Shader type, e.g GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.
         */
        GLenum type() const
        {
            return m_type;
        }

        /*
         * ID or handle to OpenGL shader.
         */
        GLuint handle()
        {
            return m_handle;
        }

        /*
         * The loaded source code for the shader.
         */
        const std::string& src() const
        {
            return m_src;
        }

        /*
         * The information log from compiling the shader.
         */
        const std::string& log() const
        {
            return m_log;
        }

        /*
         * Returns true if the shader have been compiled, false otherwise.
         */
        bool is_compiled() const
        {
            return m_compiled;
        }

    private:
        GLenum m_type;
        GLuint m_handle;

        std::string m_src;
        std::string m_log;

        bool m_have_src;
        bool m_compiled;

        /*
         * Get the infolog (implementation).
         */
        void get_log();
    };
}

#endif
