#ifndef BP_BUFFER_H_
#define BP_BUFFER_H_

#include <vector>
#include <glad/glad.h>

namespace bp
{
    /*
     * Wrapper for an OpenGL buffer object.
     */
    class buffer_object
    {
    public:
        /*
         * Default constructor.
         * The default buffer usage is GL_STATIC_DRAW.
         */
        buffer_object() :
            m_handle(0),
            m_usage(GL_STATIC_DRAW),
            m_buffer_data(nullptr),
            m_size(0),
            m_buffer_size(0),
            m_init(false),
            m_dirty(false)
        {}

        /*
         * Constructor to set buffer usage explicitly.
         */
        buffer_object(GLenum usage) :
            m_handle(0),
            m_usage(usage),
            m_buffer_data(nullptr),
            m_size(0),
            m_buffer_size(0),
            m_init(false),
            m_dirty(false)
        {}

        /*
         * Destructor.
         */
        virtual ~buffer_object()
        {
            if(m_init)
            {
                glDeleteBuffers(1, &m_handle);
            }
        }

        /*
         * Bind the buffer to the given target.
         * Will initialize the buffer with the initial data if not already
         * initialized.
         * Will reupload data if dirty has been called.
         */
        virtual void bind(GLenum target);

		/*
         * Set the data that should be put in the buffer.
         */
        void buffer_data(void* data, unsigned size);

        /*
         * Get the handle to the OpenGL buffer object.
         */
        GLuint handle()
        {
            return m_handle;
        }

        /*
         * Set the buffer usage.
         * The usage must be set before binding the buffer.
         */
        void usage(GLenum usage)
        {
            m_usage = usage;
        }

        GLenum usage() const
        {
            return m_usage;
        }

        /*
         * Dirty the buffer so the data is updated the next time it is bound.
         */
        virtual void dirty()
        {
            m_dirty = true;
        }

    protected:
        GLuint m_handle;
        GLenum m_usage;

        void* m_buffer_data;
        unsigned m_size;
        unsigned m_buffer_size;
        bool m_init;
        bool m_dirty;
    };

    /*
     * Combine std::vector and bp::buffer_object.
     */
    template <typename T>
    class buffer : public buffer_object, public std::vector<T>
    {
    public:
        using std::vector<T>::data;
        using std::vector<T>::size;

        /*
         * Default constructor.
         */
        buffer()
        {}

        /*
         * Constructor to set buffer usage explicitly.
         */
        buffer(GLenum usage) :
            buffer_object(usage)
        {}

        /*
         * Bind the buffer to the given target.
         * Overrides buffer_object::bind.
         */
        void bind(GLenum target) override
        {
            if(!m_init)
            {
                buffer_data(data(), size()*sizeof(T));
            }
            buffer_object::bind(target);
        }

        /*
         * Dirty the buffer so the data is updated the next time it is bound.
         * Overrides buffer_object::dirty.
         */
        void dirty() override
        {
            buffer_data(data(), size()*sizeof(T));
            buffer_object::dirty();
        }
    };
}

#endif
