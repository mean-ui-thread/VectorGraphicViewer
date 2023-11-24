#ifndef GPU_OBJECT_REGISTRY
#define GPU_OBJECT_REGISTRY

struct AbstractGPUObject;

bool registerGPUObject(AbstractGPUObject* gpuObject);
bool unregisterGPUObject(AbstractGPUObject* gpuObject);

#endif // GPU_OBJECT_REGISTRY
