#include "ResourcedComponent.h"

#include "Application.h"

void ResourcedComponent::LinkResource(std::string fileName)
{
	UnLinkResource();
	resource = App->resources->LinkResource(fileName, GetType());
}

void ResourcedComponent::UnLinkResource()
{
	if (resource != 0)
	{
		App->resources->UnlinkResource(resource);
		resource = 0;
	}
}
