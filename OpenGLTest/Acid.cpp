#include "Acid.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "Player.h"
#include "TweenManager.h"

bool Acid::isPlayerDying = false;

Acid::Acid() : EditorObject(vec3(0))
{
	clickCollider = new RectCollider(vec2(0), vec2(1), 0, false);

    sprite = new Sprite(RessourceManager::GetTexture("noise1.png"), vec3(0), vec2(1), 0, vec4(0), &RessourceManager::shaders["acid"]);
    sprite->DrawOnMainLoop();
	sprite->setUniforms = &SetSpriteUniforms;
	sprite->setUniformsObjectCall = this;
	sprite->isLit = true;

	if (!Editor::enabled)
		acidMainLoopFuncPos = EventManager::OnMainLoop.push_end([this] { this->OnAcidMainLoop(); });
            
	typeName = "Acid";
}

Acid::~Acid()
{
    if (sprite != nullptr)
    {
        delete sprite;
        sprite = nullptr;
    }

	if (acidMainLoopFuncPos != nullptr)
		EventManager::OnMainLoop.remove(acidMainLoopFuncPos);
}

vec2 Acid::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

    drawPos.y -= Editor::DrawProperty(drawPos, "Scale", &editorSize, Editor::panelPropertiesX, strID + "size").y;
    drawPos.y -= Editor::CheckBox(drawPos, "Show surface", &showSurface, Editor::panelPropertiesX).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* Acid::Copy()
{
	Acid* newObj = new Acid(*this);

	// copy collider
	RectCollider* oldCollider = (RectCollider*)this->clickCollider;
	newObj->clickCollider = new RectCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->orientation, oldCollider->MustCollideWithPhys());

    newObj->sprite = this->sprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void Acid::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);
    editorSize = EditorSaveManager::StringToVector2((*props)["scale"]);

	showSurface = (*props)["showSurface"] != "0";
}

void Acid::Save()
{
	EditorObject::Save();

    EditorSaveManager::WriteProp("scale", editorSize);
    EditorSaveManager::WriteProp("showSurface", showSurface);
}

void Acid::Enable()
{
	EditorObject::Enable();
    sprite->DrawOnMainLoop();
}

void Acid::Disable()
{
	EditorObject::Disable();
    sprite->StopDrawing();
}

void Acid::SetSpriteUniforms(Shader* shader, void* object)
{
	Acid* acid = (Acid*)object;
	if (acid == nullptr || !acid->enabled) return;

	shader->SetUniform("time", Utility::time);
	shader->SetUniform("spriteSize", acid->GetEditScale());
	shader->SetUniform("showSurface", acid->showSurface);
}

void Acid::OnAcidMainLoop()
{
	if (Player::ingameInstance == nullptr) return;

	vec3 res = clickCollider->CollideWith((CircleCollider*)Player::ingameInstance->collider);

	if (!isPlayerDying && res.z != 0) // Player is touching acid
	{
		isPlayerDying = true;
		Player::ingameInstance->physicsEnabled = false;

		TweenManager<float>::Tween(Camera::size, acidDeathZoom, acidDeathDuration, [this](float value) {
			Camera::SetSize(value);
		}, sineIn);

		TweenManager<float>::Tween(Camera::position.y, Camera::position.y - acidDeathCameraShift, acidDeathDuration, [this](float value) {
			Camera::position.y = value;
		}, sineIn);
				
		TweenManager<float>::Tween(0, 1, acidDeathDuration, [this](float value) {
			Utility::corruptionAmount = value;
		}, cubicOut)->SetOnFinished([] {
			Utility::corruptionAmount = 0;
		});

		TweenManager<vec2>::Tween(Player::ingameInstance->GetPos(), vec2(Player::ingameInstance->GetPos()) - vec2(0, acidDeathPlayerShift), acidDeathDuration, [this](vec2 value) {
			Player::ingameInstance->SetPos(vec2(value.x, value.y));
		}, linear)->SetOnFinished([] {
			EditorSaveManager::LoadUserSaveInSameLevel(EditorSaveManager::currentUserSave);
			isPlayerDying = false;
			Player::ingameInstance->physicsEnabled = true;
		});
	}
}

void Acid::UpdateTransform()
{
	EditorObject::UpdateTransform();
    
	sprite->position = GetEditPos();
	sprite->size = GetEditScale();
	((RectCollider*)clickCollider)->size = GetEditScale();
}