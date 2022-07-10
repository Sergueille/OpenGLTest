#include "TransformModifier.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

ObjectEvent TransformModifier::events[TRANS_MODIFIER_EVENT_COUNT] = {
	ObjectEvent {
		"Set position", 
		[](EditorObject* object, void* param) { ((TransformModifier*)object)->SetPosition(); }
	},
	ObjectEvent {
		"Set rotation",
		[](EditorObject* object, void* param) { ((TransformModifier*)object)->SetRotation(); }
	},
	ObjectEvent {
		"Set scale",
		[](EditorObject* object, void* param) { ((TransformModifier*)object)->SetScale(); }
	},
	ObjectEvent {
		"Set all transforms",
		[](EditorObject* object, void* param) { ((TransformModifier*)object)->SetAllTransforms(); }
	},
	ObjectEvent {
		"Cancell all",
		[](EditorObject* object, void* param) { ((TransformModifier*)object)->CancelAll(); }
	},
};

TransformModifier::TransformModifier() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\transformModifier.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
	else
	{
		EventManager::DoInOneFrame([this] { 
			this->targetObject = Editor::GetEditorObjectByIDInObjectContext(this, this->targetID, false, false); 

			if (targetObject != nullptr)
			{
				targetStartPos = targetObject->GetLocalEditPos();
				targetStartRotation = targetObject->GetLocalEditRotation();
				targetStartScale = targetObject->GetLocalEditScale();
			}
		});
	}
            
	typeName = "TransformModifier";
}

TransformModifier::~TransformModifier()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

	CancelAll();
}

vec2 TransformModifier::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

	// Object selector
	EditorObject* selected = Editor::GetEditorObjectByID(targetID, true, false);
	drawPos.y -= Editor::ObjectSelector(drawPos, "Target", &selected, Editor::panelPropertiesX, strID + "target").y;
	targetID = selected == nullptr ? -1 : selected->ID;

	drawPos.y -= Editor::CheckBox(drawPos, "Relative values", &relative, Editor::panelPropertiesX).y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Target position", &targetPos, Editor::panelPropertiesX, strID + "targetPos").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Target rotation", &targetRotation, Editor::panelPropertiesX, strID + "targetRotation").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Target scale", &targetSize, Editor::panelPropertiesX, strID + "targetSize").y;

	drawPos.y -= Editor::DrawProperty(drawPos, "Duration", &duration, Editor::panelPropertiesX, strID + "duration").y;
	drawPos.y -= Editor::OptionProp(drawPos, "Ease type", (int*)&easeType, EASE_TYPE_COUNT, (const char**)&EASE_TYPE_NAMES[0], Editor::panelPropertiesX).y;
	
	drawPos.y -= onFinished.DrawInPanel(drawPos, "On finished").y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* TransformModifier::Copy()
{
	TransformModifier* newObj = new TransformModifier(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

	if (!Editor::enabled)
		EventManager::DoInOneFrame([newObj] {
			newObj->targetObject = Editor::GetEditorObjectByIDInObjectContext(newObj, newObj->targetID, false, false);
		});

	newObj->moveAction = nullptr;
	newObj->roateAction = nullptr;
	newObj->scaleAction = nullptr;

	return newObj;
}

void TransformModifier::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

	EditorSaveManager::IntProp(props, "targetID", &targetID);
	relative = (*props)["relative"] == "1";
	targetPos = EditorSaveManager::StringToVector2((*props)["targetPos"]);
	EditorSaveManager::FloatProp(props, "targetRotation", &targetRotation);
	targetSize = EditorSaveManager::StringToVector2((*props)["targetSize"]);
	EditorSaveManager::FloatProp(props, "duration", &duration);

	int res = 0;
	EditorSaveManager::IntProp(props, "easeType", &res);
	easeType = (EaseType)res;

	EventList::Load(&onFinished, (*props)["onFinished"]);
}

void TransformModifier::Save()
{
	EditorObject::Save();

	EditorSaveManager::WriteProp("targetID", targetID);
	EditorSaveManager::WriteProp("relative", relative);
	EditorSaveManager::WriteProp("targetPos", targetPos);
	EditorSaveManager::WriteProp("targetRotation", targetRotation);
	EditorSaveManager::WriteProp("targetSize", targetSize);
	EditorSaveManager::WriteProp("duration", duration);
	EditorSaveManager::WriteProp("easeType", (int)easeType);
	EditorSaveManager::WriteProp("onFinished", onFinished.GetString());
}

void TransformModifier::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void TransformModifier::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();

	CancelAll(); // TEST
}

void TransformModifier::GetObjectEvents(const ObjectEvent** firstEvent, int* count)
{
	*firstEvent = &events[0];
	*count = TRANS_MODIFIER_EVENT_COUNT;
}

void TransformModifier::ResetIngameState()
{
	CancelAll();
	
	if (targetObject != nullptr)
	{
		targetObject->SetEditPos(targetStartPos);
		targetObject->SetEditRotation(targetStartRotation);
		targetObject->SetEditScale(targetStartScale);
	}
}

void TransformModifier::SetPosition()
{
	if (targetObject == nullptr) return;
	vec2 obj = relative ? vec2(targetStartPos) + targetPos : targetPos;

	moveAction = TweenManager<vec2>::Tween(vec2(targetObject->GetLocalEditPos()), obj, duration,
		[this](vec2 value) { 
			if (!EditorSaveManager::isLoading)
				this->targetObject->SetEditPos(vec3(value.x, value.y, this->targetObject->GetLocalEditPos().z)); 
		},
	easeType)->SetOnFinished([this] { 
		onFinished.Call(this); 
		this->moveAction = nullptr;
	});
}

void TransformModifier::SetRotation()
{
	if (targetObject == nullptr) return;
	float obj = relative ? targetStartRotation + targetRotation : targetRotation;

	roateAction = TweenManager<float>::Tween(targetObject->GetLocalEditRotation(), obj, duration,
		[this](float value) { 
			if (!EditorSaveManager::isLoading)
				this->targetObject->SetEditRotation(value); 
		},
	easeType)->SetOnFinished([this] { 
		onFinished.Call(this);
		this->roateAction = nullptr;
	});
}

void TransformModifier::SetScale()
{
	if (targetObject == nullptr) return;
	vec2 obj = relative ? targetStartScale * targetSize : targetSize;

	scaleAction = TweenManager<vec2>::Tween(targetObject->GetLocalEditScale(), obj, duration,
		[this](vec2 value) {
			if (!EditorSaveManager::isLoading)
				this->targetObject->SetEditScale(value); 
		},
	easeType)->SetOnFinished([this] { 
		onFinished.Call(this);
		this->scaleAction = nullptr;
	});
}

void TransformModifier::SetAllTransforms()
{
	SetPosition();
	SetRotation();
	SetScale();
}

void TransformModifier::CancelAll()
{
	if (moveAction != nullptr)
	{
		TweenManager<vec2>::Cancel(moveAction);
		moveAction = nullptr;
	}
	if (roateAction != nullptr)
	{
		TweenManager<float>::Cancel(roateAction);
		roateAction = nullptr;
	}
	if (scaleAction != nullptr)
	{
		TweenManager<vec2>::Cancel(scaleAction);
		scaleAction = nullptr;
	}
}

void TransformModifier::OnMainLoop()
{
	EditorObject::OnMainLoop();
    
    if (enabled && editorSprite != nullptr)
	{
		editorSprite->position = GetEditPos();
		editorSprite->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}
}
