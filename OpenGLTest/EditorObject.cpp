#include "EditorObject.h"

#include <string>

using namespace glm;

EditorObject::EditorObject(vec3 position)
{
	this->SetEditPos(position);

	this->ID = Editor::IDmax;
	Editor::IDmax++;

	SubscribeToEditorObjectFuncs();
}

EditorObject::~EditorObject()
{
	if (clickCollider != nullptr)
	{
		delete clickCollider;
		clickCollider = nullptr;
	}

	if (mainLoopFuncPos != nullptr)
	{
		EventManager::OnMainLoop.remove(mainLoopFuncPos);
		mainLoopFuncPos = nullptr;
	}
}

vec3 EditorObject::GetEditPos()
{
	vec3 res;
	if (GetParent() != nullptr)
	{
		vec2 delta = Rotate(vec2(editorPosition), GetParent()->GetEditRotation()) * GetParent()->GetEditScale();
		vec3 delta2 = vec3(delta.x, delta.y, editorPosition.z);

		res = GetParent()->GetEditPos() + delta2;
	}
	else
	{
		res = editorPosition;
	}

	vec2 delta = vec2(res) - Camera::position;
	vec2 prallaxed = Camera::position + delta * parallax;

	return vec3(prallaxed.x, prallaxed.y, res.z);
}

vec3 EditorObject::GetLocalEditPos()
{
	return editorPosition;
}

float EditorObject::GetEditRotation()
{
	if (GetParent() != nullptr)
	{
		return editorRotation + GetParent()->GetEditRotation();
	}
	else
	{
		return editorRotation;
	}
}

float EditorObject::GetLocalEditRotation()
{
	return editorRotation;
}

vec2 EditorObject::GetEditScale()
{
	if (GetParent() != nullptr)
	{
		return editorSize * GetParent()->GetEditScale();
	}
	else
	{
		return editorSize;
	}
}

vec2 EditorObject::GetLocalEditScale()
{
	return editorSize;
}

EditorObject* EditorObject::GetParent()
{
	// return Editor::GetEditorObjectByIDInObjectContext(this, parentID, Editor::enabled, false);
	// TEEEEST

	if (_parent == nullptr && parentID != -1)
	{
		_parent = Editor::GetEditorObjectByIDInObjectContext(this, parentID, Editor::enabled, false);
	}

	if (_parent == nullptr) // Not found
		parentID = -1; // Make sure not searching for nothig next time

	return _parent;
}

void EditorObject::SetParent(EditorObject* newParent)
{
	_parent = newParent;
	parentID = newParent == nullptr ? -1 : newParent->ID;
}

void EditorObject::SearchParent()
{
	_parent = nullptr;
}

vec3 EditorObject::SetEditPos(vec3 pos)
{
	editorPosition = pos;
	return editorPosition;
}

float EditorObject::SetEditRotation(float rot)
{
	editorRotation = rot;
	return rot;
}

vec2 EditorObject::SetEditScale(vec2 scale)
{
	editorSize = scale;
	return scale;
}

vec3 EditorObject::SetGlobalEditPos(vec3 pos)
{
	vec3 res;
	if (GetParent() != nullptr)
	{
		vec3 delta = pos - GetParent()->GetEditPos();
		vec2 finalDelta = Rotate(vec2(delta), -GetParent()->GetEditRotation()) / GetParent()->GetEditScale();
		res = vec3(finalDelta.x, finalDelta.y, delta.z);
	}
	else
	{
		res = pos;
	}

	editorPosition = res;
	return editorPosition;
}

void EditorObject::UpdateTransform()
{
	if (clickCollider)
		clickCollider->SetPos(GetEditPos());
}

void EditorObject::SubscribeToEditorObjectFuncs()
{
	mainLoopFuncPos = EventManager::OnMainLoop.push_end([this] { OnMainLoop(); });
}

vec2 EditorObject::DrawProperties(vec3 startPos)
{
	std::string strID = std::to_string(ID);

	vec3 drawPos = startPos;
	drawPos.y -= Editor::DrawProperty(drawPos, "Name", &name, Editor::panelPropertiesX, strID + "name").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Position", &editorPosition, Editor::panelPropertiesX, strID + "pos").y;
	drawPos.y -= Editor::DrawProperty(drawPos, "Parallax", &parallax, Editor::panelPropertiesX, strID + "parallax").y;
	
	EditorObject* selected = GetParent();
	drawPos.y -= Editor::ObjectSelector(drawPos, "Parent", &selected, Editor::panelPropertiesX, strID + "parent").y;
	SetParent(selected);

	vec2 res = vec2(drawPos - startPos);
	res.y *= -1;
	return res;
}

vec2 EditorObject::DrawActions(vec3 drawPos)
{
	vec3 startPos = drawPos;

	bool res;
	drawPos.x += Editor::UIButton(drawPos, "Destroy", &res).x + Editor::margin;

	if (res)
	{
		Editor::RemoveObject(this);
		return vec2(0);
	}

	vec2 lastBtnSize = Editor::UIButton(drawPos, "Duplicate", &res);

	if (res)
	{
		EditorObject* newObj = Copy();
		Editor::AddObject(newObj);
		Editor::SelectObject(newObj);
	}

	drawPos.x += lastBtnSize.x;
	drawPos.y += lastBtnSize.y * -1;

	vec2 size = vec2(drawPos - startPos);
	size.y *= -1;
	return size;
}

void EditorObject::Save()
{
	EditorSaveManager::WriteProp("ID", std::to_string(ID));
	EditorSaveManager::WriteProp("name", name);
	EditorSaveManager::WriteProp("position", editorPosition);
	EditorSaveManager::WriteProp("parent", parentID);
}

