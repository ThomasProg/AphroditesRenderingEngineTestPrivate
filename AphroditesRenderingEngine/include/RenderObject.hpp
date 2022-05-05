#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Material.hpp"

struct RenderObject 
{
	struct GPUMesh* mesh;

	EngineGraphicsMaterial* material;

	glm::mat4 transformMatrix;
};
