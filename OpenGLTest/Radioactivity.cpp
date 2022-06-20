#include "Radioactivity.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"

constexpr int MAX_VERT_COUNT = 256;
constexpr int MAX_RAY_COUNT = 256;
constexpr int MAX_COLL_COUNT = 128;
constexpr int VERT_VAL_COUNT = 5;
constexpr int FACE_VAL_COUNT = 3;
constexpr float MAX_DIST = 50;
constexpr float Z_POS = -90;

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

    vec3 editPos = GetEditPos();

    // TODO: support circle colliders
    int raycastCount = 0;
    RaycastPoint rayPoints[MAX_RAY_COUNT] = { };
    int coll_id = 0;

    // Get raycast points for rect colliders
    for (auto it = Collider::rectColliders.begin(); it != Collider::rectColliders.end(); it++)
    {
        if (!(*it)->enabled) continue;

        constexpr float RECT_COLLIDER_POINTS_DELTA = 0.05f;
        (*it)->size += vec2(RECT_COLLIDER_POINTS_DELTA, RECT_COLLIDER_POINTS_DELTA);
        std::vector<vec2> points = (*it)->GetPoints();
        (*it)->size -= vec2(RECT_COLLIDER_POINTS_DELTA, RECT_COLLIDER_POINTS_DELTA);

        for (int j = 0; j < 4; j++)
        {
            rayPoints[raycastCount + j].delta = points[j] - vec2(editPos);
            rayPoints[raycastCount + j].angle = Utility::GetVectorAngle(rayPoints[raycastCount + j].delta);
            rayPoints[raycastCount + j].coll_id = coll_id;
        }

        raycastCount += 4;
        coll_id++;
    }

    // Get raycast points for circle colliders
    for (auto it = Collider::circleColliders.begin(); it != Collider::circleColliders.end(); it++)
    {
        if (!(*it)->enabled) continue;

        constexpr float CIRCLE_COLLIDER_POINTS_DELTA = 0.025f;
        float radius = ((*it)->size / 2) + CIRCLE_COLLIDER_POINTS_DELTA;

        vec2 deltaPos = (*it)->GetPos() - vec2(editPos);
        vec2 tangent = glm::normalize(Rotate(deltaPos, 90));

        vec2 point1 = deltaPos + tangent * radius;
        vec2 point2 = deltaPos - tangent * radius;

        rayPoints[raycastCount].delta = point1;
        rayPoints[raycastCount].angle = Utility::GetVectorAngle(point1);
        rayPoints[raycastCount].coll_id = coll_id;
        rayPoints[raycastCount + 1].delta = point2;
        rayPoints[raycastCount + 1].angle = Utility::GetVectorAngle(point2);
        rayPoints[raycastCount + 1].coll_id = coll_id;

        raycastCount += 2;
        coll_id++;
    }

    // Sort raycast points by angles
    std::sort(rayPoints, rayPoints + raycastCount, [](RaycastPoint a, RaycastPoint b) { return a.angle < b.angle; });

    // Create mesh data
    float vertices[MAX_VERT_COUNT * VERT_VAL_COUNT] = {};
    unsigned int indices[MAX_VERT_COUNT * FACE_VAL_COUNT] = {};

    // Set middle point
    vertices[0] = editPos.x;
    vertices[1] = editPos.y;
    vertices[2] = Z_POS;
    vertices[3] = 0;
    vertices[4] = 1;

    int hitColliders[MAX_COLL_COUNT] = { };
    int hitCollidersCount = 0;

    int vertexCount = 1;
    int faceCount = 0;
    for (int i = 0; i < raycastCount; i++)
    {
        vec2 dir = rayPoints[i].delta;
        vec2 res;

        if (!Collider::Raycast(vec2(editPos), dir, &res))
        {
            // If nothig is hit, use max distance
            res = vec2(editPos) + glm::normalize(dir) * MAX_DIST;
        }

        // If the ray hit behind the point, this is a corner
        vec2 raycastHitDelta = res - vec2(editPos);
        float targetSqrDist = dir.x * dir.x + dir.y * dir.y;
        float raycastSqrDist = raycastHitDelta.x * raycastHitDelta.x + raycastHitDelta.y * raycastHitDelta.y;
        if (raycastSqrDist > targetSqrDist + 0.03)
        {
            bool alreadyHit = false;
            for (int coll = 0; coll < hitCollidersCount; coll++)
            {
                if (hitColliders[coll] == rayPoints[i].coll_id)
                {
                    alreadyHit = true;
                    break;
                }
            }

            if (alreadyHit)
            {
                CreateVertex(vertices, &vertexCount, dir + vec2(editPos));
                if (i > 0) CreateTriangle(indices, &vertexCount, &faceCount, raycastCount);
                CreateVertex(vertices, &vertexCount, res);
            }
            else
            {
                CreateVertex(vertices, &vertexCount, res);
                if (i > 0) CreateTriangle(indices, &vertexCount, &faceCount, raycastCount);
                CreateVertex(vertices, &vertexCount, dir + vec2(editPos));
            }
        }
        else
        {
            CreateVertex(vertices, &vertexCount, res);
            if (i > 0) CreateTriangle(indices, &vertexCount, &faceCount, raycastCount);

            hitColliders[hitCollidersCount] = rayPoints[i].coll_id;
            hitCollidersCount++;
        }
    }

    std::printf("Radioactivity source baked with %d raycasts and %d vertices\n", raycastCount, vertexCount);
    mesh = new Mesh(vertices, vertexCount * VERT_VAL_COUNT, indices, faceCount * FACE_VAL_COUNT);

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

void Radioactivity::CreateVertex(float* vertices, int* vertexCount, vec2 pos)
{
    int vtPos = (*vertexCount) * VERT_VAL_COUNT;
    vertices[vtPos] = pos.x;
    vertices[vtPos + 1] = pos.y;
    vertices[vtPos + 2] = Z_POS;
    vertices[vtPos + 3] = 0;
    vertices[vtPos + 4] = 0;
    (*vertexCount)++;
}

void Radioactivity::CreateTriangle(unsigned int* indices, int* vertexCount, int* faceCount, int raycastCount)
{
    // Create face
    int idPos = (*faceCount) * FACE_VAL_COUNT;
    indices[idPos] = 0;
    indices[idPos + 1] = (*vertexCount) - 2;

    if (*faceCount == raycastCount - 2)
        indices[idPos + 2] = 1;
    else
        indices[idPos + 2] = (*vertexCount) - 1;

    (*faceCount)++;
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

    if (enabled)
    {
        if (isRealTime)
            ForceRefreshMesh();

        glDisable(GL_DEPTH_TEST);

        Shader* shader = &RessourceManager::shaders["radioactivity"];
        shader->Use();
        shader->SetUniform("projection", Camera::GetOrthographicProjection());
        shader->SetUniform("transform", glm::mat4(1.0f));
        GetMesh()->DrawMesh();

        glEnable(GL_DEPTH_TEST);
    }
}
