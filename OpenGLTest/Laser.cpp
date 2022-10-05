#include "Laser.h"
#include "RessourceManager.h"
#include "CircleCollider.h"
#include "EventManager.h"
#include "Player.h"

std::list<Laser*> Laser::lasers = std::list<Laser*>();

const ObjectEvent Laser::events[LASER_EVENT_COUNT]{
	ObjectEvent {
		"Turn on",
		[] (EditorObject* object, void* param) {
			((Laser*)object)->TurnOn();
		},
	},
	ObjectEvent {
		"Turn off",
		[] (EditorObject* object, void* param) {
			((Laser*)object)->TurnOff();
		},
	},
	ObjectEvent {
		"Toggle On/Off",
		[](EditorObject* object, void* param) {
			((Laser*)object)->ToggleOnOff();
		},
	},
};

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

	emmiterSprite = new Sprite(RessourceManager::GetTexture("emmiter.png"), vec3(0), vec2(1), 0, vec4(1.3, 1.3, 1.3, 1));
	emmiterSprite->DrawOnMainLoop();
	glowSprite = new Sprite(RessourceManager::GetTexture("glow.png"), vec3(0), vec2(1.54f), 0, vec4(1));
	glowSprite->DrawOnMainLoop();

	clickCollider = new CircleCollider(vec3(0), 1, false);
	laserCollider = new RectCollider(vec2(0), vec2(1), 0, false);

	lasers.push_back(this);
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

	delete emmiterSprite;
	emmiterSprite = nullptr;

	delete glowSprite;
	glowSprite = nullptr;

	delete laserCollider;
	laserCollider = nullptr;

	lasers.remove(this);
	StopLoopSound();
}

void Laser::OnMainLoop()
{
	EditorObject::OnMainLoop();

	if (!enabled) return;

	if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos() + vec3(0, 0, 1); // Bigger Z to render above laser
	}

	vec3 pos = GetEditPos();
	float rot = GetEditRotation();

	vec3 emmiterDelta = vec3(0.025, 0.4, 2);
	vec2 rotatedDelta = Rotate(vec2(emmiterDelta.x, emmiterDelta.y), rot - 90);
	emmiterSprite->rotate = rot - 90;
	emmiterSprite->position = pos + vec3(rotatedDelta.x, rotatedDelta.y, emmiterDelta.z);

	vec3 glowDelta = vec3(0, 0.9, 3);
	rotatedDelta = Rotate(vec2(glowDelta.x, glowDelta.y), rot - 90);
	glowSprite->position = pos + vec3(rotatedDelta.x, rotatedDelta.y, glowDelta.z);

	vec4 colors[2] = { vec4(1, 0.5, 0.9, 1), vec4(1, 0.5, 0.5, 1) };
	glowSprite->color = colors[(int)laserType];

	if (laserOn)
	{
		float playerDist = Player::ingameInstance == nullptr
			? 0
			: glm::length(Player::ingameInstance->GetPos() - vec2(GetEditPos()));

		if (!EditorSaveManager::isLoading && (Editor::enabled || playerDist < minPlayerDist || !hasRefreshedOnce)) // Refresh anyway for the first time to relpace the sprite correctly
		{
			if (displaySprite != nullptr)
				displaySprite->DrawOnMainLoop(); // NOCHECK

			vec2 raycastRes;
			vec2 direction = Utility::Rotate(vec2(1, 0), GetEditRotation());
			if (Collider::Raycast(GetEditPos(), direction, &raycastRes))
			{
				vec2 spriteStart = vec2(GetEditPos()) + (direction * startOffset);
				vec2 spriteEnd = raycastRes + (direction * endOffset);

				vec2 middle = (spriteStart + spriteEnd) / 2.f;
				float length = glm::length(spriteStart - spriteEnd);

				displaySprite->position = vec3(middle.x, middle.y, GetEditPos().z);
				displaySprite->size = vec2(length, width);
				displaySprite->rotate = GetEditRotation();

				laserCollider->SetPos(vec3(middle.x, middle.y, GetEditPos().z));
				laserCollider->size = vec2(length, width);
				laserCollider->orientation = GetEditRotation();

				if (Player::ingameInstance != nullptr && Player::ingameInstance->canTeleport)
				{
					if (laserType == LaserType::noTeleport)
					{
						float la, lb;
						Utility::GetLineEquationFromPoints(spriteStart, spriteEnd, &la, &lb);
						float pa, pb;
						Utility::GetLineEquationFromPoints(Player::ingameInstance->GetPos(), Player::ingameInstance->teleportPosition, &pa, &pb);

						bool res = Utility::SegementIntersection(
							vec2(GetEditPos()) - (direction * 100.0f),
							raycastRes + (direction * 100.0f),
							Player::ingameInstance->GetPos(),
							Player::ingameInstance->teleportPosition,
							&intersectionPos);

						if (!res)
						{
							if (SqrDist(intersectionPos, Player::ingameInstance->GetPos()) <
								SqrDist(intersectionPos, Player::ingameInstance->teleportPosition))
							{
								intersectionPos = Player::ingameInstance->GetPos();
							}
							else
							{
								intersectionPos = Player::ingameInstance->teleportPosition;
							}
						}
					}
					else if (laserType == LaserType::deadlyLaser)
					{
						intersectionPos = Player::ingameInstance->GetPos();

						// Kill player
						vec3 res = Player::ingameInstance->collider->CollideWith(laserCollider);
						if (res.z != 0)
							Player::ingameInstance->Kill();
					}
				}

				hasRefreshedOnce = true;
			}

			// Get distance between camera and line
			vec2 editPos = vec2(GetEditPos());
			vec3 h = Utility::Project(editPos, raycastRes, Camera::position);
			float camDist;

			if (h.z < 0) // editPos is the nearest point
			{
				camDist = glm::distance(Camera::position, editPos);
			}
			else if (h.z > glm::distance(editPos, raycastRes)) // raycastRes is the nearest point
			{
				camDist = glm::distance(Camera::position, raycastRes);
			}
			else // H is the nearest point
			{
				camDist = glm::distance(Camera::position, vec2(h));
			}

			soundsAttenuation = (camDist - soundsMinDist) / (soundsMaxDist - soundsMinDist);
			if (soundsAttenuation > 1) soundsAttenuation = 1;
			if (soundsAttenuation < 0) soundsAttenuation = 0;
			soundsAttenuation = 1 - soundsAttenuation;

			soloud->setVolume(loopSound, Utility::gameSoundsVolume * soundOnVolume * soundsAttenuation);
		}
		else
		{
			soloud->setVolume(loopSound, 0);
		}
	}

	emmiterSprite->isLit = !laserOn;
}

