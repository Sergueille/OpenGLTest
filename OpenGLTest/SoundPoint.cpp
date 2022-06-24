#include "SoundPoint.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "Player.h"

ObjectEvent SoundPoint::events[SOUND_POINT_EVENT_COUNT] = {
	ObjectEvent {
		"Play sound",
		[](EditorObject* object, void* param) { ((SoundPoint*)object)->PlaySound(); }
	},
	ObjectEvent {
		"Stop sound",
		[](EditorObject* object, void* param) { ((SoundPoint*)object)->StopSound(); }
	},
};

SoundPoint::SoundPoint() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\soundPoint.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "SoundPoint";
}

SoundPoint::~SoundPoint()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

	StopSound();
}

vec2 SoundPoint::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::FileSelector(drawPos, "Sound", &soundName, &Editor::soundFiles, Editor::panelPropertiesX, strID + "sound").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Volume", &volume, Editor::panelPropertiesX, strID + "volume").y;

	drawPos.y -= Editor::CheckBox(drawPos, "Is spatial", &spatial, Editor::panelPropertiesX).y;
	if (spatial)
	{
		drawPos.y -= Editor::DrawProperty(drawPos, "Min distance", &minDist, Editor::panelPropertiesX, strID + "min").y;
		drawPos.y -= Editor::DrawProperty(drawPos, "Max distance", &maxDist, Editor::panelPropertiesX, strID + "max").y;
	}

	drawPos.y -= Editor::CheckBox(drawPos, "Auto start", &autoStart, Editor::panelPropertiesX).y;
	drawPos.y -= Editor::CheckBox(drawPos, "Is looping", &loop, Editor::panelPropertiesX).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* SoundPoint::Copy()
{
	SoundPoint* newObj = new SoundPoint(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	return newObj;
}

void SoundPoint::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	soundName = (*props)["soundName"];
	EditorSaveManager::FloatProp(props, "volume", &volume);
    
	spatial = (*props)["spatial"] == "1";
	EditorSaveManager::FloatProp(props, "minDist", &minDist);
	EditorSaveManager::FloatProp(props, "maxDist", &maxDist);

	autoStart = (*props)["autoStart"] == "1";
	loop = (*props)["loop"] == "1";

	if (!Editor::enabled && autoStart)
	{
		EventManager::DoInOneFrame([this] { this->PlaySound(); });
	}

	RessourceManager::GetSound(soundName); // Make sure the sound is loaded to prevent FPS drop during the level
}

void SoundPoint::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("soundName", soundName);
	EditorSaveManager::WriteProp("volume", volume);
	EditorSaveManager::WriteProp("spatial", spatial);
	EditorSaveManager::WriteProp("minDist", minDist);
	EditorSaveManager::WriteProp("maxDist", maxDist);
	EditorSaveManager::WriteProp("autoStart", autoStart);
	EditorSaveManager::WriteProp("loop", loop);
}

void SoundPoint::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
	
	if (playingBeforeDisabled)
		PlaySound();
}

void SoundPoint::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();

	playingBeforeDisabled = isPlaying;
	StopSound();
}

void SoundPoint::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = events;
	*resCount = SOUND_POINT_EVENT_COUNT;
}

void SoundPoint::PlaySound()
{
	if (isPlaying) StopSound();
	isPlaying = true;

	handle = Utility::PlaySound(soundName, Utility::gameSoundsVolume * volume);
	soloud->setLooping(handle, loop);
}

void SoundPoint::StopSound()
{
	if (!isPlaying) return;
	isPlaying = false;

	soloud->stop(handle);
}

void SoundPoint::OnMainLoop()
{
	EditorObject::OnMainLoop();
	if (!enabled) return;
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}

	if (isPlaying && spatial)
	{
		float dist = glm::length(Camera::position - vec2(GetEditPos()));
		float attenuation = 1 - ((dist - minDist) / (maxDist - minDist));
		if (attenuation < 0) attenuation = 0;
		if (attenuation > 1) attenuation = 1;

		soloud->setVolume(handle, Utility::gameSoundsVolume * volume * attenuation);
	}
}
