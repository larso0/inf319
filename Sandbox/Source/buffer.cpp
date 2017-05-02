#include <bp/buffer.h>
#include <stdexcept>

namespace bp
{
    void buffer_object::bind(GLenum target)
    {
        if(!m_init)
        {
            if(m_size == 0)
            {
                throw std::runtime_error("Buffer size can't be 0.");
            }
            glGenBuffers(1, &m_handle);
            glBindBuffer(target, m_handle);
            glBufferData(target, m_size, m_buffer_data, m_usage);
            m_buffer_size = m_size;
            m_init = true;
        }
        else
        {
            glBindBuffer(target, m_handle);
            if(m_dirty)
            {
                if(m_size <= m_buffer_size)
                {
                    glBufferSubData(target, 0, m_size, m_buffer_data);
                }
                else
                {
                    glBufferData(target, m_size, m_buffer_data, m_usage);
                }
                m_dirty = false;
            }
        }
    }

    void buffer_object::buffer_data(void* data, unsigned size)
    {
        if(size == 0)
        {
            throw std::invalid_argument("Size has to be larger than 0.");
        }
        m_buffer_data = data;
        m_size = size;
    }
}
