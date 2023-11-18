#ifndef EXAMPLE_H
#define EXAMPLE_H

#include <memory>
#include <string>

struct ViewerApp;

struct Example {
    std::string name;
    Example(const std::string &name) : name(name) {}
    virtual ~Example() {}
    virtual bool setup() = 0;
    virtual void render(const std::shared_ptr<ViewerApp> &app) = 0;
    virtual void renderUI(const std::shared_ptr<ViewerApp> &app) = 0;
};

#endif // EXAMPLE_H