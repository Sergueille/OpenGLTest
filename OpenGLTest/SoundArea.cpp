#include "SoundArea.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "Player.h"

SoundArea* SoundArea::mainArea = nullptr;
SoundArea* SoundArea::mainMusicArea = nullptr;

SoundArea::SoundArea() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\SoundArea.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "SoundArea";
}

SoundArea::~SoundArea()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

	StopSound();
}

vec2 SoundArea::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Size", &editorSize, Editor::panelPropertiesX, strID + "size").y;
	drawPos.y -= Editor::FileSelector(drawPos, "Sound", &soundName, &Editor::soundFiles, Editor::panelPropertiesX, strID + "sound").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Volume", &volume, Editor::panelPropertiesX, strID + "volume").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Fade in duration", &fadeIn, Editor::panelPropertiesX, strID + "fadeIn").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Fade out duration", &fadeOut, Editor::panelPropertiesX, strID + "fadeOut").y;

	drawPos.y -= Editor::CheckBox(drawPos, "Is music", &isMusic, Editor::panelPropertiesX).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* SoundArea::Copy()
{
	SoundArea* newObj = new SoundArea(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	return newObj;
}

void SoundArea::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	editorSize = EditorSaveManager::StringToVector2((*props)["editorSize"], vec2(5));

	soundName = (*props)["soundName"];
	EditorSaveManager::FloatProp(props, "volume", &volume);
    
	isMusic = (*props)["isMusic"] == "1";
	EditorSaveManager::FloatProp(props, "fadeIn", &fadeIn);
	EditorSaveManager::FloatProp(props, "fadeOut", &fadeOut);

	RessourceManager::GetSound(soundName); // Make sure the sound is loaded to prevent FPS drop during the level
}

void SoundArea::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("editorSize", editorSize);
	EditorSaveManager::WriteProp("soundName", soundName);
	EditorSaveManager::WriteProp("volume", volume);
	EditorSaveManager::WriteProp("isMusic", isMusic);
	EditorSaveManager::WriteProp("fadeIn", fadeIn);
	EditorSaveManager::WriteProp("fadeOut", fadeOut);
}

void SoundArea::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void SoundArea::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();

	StopSound();
}

void SoundArea::PlaySound()
{
	if (isPlaying) StopSound();
	isPlaying = true;

	handle = Utility::PlaySound(soundName, Utility::gameSoundsVolume * volume);
	soloud->setLooping(handle, true);
}

void SoundArea::StopSound()
{
	if (!isPlaying) return;
	isPlaying = false;

	soloud->stop(handle);
}

void SoundArea::OnMainLoop()
{
	EditorObject::OnMainLoop();
	if (!enabled) return;
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}

	// Display size if selected
	if (Editor::enabled && Editor::GetSelectedObject() == this)
	{
		Sprite(nullptr, GetEditPos() - vec3(0, 0, 1), GetEditScale(), 0, vec4(0.5, 1, 0, 0.2)).Draw();
	}

	if (!Editor::enabled && Player::ingameInstance != nullptr)
	{
		RectCollider coll(GetEditPos(), GetEditScale(), 0, false);
		vec3 res = Player::ingameInstance->collider->CollideWith(&coll);

		if (res.z != 0) // Player is in the zone: start sound
		{
			if (isMusic)
				mainMusicArea = this;
			else
				mainArea = this;
		}
		else // Player is not in the zone: stop sound
		{
			if (isMusic && mainMusicArea == this) mainMusicArea = nullptr;
			if (!isMusic && mainArea == this) mainArea = nullptr;
		}

		if ((isMusic && mainMusicArea == this) || (!isMusic && mainArea == this)) // Increase volume
		{
			if (!isPlaying) PlaySound();

			relativeVolume += GetDeltaTime() / fadeIn;
			if (relativeVolume > 1) relativeVolume = 1;
		}
		else if (isPlaying) // Decrease volume
		{
			relativeVolume -= GetDeltaTime() / fadeOut;
			if (relativeVolume < 0)
			{
				relativeVolume = 0;
				StopSound();
			}
		}

		if (isPlaying) // Update volume
		{
			if (isMusic)
				soloud->setVolume(handle, Utility::musicVolume * volume * relativeVolume);
			else
				soloud->setVolume(handle, Utility::gameSoundsVolume * volume * relativeVolume);
		}
	}
}
