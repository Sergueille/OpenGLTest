#include "EditorParticleSystem.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "ParticleSystem.h"

ObjectEvent EditorParticleSystem::events[PSYS_EVENT_COUNT] = {
	ObjectEvent{
		"Start",
		[](EditorObject* object, void* param) { ((EditorParticleSystem*)object)->Start(); },
	},
	ObjectEvent{
		"Stop",
		[](EditorObject* object, void* param) { ((EditorParticleSystem*)object)->Stop(); },
	},
};

EditorParticleSystem::EditorParticleSystem() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\particles.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();

		previewSprite = new Sprite(nullptr, vec3(0), vec2(1), 0, vec4(1, 1, 1, 0.1f));
    }

	changeSize = true;
	changeColor = true;
	paticleTemplate = new Sprite(nullptr, vec3(0));
            
	typeName = "EditorParticleSystem";
}

EditorParticleSystem::~EditorParticleSystem()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

	if (previewSprite != nullptr)
	{
		delete previewSprite;
		previewSprite = nullptr;
	}
}

vec2 EditorParticleSystem::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y + Editor::margin;

	bool pressed;
	drawPos.y -= Editor::UIButton(drawPos, "Start again", &pressed).y + Editor::margin;
	if (pressed) Start();

    drawPos.y -= Editor::CheckBox(drawPos, "Use circle emitter", &emitCircle, Editor::panelPropertiesX).y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Emitter orientation", &editorRotation, Editor::panelPropertiesX, strID + "ori").y;
    drawPos.y -= Editor::DrawProperty(drawPos, "Emitter scale", &editorSize, Editor::panelPropertiesX, strID + "size").y + Editor::margin;

    drawPos.y -= Editor::CheckBox(drawPos, "Auto start", &autoStart, Editor::panelPropertiesX).y;
    drawPos.y -= Editor::DrawProperty(drawPos, "Duration", &duration, Editor::panelPropertiesX, strID + "duration").y;
    drawPos.y -= Editor::DrawProperty(drawPos, "Particles each second", &particlesPerSecond, Editor::panelPropertiesX, strID + "quantity").y;
    drawPos.y -= Editor::DrawProperty(drawPos, "Particles lifetime", &particleLifetime, Editor::panelPropertiesX, strID + "lifetime").y + Editor::margin;

	drawPos.y -= Editor::DrawProperty(drawPos, "Start velocity", &startVelocity, Editor::panelPropertiesX, strID + "startVel").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "End velocity", &endVelocity, Editor::panelPropertiesX, strID + "endVel").y + Editor::margin;
    drawPos.y -= Editor::CheckBox(drawPos, "Velocity from angle", &velocityFromAngle, Editor::panelPropertiesX).y;

    drawPos.y -= Editor::DrawProperty(drawPos, "Start scale", &startSize, Editor::panelPropertiesX, strID + "startsize").y;
    drawPos.y -= Editor::DrawProperty(drawPos, "End scale", &endSize, Editor::panelPropertiesX, strID + "endsize").y + Editor::margin;

	drawPos.y -= Editor::DrawProperty(drawPos, "Start color", &startColor, Editor::panelPropertiesX, strID + "startcolor").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "End color", &endColor, Editor::panelPropertiesX, strID + "endcolor").y + Editor::margin;

	drawPos.y -= Editor::DrawProperty(drawPos, "Particle texture", &paticleTemplate->texture, Editor::panelPropertiesX, strID + "tex").y;
	drawPos.y -= Editor::CheckBox(drawPos, "Is lit", &paticleTemplate->isLit, Editor::panelPropertiesX).y;
	drawPos.y -= Editor::CheckBox(drawPos, "Ignore distance optilization", &ignoreDistanceOptimization, Editor::panelPropertiesX).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* EditorParticleSystem::Copy()
{
	EditorParticleSystem* newObj = new EditorParticleSystem(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();
    if (previewSprite != nullptr) newObj->previewSprite = this->previewSprite->Copy();
    if (paticleTemplate != nullptr) newObj->paticleTemplate = this->paticleTemplate->Copy();

	newObj->particles = std::list<Particle*>();
	newObj->SubscribeToPerticleSysEvents();

	return newObj;
}

void EditorParticleSystem::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

    EditorSaveManager::FloatProp(props, "orientation", &editorRotation);
    editorSize = EditorSaveManager::StringToVector2((*props)["scale"]);

	duration = -1; // Default
    EditorSaveManager::FloatProp(props, "duration", &duration);

	emitCircle = (*props)["emitCircle"] == "1";
	velocityFromAngle = (*props)["velocityFromAngle"] == "1";

	startSize = EditorSaveManager::StringToVector2((*props)["startSize"]);
	endSize = EditorSaveManager::StringToVector2((*props)["endSize"]);

	startVelocity = EditorSaveManager::StringToVector2((*props)["startVelocity"]);
	endVelocity = EditorSaveManager::StringToVector2((*props)["endVelocity"]);

	startColor = EditorSaveManager::StringToVector4((*props)["startColor"]);
	endColor = EditorSaveManager::StringToVector4((*props)["endColor"]);

	if ((*props)["texture"] != "")
		paticleTemplate->texture = RessourceManager::GetTexture((*props)["texture"]);

	paticleTemplate->isLit = (*props)["isLit"] == "1";

	EditorSaveManager::FloatProp(props, "particlesPerSecond", &particlesPerSecond);
	EditorSaveManager::FloatProp(props, "particleLifetime", &particleLifetime);

	autoStart = (*props)["autoStart"] != "0";
	ignoreDistanceOptimization = (*props)["ignoreDistanceOptimization"] == "1";
	
	if (autoStart)
		EventManager::DoInOneFrame([this] { Start(); });
}

