#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <memory>
#include <string>

#include <glm/mat4x4.hpp>

class ViewerApp;

class AbstractSample {

public:
    AbstractSample(const std::string &name) : m_name(name) {}
    virtual ~AbstractSample() {}
    virtual bool setup() = 0;
    virtual void teardown() = 0;
    virtual void render(const glm::mat4 &mvp) = 0;
    virtual void renderVertices(const glm::mat4 &mvp) = 0;
    virtual void renderTriangles(const glm::mat4 &mvp) = 0;
    virtual void renderUI() = 0;

    const std::string &name() const {
        return m_name;
    }

private:
    std::string m_name;
};

#endif // EXAMPLE_H