#include "Material.h"

#include "GameObject.h"

Material::Material(GameObject* linkedTo, int id) :Component(linkedTo, id)
{
	char tmp[NAME_MAX_LEN];
	sprintf(tmp, "Material##%i", id);
	name = tmp;
	type = C_transform;
}
Material::~Material()
{

}