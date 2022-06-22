#pragma once

#include "Collider.h"
#include "Editor.h"

#include <glm/common.hpp>
#include <string>

using namespace glm;

class EditorObject;
struct ObjectEvent
{
	std::string eventName;
	std::function<void(EditorObject* object, void* param)> func;
};

class Prefab;
class EditorSaveManager;
class EditorObject
{
public:
	EditorObject(vec3 position);
	virtual ~EditorObject();

	std::string typeName;

	int ID;
	std::string name;

	vec3 GetEditPos();
	vec3 GetLocalEditPos();
	float GetEditRotation();
	float GetLocalEditRotation();
	vec2 GetEditScale();
	vec2 GetLocalEditScale();
	EditorObject* GetParent();
	int parentID = -1;

	vec3 SetEditPos(vec3 pos);
	float SetEditRotation(float rot);
	vec2 SetEditScale(vec2 scale);
	void SetParent(EditorObject* newParent);

	void SearchParent();

	vec3 SetGlobalEditPos(vec3 pos);

	float parallax = 1;

	/// <summary>
	/// Collider used to click on object in the editor
	/// </summary>
	Collider* clickCollider;

	/// <summary>
	/// List in wich the object must search for events, parenting, etc., for example a prefab list
	/// </summary>
	std::list<EditorObject*>* contextList = nullptr;

	/// <summary>
	/// Draws properties of object in the editor panel
	/// </summary>
	virtual vec2 DrawProperties(vec3 drawPos);
	/// <summary>
	/// Draws buttons under the properties
	/// </summary>
	virtual vec2 DrawActions(vec3 drawPos);

	/// <summary>
	/// Returns a copy of the object
	/// </summary>
	/// <returns></returns>
	virtual EditorObject* Copy() = 0;

	/// <summary>
	/// Save the object, please use EditorSaveManager api
	/// because of inhiterance, DO NOT INCLUDE StartObject() AND EndObject()
	/// </summary>
	virtual void Save();
	virtual void Load(std::map<std::string, std::string>* props);

	/// <summary>
	/// Restores the sprites as it was before being disabled
	/// </summary>
	virtual void Enable();
	/// <summary>
	/// The object will not be visible, clickable or everything else. DON'T FORGET TO CALL BASE FUNCTION WHEN OVERRIDING
	/// </summary>
	virtual void Disable();
	bool IsEnabled();
	/// <summary>
	/// TOGGLE TOGGLE TOGGLE!
	/// </summary>
	void ToggleEnabled();

	virtual void GetObjectEvents(const ObjectEvent** res, int* resCount);

	void CallEvent(std::string eventName);

	virtual void GetAABB(vec2* minRes, vec2* maxRes);

	virtual void OnSelected() { };
	virtual void OnUnselected() { };

	/// <summary>
	/// Called when loading user save
	/// </summary>
	virtual void ResetIngameState() { };

	virtual void OnMainLoop();

protected:
	bool enabled = true;
	vec3 editorPosition;
	float editorRotation;
	vec2 editorSize = vec2(1);

private:
	EditorObject* _parent = nullptr;
};

struct EventList
{
	EventList();
	~EventList();

	/// <summary>
	/// TODO: store object pointers instead of their ID
	/// </summary>
	void Call(EditorObject* context); 

	vec2 DrawInPanel(vec3 drawPos, std::string eventName);

	std::string GetString();
	static void Load(EventList* res, std::string text);

	std::list<int> ids;
	std::list<std::string> events;
};
