#include "ResourcedComponent.h"

#include "Application.h"


void ResourcedComponent::LinkResource(std::string resName)
{
	UnLinkResource();
	resource = App->resources->LinkResource(resName, GetType());
}

void ResourcedComponent::UnLinkResource()
{
	if (resource != 0)
	{
		App->resources->UnlinkResource(resource);
		resource = 0;
	}
}
