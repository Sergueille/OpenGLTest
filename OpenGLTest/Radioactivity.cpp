#include "Radioactivity.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

Radioactivity::Radioactivity() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\radioactivity.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "Radioactivity";
}

Radioactivity::~Radioactivity()
{
    if (editorSprite != nullptr)
    {
        delete editorSprite;
        editorSprite = nullptr;
    }

    if (mesh != nullptr)
    {
        delete mesh;
        mesh = nullptr;
    }        
}

vec2 Radioactivity::DrawProperties(vec3 drawPos)
{
	std::string strID = std::to_string(ID);
	vec2 startPos = vec2(drawPos);

	drawPos.y -= EditorObject::DrawProperties(drawPos).y;

    bool pressed;
    drawPos.y -= Editor::UIButton(drawPos, "Reload", &pressed).y;

    if (pressed)
        ForceRefreshMesh();    

    drawPos.y -= Editor::DrawProperty(drawPos, "Raycast count", &nbRaycast, Editor::panelPropertiesX, strID + "nbRaycast").y;
    drawPos.y -= Editor::CheckBox(drawPos, "Is realtime", &isRealTime, Editor::panelPropertiesX).y;

	vec2 res = vec2(drawPos) - startPos;
	res.y *= -1;
	return res;
}

EditorObject* Radioactivity::Copy()
{
	Radioactivity* newObj = new Radioactivity(*this);

	// copy collider
	CircleCollider* oldCollider = (CircleCollider*)this->clickCollider;
	newObj->clickCollider = new CircleCollider(oldCollider->GetPos(), oldCollider->size, oldCollider->MustCollideWithPhys());

    if (editorSprite != nullptr) newObj->editorSprite = this->editorSprite->Copy();

    newObj->mesh = nullptr;

	newObj->SubscribeToEditorObjectFuncs();

	return newObj;
}

void Radioactivity::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

    EditorSaveManager::IntProp(props, "nbRaycast", &nbRaycast);
    isRealTime = (*props)["isRealTime"] == "1";
}

void Radioactivity::Save()
{
	EditorObject::Save();

    EditorSaveManager::WriteProp("nbRaycast", nbRaycast);
    EditorSaveManager::WriteProp("isRealTime", isRealTime);
}

void Radioactivity::Enable()
{
	EditorObject::Enable();
    if (editorSprite != nullptr) editorSprite->DrawOnMainLoop();
}

void Radioactivity::Disable()
{
	EditorObject::Disable();
    if (editorSprite != nullptr) editorSprite->StopDrawing();
}

Mesh* Radioactivity::GetMesh()
{
    if (mesh != nullptr) return mesh;

    int vertCount = nbRaycast + 1;
    int faceCount = nbRaycast;
    constexpr int valuesPerVert = 5;
    constexpr int valuesPerFace = 3;
    constexpr float maxDist = 50;
    constexpr float zPos = -90;

    float* vertices = new float[vertCount * valuesPerVert];
    unsigned int* indices = new unsigned int[faceCount * valuesPerFace];

    // Set middle point
    vec3 editPos = GetEditPos();
    vertices[0] = editPos.x;
    vertices[1] = editPos.y;
    vertices[2] = zPos;
    vertices[3] = 0;
    vertices[4] = 1;

    int currentVtex = 0;
    for (int i = 0; i < nbRaycast; i++)
    {
        float tetha = i * (360.f/ (float)nbRaycast);
        vec2 dir = Rotate(vec2(1, 0), tetha);
        vec2 res;

        if (!Collider::Raycast(vec2(editPos), dir, &res))
        {
            // If nothig is hit, use max distance
            res = vec2(editPos) + dir * maxDist;
        }

        if (i > 2 && i < nbRaycast - 1)
        {
            int lastArrayPos = currentVtex * valuesPerVert;
            int secondLastArrayPos = (currentVtex - 1) * valuesPerVert;

            vec2 lastVtexPos = vec2(vertices[lastArrayPos], vertices[lastArrayPos + 1]);
            vec2 secondLastVtexPos = vec2(vertices[secondLastArrayPos], vertices[secondLastArrayPos + 1]);

            vec2 lastDelta = lastVtexPos - secondLastVtexPos;
            vec2 currentDelta = res - lastVtexPos;

            float diff = glm::dot(lastDelta, currentDelta) - glm::length(lastDelta) * glm::length(currentDelta);

            // Same direction, do not need the previous vertex
            if (abs(diff) < 0.01)
            {
                vertices[lastArrayPos] = res.x;
                vertices[lastArrayPos + 1] = res.y;
                continue;
            }
        }

        int arrayPos = (currentVtex + 1) * valuesPerVert;
        vertices[arrayPos] = res.x;
        vertices[arrayPos + 1] = res.y;
        vertices[arrayPos + 2] = zPos;
        vertices[arrayPos + 3] = (float)i / (float)nbRaycast;
        vertices[arrayPos + 4] = 0;

        // set faces
        int faceArrayPos = currentVtex * valuesPerFace;
        indices[faceArrayPos] = 0;
        indices[faceArrayPos + 1] = currentVtex + 1;

        if (i == nbRaycast - 1)
            indices[faceArrayPos + 2] = 1;
        else
            indices[faceArrayPos + 2] = currentVtex + 2;

        currentVtex++;

        //std::printf("%d:\n", currentVtex);
        //std::printf("   %f, %f, %f, %f, %f\n", vertices[arrayPos], vertices[arrayPos+1], vertices[arrayPos+2], vertices[arrayPos+3], vertices[arrayPos+4]);
        //std::printf("   %d, %d, %d\n", indices[faceArrayPos], indices[faceArrayPos+1], indices[faceArrayPos+2]);
    }

    std::printf("Radioactivity source baked with %d vertices\n", currentVtex);

    mesh = new Mesh(vertices, (currentVtex + 1) * valuesPerVert, indices, faceCount * valuesPerFace);

    delete[] vertices;
    delete[] indices;

    return mesh;
}

void Radioactivity::ForceRefreshMesh()
{
    if (mesh != nullptr)
    {
        delete mesh;
        mesh = nullptr;
    }
}

void Radioactivity::UpdateTransform()
{
	EditorObject::UpdateTransform();
    
    if (editorSprite != nullptr)
    {
        editorSprite->position = GetEditPos();
        editorSprite->size = vec2(Editor::gizmoSize);
        ((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
    }

    if (isRealTime)
    {
        ForceRefreshMesh();
    }

    glDisable(GL_DEPTH_TEST);

    Shader* shader = &RessourceManager::shaders["radioactivity"];
    shader->Use();
    shader->SetUniform("projection", Camera::GetOrthographicProjection());
    shader->SetUniform("transform", glm::mat4(1.0f));
    GetMesh()->DrawMesh();

    glEnable(GL_DEPTH_TEST);
}
