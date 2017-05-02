#include <bp/shader.h>
#include <fstream>
#include <stdexcept>

using namespace std;

namespace bp
{
    void shader::compile()
    {
        if(!m_have_src)
        throw std::runtime_error("No source code to compile.");
        m_handle = glCreateShader(m_type);
        const GLchar* csrc = m_src.c_str();
        glShaderSource(m_handle, 1, &csrc, nullptr);
        glCompileShader(m_handle);
        GLint compile_status = 0;
        glGetShaderiv(m_handle, GL_COMPILE_STATUS, &compile_status);
        get_log();
        if(compile_status == GL_FALSE)
        {
            glDeleteShader(m_handle);
            throw runtime_error("Unable to compile shader: " + m_log);
        }
        m_compiled = true;
    }

    void shader::load_src(const string& file)
    {
        ifstream srcfile(file);
        if(!srcfile.is_open())
        throw runtime_error("Could not open '" + file + "'.");
        srcfile.seekg(0, std::ios::end);
        m_src.reserve(srcfile.tellg());
        srcfile.seekg(0, std::ios::beg);
        m_src.assign((istreambuf_iterator<char>(srcfile)),
                     istreambuf_iterator<char>());
        m_have_src = true;
    }

    void shader::get_log()
    {
        GLint len = 0;
        glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &len);
        GLchar* cstr = new GLchar[len];
        glGetShaderInfoLog(m_handle, len, &len, cstr);
        m_log = cstr;
        delete[] cstr;
    }
}