void EditorParticleSystem::Save()
{
	EditorObject::Save();

    EditorSaveManager::WriteProp("orientation", editorRotation);
    EditorSaveManager::WriteProp("scale", editorSize);

    EditorSaveManager::WriteProp("emitCircle", emitCircle);
    EditorSaveManager::WriteProp("velocityFromAngle", velocityFromAngle);

    EditorSaveManager::WriteProp("startVelocity", startVelocity);
    EditorSaveManager::WriteProp("endVelocity", endVelocity);
    EditorSaveManager::WriteProp("startSize", startSize);
    EditorSaveManager::WriteProp("endSize", endSize);
    EditorSaveManager::WriteProp("startColor", startColor);
    EditorSaveManager::WriteProp("endColor", endColor);

	EditorSaveManager::WriteProp("texture", paticleTemplate->texture == nullptr ? "" : paticleTemplate->texture->path);
	EditorSaveManager::WriteProp("isLit", paticleTemplate->isLit);

	EditorSaveManager::WriteProp("particlesPerSecond", particlesPerSecond);
	EditorSaveManager::WriteProp("particleLifetime", particleLifetime);

	EditorSaveManager::WriteProp("duration", duration);

	EditorSaveManager::WriteProp("autoStart", autoStart);
	EditorSaveManager::WriteProp("ignoreDistanceOptimization", ignoreDistanceOptimization);
}

void EditorParticleSystem::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) 
		editorSprite->DrawOnMainLoop();

	Start();
}

void EditorParticleSystem::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) 
		editorSprite->StopDrawing();

	if (previewSprite != nullptr)
		previewSprite->StopDrawing();

	Stop();
}

void EditorParticleSystem::OnSelected()
{
	if (previewSprite != nullptr)
		previewSprite->DrawOnMainLoop();
}

void EditorParticleSystem::OnUnselected()
{
	if (previewSprite != nullptr)
		previewSprite->StopDrawing();
}

void EditorParticleSystem::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = &events[0];
	*resCount = PSYS_EVENT_COUNT;
}

void EditorParticleSystem::ResetIngameState()
{
	if (!autoStart)
	{
		Stop();
	}
}

void EditorParticleSystem::OnMainLoop()
{
	EditorObject::OnMainLoop();
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos() + vec3(0, 0, 50); 
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;

		previewSprite->position = GetEditPos() - vec3(0, 0, 0.1f);
		previewSprite->rotate = GetEditRotation();
		previewSprite->size = GetEditScale();

		previewSprite->texture = emitCircle ? RessourceManager::GetTexture("Engine\\circle.png") : nullptr;
	}

	emitterPosition = GetEditPos();
	emitterRotation = GetEditRotation();
	emitterSize = GetEditScale();
}
