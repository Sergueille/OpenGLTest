#include "Laser.h"
#include "RessourceManager.h"
#include "CircleCollider.h"
#include "EventManager.h"

Laser::Laser() : EditorObject(vec3(0, 0, 0))
{
	if (Editor::enabled)
	{
		editorSprite = new Sprite(RessourceManager::GetTexture("engine\\circle.png"),
			vec3(0), vec2(1), 0, vec4(1, 0, 0, 0.5));
		editorSprite->DrawOnMainLoop();
	}

	displaySprite = new Sprite(nullptr, vec3(0), vec2(1), 0, vec4(1, 0, 0, 1));
	displaySprite->shader = &RessourceManager::shaders["laser"];
	displaySprite->setUniforms = &SetSpriteUniforms;
	displaySprite->setUniformsObjectCall = this;
	displaySprite->DrawOnMainLoop();

	clickCollider = new CircleCollider(vec3(0), 1, false);

	SubscribeToFuncs();

	typeName = "Laser";

	SetType(laserType);
}

Laser::~Laser()
{
	delete displaySprite;
	displaySprite = nullptr;

	if (editorSprite != nullptr)
	{
		delete editorSprite;
		editorSprite = nullptr;
	}

	EventManager::OnMainLoop.remove(mainLoopFuncPos);
}

void Laser::UpdateTransform()
{
	EditorObject::UpdateTransform();

	if (editorSprite != nullptr)
	{
		editorSprite->position = editorPosition + vec3(0, 0, 1); // Bigger Z to render above laser
	}
}

void Laser::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("orientation", std::to_string(editorRotation));
	EditorSaveManager::WriteProp("laserType", (int)laserType);
	EditorSaveManager::WriteProp("startOffset", std::to_string(startOffset));
	EditorSaveManager::WriteProp("endOffset", std::to_string(endOffset));
}

void Laser::Load(std::map<std::string, std::string>* props)
{
	EditorSaveManager::FloatProp(props, "orientation", &editorRotation);

	if ((*props)["laserType"] != "")
		laserType = (LaserType)std::stoi((*props)["laserType"]);
	SetType(laserType);

	EditorSaveManager::FloatProp(props, "startOffset", &startOffset);
	EditorSaveManager::FloatProp(props, "endOffset", &endOffset);

	EditorObject::Load(props);
}

void Laser::Enable()
{
	EditorObject::Enable();
	displaySprite->DrawOnMainLoop();

	if (editorSprite != nullptr)
		editorSprite->DrawOnMainLoop();
}

void Laser::Disable()
{
	EditorObject::Disable();
	displaySprite->StopDrawing();

	if (editorSprite != nullptr)
		editorSprite->StopDrawing();
}

Laser* Laser::Copy()
{
	Laser* copy = new Laser(*this);

	if (this->editorSprite != nullptr)
	{
		copy->editorSprite = this->editorSprite->Copy();
	}

	copy->displaySprite = this->displaySprite->Copy();

	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	copy->clickCollider = new CircleCollider(oldCollider->position, oldCollider->size, false);

	copy->SubscribeToFuncs();

	return copy;
}

vec2 Laser::DrawProperties(vec3 startPos)
{
	std::string strID = std::to_string(ID);
	vec3 drawPos = startPos;

	drawPos.y -= EditorObject::DrawProperties(startPos).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &this->editorRotation, Editor::panelPropertiesX, strID + "orientation").y;

	std::string types[] = { "No teleportation", "Disable teleportation" };
	int intType = (int)laserType;
	drawPos.y -= Editor::OprionProp(drawPos, "Type", &intType, (int)LaserType::lastValue, &types[0], Editor::panelPropertiesX).y;
	SetType((LaserType)intType);

	drawPos.y -= Editor::DrawProperty(drawPos, "Start offset", &startOffset, Editor::panelPropertiesX, strID + "startOffset").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "End offset", &endOffset, Editor::panelPropertiesX, strID + "endOffset").y;

	UpdateTransform();

	vec2 res = vec2(drawPos - startPos);
	res.y *= -1;
	return res;
}

void Laser::SetType(LaserType newType)
{
	laserType = newType;
	LaserSharedProps newProps = props[(int)laserType];
	displaySprite->color = newProps.centerColor;

	if (editorSprite != nullptr)
		editorSprite->color = vec4(newProps.centerColor.x, newProps.centerColor.y, newProps.centerColor.z, editorSpriteAlpha);
}

void Laser::OnMainLoop()
{
	if (enabled)
	{
		vec2 raycastRes;
		vec2 direction = Utility::Rotate(vec2(1, 0), editorRotation);
		if (Collider::Raycast(editorPosition, direction, &raycastRes))
		{
			vec2 spriteStart = vec2(editorPosition) + (direction * startOffset);
			vec2 spriteEnd = raycastRes + (direction * endOffset);

			vec2 middle = (spriteStart + spriteEnd) / 2.f;
			float length = glm::length(spriteStart - spriteEnd);

			displaySprite->position = vec3(middle.x, middle.y, editorPosition.z);
			displaySprite->size = vec2(length, width);
			displaySprite->rotate = editorRotation;
		}
	}
}

void Laser::SubscribeToFuncs()
{
	mainLoopFuncPos = EventManager::OnMainLoop.push_end([this] { this->OnMainLoop(); });
}

void Laser::SetSpriteUniforms(Shader* shader, void* object)
{
	Laser* laser = (Laser*)object;

	LaserSharedProps myProps = laser->props[(int)laser->laserType];
	shader->SetUniform("secColor", myProps.borderColor);
}
