#include "AbstractGPUObject.h"

#include <string>
#include <vector>

std::map<std::string, AbstractGPUObject*> AbstractGPUObject::registry;

AbstractGPUObject::AbstractGPUObject(const std::string &name) 
    : name(name) 
{
	if (name[0] != '#') {
		printf("Registering '%s'\n", name.c_str());
		assert(registry.find(name) == registry.end());
		registry[name] = this;
	}
}

AbstractGPUObject::~AbstractGPUObject() {
	if (name[0] != '#') {
		printf("Unregistering '%s'\n", name.c_str());
	    int count = registry.erase(name);
		assert(count == 1);
	}
}

std::string AbstractGPUObject::getPrintableMemoryUsage() const
{
	static std::vector<std::string> suffix = {"B", "KB", "MB", "GB", "TB"};

	int i = 0;

    size_t bytes = getMemoryUsage();

	double dblBytes = bytes;

	if (bytes > 1024) {
		for (i = 0; (bytes / 1024) > 0 && i < suffix.size()-1; ++i, bytes /= 1024) {
            dblBytes = bytes / 1024.0;
        }
	}

	static char output[128];
	if (i == 0) {
		sprintf(output, "%.0f B", dblBytes);
	} else {
		sprintf(output, "%.02lf %s", dblBytes, suffix[i].c_str());
	}
	return output;
}