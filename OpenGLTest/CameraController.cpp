#include "CameraController.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "TweenManager.h"

ObjectEvent CameraController::events[CAM_CONTROL_EVENT_COUNT] =
{
	ObjectEvent {
		"Set zoom",
		[](EditorObject* object, void* param) {
			((CameraController*)object)->SetZoomWithProps();
		},
	},
};

CameraController::CameraController() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\cameraZoom.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "CameraController";
}

CameraController::~CameraController()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

	if (zoomTweenAction != nullptr && !zoomTweenAction->IsFinshedAt(Utility::time))
		TweenManager<float>::Cancel(zoomTweenAction);
}

vec2 CameraController::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Zoom", &zoom, Editor::panelPropertiesX, strID + "zoom").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Trans. duration", &transTime, Editor::panelPropertiesX, strID + "duration").y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* CameraController::Copy()
{
	CameraController* newObj = new CameraController(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void CameraController::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	EditorSaveManager::FloatProp(props, "zoom", &zoom);
	EditorSaveManager::FloatProp(props, "transTime", &transTime);
}

void CameraController::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("zoom", zoom);
	EditorSaveManager::WriteProp("transTime", transTime);
}

void CameraController::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void CameraController::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

void CameraController::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = &(events[0]);
	*resCount = CAM_CONTROL_EVENT_COUNT;
}

void CameraController::SetZoom(float zoom, float transTime)
{
	zoomTweenAction = TweenManager<float>::Tween(Camera::size, zoom * Camera::defaultSize, transTime, [](float val) { Camera::size = val; }, EaseType::sineInOut);
}

void CameraController::SetZoomWithProps()
{
	SetZoom(zoom, transTime);
}

void CameraController::UpdateTransform()
{
	EditorObject::UpdateTransform();
    
    if (editorSprite != nullptr)
	{
		editorSprite->position = editorPosition;
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}
}