void Laser::Save()
{
	EditorObject::Save();
	EditorSaveManager::WriteProp("orientation", std::to_string(editorRotation));
	EditorSaveManager::WriteProp("laserType", (int)laserType);
	EditorSaveManager::WriteProp("startOffset", std::to_string(startOffset));
	EditorSaveManager::WriteProp("endOffset", std::to_string(endOffset));
	EditorSaveManager::WriteProp("startOn", startOn);
}

void Laser::Load(std::map<std::string, std::string>* props)
{
	EditorSaveManager::FloatProp(props, "orientation", &editorRotation);

	if ((*props)["laserType"] != "")
		laserType = (LaserType)std::stoi((*props)["laserType"]);
	SetType(laserType);

	EditorSaveManager::FloatProp(props, "startOffset", &startOffset);
	EditorSaveManager::FloatProp(props, "endOffset", &endOffset);
	startOn = (*props)["startOn"] != "0";

	if (!Editor::enabled && !startOn) TurnOff();
	else if (startOn) StartLoopSound();

	EditorObject::Load(props);
}

void Laser::Enable()
{
	EditorObject::Enable();
	emmiterSprite->DrawOnMainLoop();

	if (editorSprite != nullptr)
		editorSprite->DrawOnMainLoop();

	laserCollider->enabled = true;

	if (laserOn) 
	{
		displaySprite->DrawOnMainLoop();
		glowSprite->DrawOnMainLoop();
		lasers.push_back(this);
	}
}

void Laser::Disable()
{
	EditorObject::Disable();
	displaySprite->StopDrawing();
	emmiterSprite->StopDrawing();
	glowSprite->StopDrawing();

	if (editorSprite != nullptr)
		editorSprite->StopDrawing();

	laserCollider->enabled = false;

	if (laserOn)
		lasers.remove(this);

	StopLoopSound();
}

