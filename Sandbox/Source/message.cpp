#include <bp/message.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace bp
{
    message_stream info("Information", &std::cout);
    message_stream debug("Debug information", &std::cout);
    message_stream warning("Warning", &std::cerr);
    message_stream error("Error", &std::cerr);

    void message_stream::push()
    {
        switch(m_dst)
        {
        case MESSAGE_DESTINATION_STREAM:
            if(!m_stream)
            throw std::runtime_error("Message output stream was nullptr.");
            if(m_title != "") (*m_stream) << m_title << ':' << std::endl;
            (*m_stream) << str() << std::endl;
            str("");
            break;
        case MESSAGE_DESTINATION_LOGFILE:
            {
                std::ofstream out(m_logfile, std::ofstream::app);
                if(!out.good())
                    throw std::runtime_error("Could not open logfile '" +
                                             m_logfile
                                             + "'.");
                if(m_title != "") out << m_title << ':' << std::endl;
                out << str() << std::endl;
            }
            break;
        }
    }

    std::ostream& operator <<(std::ostream& s, message_stream::operation op)
    {
        switch(op)
        {
        case message_stream::operation::PUSH:
            ((message_stream&)s).push();
            break;
        }
        return s;
    }
}
