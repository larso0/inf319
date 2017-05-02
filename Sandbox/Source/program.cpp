#include <bp/program.h>
#include <stdexcept>

using namespace std;

namespace bp
{
    void program::attach(shader& s)
    {
        if(!s.is_compiled()) s.compile();
        attach(s.handle());
    }

    void program::attach(GLuint s)
    {
        if(!m_handle_created)
        {
            m_handle = glCreateProgram();
            m_handle_created = true;
        }
        glGetError();
        glAttachShader(m_handle, s);
        GLenum err = glGetError();
        if(err != GL_NO_ERROR)
            throw runtime_error("Error when attaching shader.");
    }

    void program::link()
    {
        if(!m_handle_created) throw runtime_error("Unable to link program: "
                                                  "No shaders are attached.");
        glLinkProgram(m_handle);
        GLint link_status = 0;
        glGetProgramiv(m_handle, GL_LINK_STATUS, &link_status);
        get_log();
        if(link_status == GL_FALSE)
            throw runtime_error("Unable to link program: " + m_log);
        m_linked = true;
    }

    GLint program::attribute(const std::string& name)
    {
        if(!m_linked) link();
        GLint location = glGetAttribLocation(m_handle, name.c_str());
        if(location == -1)
            throw runtime_error("Unable to get attribute location for \""
                                + name
                                + "\".");
        return location;
    }

    GLint program::uniform(const std::string& name)
    {
        if(!m_linked) link();
        GLint location = glGetUniformLocation(m_handle, name.c_str());
        if(location == -1)
            throw runtime_error("Unable to get uniform location for \""
                                + name
                                + "\".");
        return location;
    }

    void program::get_log()
    {
        GLint len = 0;
        glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &len);
        GLchar* cstr = new GLchar[len];
        glGetProgramInfoLog(m_handle, len, &len, cstr);
        m_log = cstr;
        delete[] cstr;
    }
}
