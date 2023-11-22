#ifndef ABSTRACT_GPU_OBJECT_H
#define ABSTRACT_GPU_OBJECT_H

#include <cassert>
#include <string>
#include <map>


struct AbstractGPUObject {

    static std::map<std::string, AbstractGPUObject*> registry;

    std::string name;

    AbstractGPUObject(const std::string &name);
    virtual ~AbstractGPUObject();

    std::string getPrintableMemoryUsage() const;

    virtual size_t getMemoryUsage() const = 0;
    virtual void renderUI() = 0;

};


#endif // ABSTRACT_GPU_OBJECT_H