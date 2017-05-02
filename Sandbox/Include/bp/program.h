#ifndef BP_PROGRAM_H_
#define BP_PROGRAM_H_

#include "shader.h"

namespace bp
{
    /*
     * Wrapper class for an OpenGL program.
     */
    class program
    {
    public:
        /*
         * Default constructor
         */
        program() :
            m_handle(0),
            m_handle_created(false),
            m_linked(false)
        {
        }

        /*
         * Destructor
         */
        ~program()
        {
            if(m_linked) glDeleteProgram(m_handle);
        }

        /*
         * Attach a shader to the program based on the shader wrapper class.
         */
        void attach(shader& s);

        /*
         * Attach a shader to the program based on an OpenGL shader handle.
         */
        void attach(GLuint s);

        /*
         * Link the program.
         */
        void link();

        /*
         * Use the program.
         */
        void use()
        {
            if(!m_linked) link();
            glUseProgram(m_handle);
        }

        /*
         * Get an attribute location from an attribute name.
         */
        GLint attribute(const std::string& name);

        /*
         * Get a uniform location from a uniform name.
         */
        GLint uniform(const std::string& name);

        /*
         * ID or handle to OpenGL program.
         */
        GLuint handle()
        {
            return m_handle;
        }

        /*
         * The information log from linking the program.
         */
        const std::string& log() const
        {
            return m_log;
        }

        /*
         * Returns true if the program have been linked, false otherwise.
         */
        bool is_linked() const
        {
            return m_linked;
        }

    private:
        GLuint m_handle;

        bool m_handle_created;
        bool m_linked;

        std::string m_log;

        /*
         * Get the infolog (implementation).
         */
        void get_log();
    };
}

#endif
