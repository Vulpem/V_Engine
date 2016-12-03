#ifndef  __R_RESOURCE__
#define __R_RESOURCE__

#include "Globals.h"
#include "Application.h"
#include "Component.h"

class Resource
{
public:
	Resource() { uid = GenerateUUID(); }
	Resource(uint64_t UID) { this->uid = UID; }

	virtual ~Resource() { }

	//Number of references to this resource. Won't unload while it's over 0
	uint nReferences = 0;

	//Unique ID for this resource
	uint64_t uid = 0;

	//Filename this resource extraced the data from
	std::string name;

	virtual Component::Type GetType() = 0;

	template <typename T>
	T* Read() { return (T*)this; }
};

#endif // ! __R_RESOURCE__