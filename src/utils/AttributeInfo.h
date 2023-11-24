#ifndef ATTRIBUTEINFO_H
#define ATTRIBUTEINFO_H

#include <cassert>
#include <string>

#include <glad/glad.h>

struct AttributeInfo {

    enum Type {
        Byte          = GL_BYTE,
        UnsignedByte  = GL_UNSIGNED_BYTE,
        Short         = GL_SHORT,
        UnsignedShort = GL_UNSIGNED_SHORT,
        Int           = GL_INT,
        UnsignedInt   = GL_UNSIGNED_INT,
        Float         = GL_FLOAT
    };

    enum Action {
        LeaveAsIs = GL_FALSE,
        Normalize = GL_TRUE
    };

    inline GLint sizeOfType() const {
        switch(type)
        {
        case Byte:          return sizeof(GLbyte);
        case UnsignedByte:  return sizeof(GLubyte);
        case Short:         return sizeof(GLshort);
        case UnsignedShort: return sizeof(GLushort);
        case Int:           return sizeof(GLint);
        case UnsignedInt:   return sizeof(GLuint);
        case Float:         return sizeof(GLfloat);
        }
        assert(false);
        return 0;
    }

    inline GLint size() const {
        return count * sizeOfType();
    }

    std::string name;
    Type type;
    GLint count;
    Action action = LeaveAsIs;
};

#endif // ATTRIBUTEINFO_H