void Laser::StartLoopSound()
{
	if (!Editor::enabled && !isPlayingLoopSound)
	{
		isPlayingLoopSound = true;
		loopSound = Utility::PlaySound("laser_on.wav", Utility::gameSoundsVolume * soundOnVolume * soundsAttenuation);
		soloud->setLooping(loopSound, true);
	}
}

void Laser::StopLoopSound()
{
	soloud->stop(loopSound);
	isPlayingLoopSound = false;
}

Laser* Laser::Copy()
{
	Laser* copy = new Laser(*this);

	if (this->editorSprite != nullptr)
	{
		copy->editorSprite = this->editorSprite->Copy();
	}

	copy->displaySprite = this->displaySprite->Copy();
	copy->emmiterSprite = this->emmiterSprite->Copy();
	copy->glowSprite = this->glowSprite->Copy();

	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	copy->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, false);

	copy->laserCollider = new RectCollider(vec2(0), vec2(1), 0, false);

	return copy;
}

vec2 Laser::DrawProperties(vec3 startPos)
{
	std::string strID = std::to_string(ID);
	vec3 drawPos = startPos;

	drawPos.y -= EditorObject::DrawProperties(startPos).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &this->editorRotation, Editor::panelPropertiesX, strID + "orientation").y;

	std::string types[] = { "No teleportation", "Deadly" };
	int intType = (int)laserType;
	drawPos.y -= Editor::OptionProp(drawPos, "Type", &intType, (int)LaserType::lastValue, &types[0], Editor::panelPropertiesX).y;
	SetType((LaserType)intType);

	drawPos.y -= Editor::CheckBox(drawPos, "Start on", &startOn, Editor::panelPropertiesX).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Start offset", &startOffset, Editor::panelPropertiesX, strID + "startOffset").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "End offset", &endOffset, Editor::panelPropertiesX, strID + "endOffset").y;

	vec2 res = vec2(drawPos - startPos);
	res.y *= -1;
	return res;
}

void Laser::SetType(LaserType newType)
{
	laserType = newType;

	if (editorSprite != nullptr)
		editorSprite->color = vec4(1, 0.8f, 0.9f, editorSpriteAlpha);
}

void Laser::ResetIngameState()
{
	if (startOn)
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}
}

void Laser::SetSpriteUniforms(Shader* shader, void* object)
{
	Laser* laser = (Laser*)object;

	shader->SetUniform("mainTexture", 0);
	RessourceManager::GetTexture("noise1.png")->Use(0);
	shader->SetUniform("time", Utility::time);
	shader->SetUniform("laserType", (int)laser->laserType);
	shader->SetUniform("intersectionPosition", laser->intersectionPos);
}

void Laser::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = &(events[0]);
	*resCount = LASER_EVENT_COUNT;
}

void Laser::TurnOn()
{
	if (laserOn) return;
	if (!enabled) throw "Don't call this method when object is disabled!";
	laserOn = true;

	displaySprite->DrawOnMainLoop();
	glowSprite->DrawOnMainLoop();

	lasers.push_back(this);

	float camDist = glm::length(Camera::position - (vec2)this->GetEditPos());
	float attenuation = Utility::GetSoundAttenuation(camDist, onOffSoundMinDist, onOffSoundMaxDist);
	Utility::PlaySound("laser_start.wav", Utility::gameSoundsVolume * soundStartVolume * attenuation);
	StartLoopSound();
}

void Laser::TurnOff()
{
	if (!laserOn) return;
	if (!enabled) throw "Don't call this method when object is disabled!";
	laserOn = false;

	displaySprite->StopDrawing();
	glowSprite->StopDrawing();

	lasers.remove(this);

	float camDist = glm::length(Camera::position - (vec2)this->GetEditPos());
	float attenuation = Utility::GetSoundAttenuation(camDist, onOffSoundMinDist, onOffSoundMaxDist);
	Utility::PlaySound("laser_stop.wav", Utility::gameSoundsVolume * soundStopVolume * attenuation);
	StopLoopSound();
}

void Laser::ToggleOnOff()
{
	if (laserOn)
		TurnOff();
	else
		TurnOn();
}
