print("Welcome to the editor class generator!")
class_name = input("Class name : ")
use_circle_coll = input("Collider type (Circle / Rect) : ").lower()[0] == "c"

use_ori = input("Use orientation (Yes / No) : ").lower()[0] == "y"
use_scale = input("Use scale (Yes / No) : ").lower()[0] == "y"

sprite_type = input("Sprite type (None / Always / Editor) : ").lower()[0]
if sprite_type == "n":
    sprite_type = "none"
elif sprite_type == "a":
    sprite_type = "always"
elif sprite_type == "e":
    sprite_type = "editor"
else:
    raise "Wrong sprite type!"

collName = "CircleCollider" if use_circle_coll else "RectCollider"
spriteName = "sprite" if sprite_type == "always" else "editorSprite"

h_txt = f"""#pragma once

#include "EditorObject.h"
#include "Sprite.h"
#include "RessourceManager.h"

using namespace glm;

class {class_name} : public EditorObject
{{
public:
	{class_name}();
	~{class_name}();

    {
        f"Sprite* {spriteName} = nullptr;"
        if sprite_type != "none" else ""
    }

	virtual vec2 DrawProperties(vec3 drawPos) override;
	virtual void UpdateTransform() override;

	virtual EditorObject* Copy() override;

	virtual void Load(std::map<std::string, std::string>* props) override;
	virtual void Save() override;

	virtual void Enable() override;
	virtual void Disable() override;
}};

"""


def GetSpriteCreation():
    res = ""
    if sprite_type != "none":
        if sprite_type == "editor":
            res = f"""if (Editor::enabled)
    {{
        {spriteName} = new Sprite(nullptr, vec3(0), vec2(1), 0);
        {spriteName}->DrawOnMainLoop();
    }}
            """
        else:
            res = f"""{spriteName} = new Sprite(nullptr, vec3(0), vec2(1), 0);
    {spriteName}->DrawOnMainLoop();
            """

    return res


def GetSpriteDestruction():
    res = ""
    if sprite_type != "none":
        res = f"""if ({spriteName} != nullptr)
    {{
        delete {spriteName};
        {spriteName} = nullptr;
    }}
        """
    return res


def GetSpriteUpdateTransform():
    res = "\n"
    if use_circle_coll:
        if sprite_type == "editor":
            res += f"""    if ({spriteName} != nullptr)
	{{
		{spriteName}->position = editorPosition;
		{spriteName}->size = vec2(Editor::gizmoSize);
		((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
	}}"""


        else:
            res += f"\t{spriteName}->position = editorPosition;"

            
    else:
        if sprite_type == "editor":
            res += f"""    if ({spriteName} != nullptr)
	{{
		{spriteName}->position = editorPosition;
	}}"""

        else:
            res += f"{spriteName}->position = editorPosition;"

            
        if use_scale:
            res += "\n\t((RectCollider*)clickCollider)->size = editorSize;"
        if use_ori:
            res += "\n\t((RectCollider*)clickCollider)->orientation = editorRotation;"

    return res


def GetSpriteCopy():
    res = ""
    if sprite_type != "none":
        if sprite_type == "editor":
            res += f"if ({spriteName} != nullptr) "

        res += f"newObj->{spriteName} = this->{spriteName}->Copy();"
    return res

def GetSpriteEnable():
    res = ""
    if sprite_type != "none":
        if sprite_type == "editor":
            res += f"if ({spriteName} != nullptr) "

        res += f"{spriteName}->DrawOnMainLoop();"
    return res

def GetSpriteDisable():
    res = ""
    if sprite_type != "none":
        if sprite_type == "editor":
            res += f"if ({spriteName} != nullptr) "

        res += f"{spriteName}->StopDrawing();"
    return res


cpp_txt = f"""#include "{class_name}.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

{class_name}::{class_name}() : EditorObject(vec3(0))
{{
	clickCollider = { "new CircleCollider(vec2(0), 1, false);"
        if use_circle_coll else
        "new RectCollider(vec2(0), vec2(1), 0, false);"
    }

    {GetSpriteCreation()}
	typeName = "{class_name}";
}}

{class_name}::~{class_name}()
{{
    {GetSpriteDestruction()}
}}

vec2 {class_name}::DrawProperties(vec3 drawPos)
{{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

    {
        'drawPos.y -= Editor::DrawProperty(drawPos, "Orientation", &editorRotation, Editor::panelPropertiesX, strID + "ori").y;'
        if use_ori else ''
    }
    {
        'drawPos.y -= Editor::DrawProperty(drawPos, "Scale", &editorSize, Editor::panelPropertiesX, strID + "size").y;'
        if use_scale else ''
    }

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}}

EditorObject* {class_name}::Copy()
{{
	{class_name}* newObj = new {class_name}(*this);

	// copy collider
	{collName}* oldCollider = ({collName}*)this->clickCollider;
	newObj->clickCollider = { "new RectCollider(oldCollider->position, oldCollider->size, oldCollider->orientation, oldCollider->MustCollideWithPhys());"
        if not use_circle_coll else
        "new CircleCollider(oldCollider->position, oldCollider->size, oldCollider->MustCollideWithPhys());"
    }

    {GetSpriteCopy()}

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}}

void {class_name}::Load(std::map<std::string, std::string>* props)
{{
	EditorObject::Load(props);

    {
        'EditorSaveManager::FloatProp(props, "orientation", &editorRotation);'
        if use_ori else ''
    }
    {
        'editorSize = EditorSaveManager::StringToVector2((*props)["scale"]);'
        if use_scale else ''
    }
}}

void {class_name}::Save()
{{
	EditorObject::Save();

    {
        'EditorSaveManager::WriteProp("orientation", editorRotation);'
        if use_ori else ''
    }
    {
        'EditorSaveManager::WriteProp("scale", editorSize);'
        if use_scale else ''
    }
}}

void {class_name}::Enable()
{{
	EditorObject::Enable();
    {GetSpriteEnable()}
}}

void {class_name}::Disable()
{{
	EditorObject::Disable();
    {GetSpriteDisable()}
}}

void {class_name}::UpdateTransform()
{{
	EditorObject::UpdateTransform();
    {GetSpriteUpdateTransform()}
}}
"""

with open(__file__ + "\\..\\..\\" + class_name + ".h", "w") as hfile:
    hfile.write(h_txt)

with open(__file__ + "\\..\\..\\" + class_name + ".cpp", "w") as cppfile:
    cppfile.write(cpp_txt)

print("-------")
print("Files created!")
print("Now you have to:")
print("  - Add files to the project in VisualStudio")
print("  - Add your class to EditorSaveManager::ReadObject()")
print("  - Add your class to Editor::DrawAddTab()")