void EditorObject::Load(std::map<std::string, std::string>* props)
{
	editorPosition = EditorSaveManager::StringToVector3((*props)["position"]);
	name = (*props)["name"];
	ID = std::stoi((*props)["ID"]);
	EditorSaveManager::IntProp(props, "parent", &parentID);
}

void EditorObject::Enable()
{
	if (enabled) return;

	enabled = true;
	clickCollider->enabled = true;
	SubscribeToEditorObjectFuncs();
}

void EditorObject::Disable()
{
	if (!enabled) return;

	enabled = false;
	clickCollider->enabled = false;

	if (mainLoopFuncPos != nullptr)
	{
		EventManager::OnMainLoop.remove(mainLoopFuncPos);
		mainLoopFuncPos = nullptr;
	}
}

bool EditorObject::IsEnabled()
{
	return enabled;
}

void EditorObject::ToggleEnabled()
{
	if (enabled)
		Disable();
	else
		Enable();
}

void EditorObject::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
	*res = nullptr;
	*resCount = 0;
}

void EditorObject::CallEvent(std::string eventName)
{
	const ObjectEvent* firstEvent; int eventCount;
	this->GetObjectEvents(&firstEvent, &eventCount);

	for (int i = 0; i < eventCount; i++)
	{
		if ((firstEvent + i)->eventName == eventName)
		{
			(firstEvent + i)->func(this, nullptr);
			return;
		}
	}
}

void EditorObject::GetAABB(vec2* minRes, vec2* maxRes)
{
	clickCollider->GetAABB(minRes, maxRes);
}

void EditorObject::OnMainLoop()
{
	if (!enabled) return;

	UpdateTransform();
	DerivedOnMainLoop();
}

EventList::EventList()
{
	ids = std::list<int>();
	events = std::list<std::string>();
}

EventList::~EventList()
{
	
}

void EventList::Call(EditorObject* context)
{
	if (Editor::enabled)
		throw "Must not call event while in editor";

	auto itID = ids.begin();
	auto itEvents = events.begin();
	for (; itID != ids.end(); itID++, itEvents++)
	{
		EditorObject* target = Editor::GetEditorObjectByIDInObjectContext(context, *itID, false, false);

		if (target == nullptr)
		{
			std::cout << "Object event has no valid target!" << std::endl;
			continue;
		}

		target->CallEvent(*itEvents);
	}
}

vec2 EventList::DrawInPanel(vec3 drawPos, std::string eventName)
{
	vec2 startPos = vec2(drawPos);
	drawPos.y -= Editor::margin;
	drawPos.y -= TextManager::RenderText(eventName, drawPos, Editor::textSize).y;
	drawPos.x += Editor::indentation;

	// For each event to send
	auto itID = ids.begin();
	auto itEvents = events.begin();
	int i = 0;
	for (; itID != ids.end(); itID++, itEvents++, i++)
	{
		// Display number
		float numberSize = TextManager::RenderText(std::to_string(i) + ":", drawPos, Editor::textSize).x;
		drawPos.x += numberSize;

		// Target selector
		EditorObject* target = Editor::GetEditorObjectByID(*itID, true, false);
		drawPos.y -= Editor::ObjectSelector(drawPos, "Target", &target, Editor::panelPropertiesX - Editor::indentation, eventName + std::to_string(i) + "ID").y;
		
		int newId = target == nullptr ? -1 : target->ID;
		if (newId != *itID) // Apply if new object
		{
			*itID = newId;
		}
		
		if (target != nullptr)
		{
			// Select event method
			const ObjectEvent* firstEvent; int eventCount;
			target->GetObjectEvents(&firstEvent, &eventCount);

			std::string* names = new std::string[eventCount + 1]; // Get event names
			int selected = eventCount; // get currently selected event
			for (int i = 0; i < eventCount; i++)
			{
				names[i] = (firstEvent + i)->eventName;
				if ((firstEvent + i)->eventName == *itEvents)
				{
					selected = i;
				}
			}

			names[eventCount] = "None"; // Additional element if nthing selected

			drawPos.y -= Editor::OptionProp(drawPos, "Event", &selected, eventCount, names, Editor::panelPropertiesX).y;

			if (selected != eventCount) // If an event is selected
			{
				*itEvents = names[selected];
			}

			delete[] names;
		}

		// Remove button
		bool mustRemove = false;
		drawPos.y -= Editor::UIButton(drawPos, "Remove", &mustRemove).y;

		if (mustRemove)
		{
			ids.erase(itID);
			events.erase(itEvents);
			break;
		}

		drawPos.x -= numberSize;
		drawPos.y -= Editor::margin;
	}

	// Add button
	bool mustAdd;
	drawPos.y -= Editor::UIButton(drawPos, "Add event target", &mustAdd).y;

	if (mustAdd)
	{
		ids.push_back(-1);
		events.push_back("");
	}

	return Abs(vec2(drawPos) - startPos);
}

std::string EventList::GetString()
{
	std::string res = "";

	auto itID = ids.begin();
	auto itEvents = events.begin();
	int i = 0;
	for (; itID != ids.end(); itID++, itEvents++, i++)
	{
		res += std::to_string(*itID) + "|" + *itEvents + "|";
	}

	return res;
}

void EventList::Load(EventList* res, std::string text)
{
	*res = EventList();

	int elementBegin = 0;
	bool isID = true;

	for (int i = 0; i < text.length(); i++)
	{
		if (text[i] == '|')
		{
			std::string element = text.substr(elementBegin, i - elementBegin);

			if (isID)
			{
				res->ids.push_back(std::stoi(element));
			}
			else
			{
				res->events.push_back(element);
			}

			elementBegin = i + 1;
			isID = !isID;
		}
	}
}
