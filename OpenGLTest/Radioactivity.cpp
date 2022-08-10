#include "Radioactivity.h"

#include "EditorObject.h"
#include "Sprite.h"
#include "CircleCollider.h"
#include "RectCollider.h"
#include "RessourceManager.h"
#include "Player.h"
#include <cmath>

constexpr int MAX_VERT_COUNT = 256;
constexpr int MAX_RAY_COUNT = 256;
constexpr int MAX_COLL_COUNT = 128;
constexpr int VERT_VAL_COUNT = 5;
constexpr int FACE_VAL_COUNT = 3;
constexpr float Z_POS = 90;

ObjectEvent Radioactivity::events[RADIOACTIVITY_EVENT_COUNT] = {
    ObjectEvent {
        "Enable realtime",
        [](EditorObject* object, void* param) { ((Radioactivity*)object)->isRealTime = true; },
    },
    ObjectEvent {
        "Disable realtime",
        [](EditorObject* object, void* param) { ((Radioactivity*)object)->isRealTime = false; },
    },
};

Radioactivity::Radioactivity() : EditorObject(vec3(0))
{
	clickCollider = new CircleCollider(vec2(0), 1, false);

    if (Editor::enabled)
    {
        editorSprite = new Sprite(RessourceManager::GetTexture("Engine\\radioactivity.png"), vec3(0), vec2(1), 0);
        editorSprite->DrawOnMainLoop();
    }
            
	typeName = "Radioactivity";

    mainLoopFuncPos = EventManager::OnAfterMainLoop.push_end([this] { this->OnAfterMainLoop(); });
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

    EventManager::OnAfterMainLoop.remove(mainLoopFuncPos);
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

    drawPos.y -= Editor::DrawProperty(drawPos, "Max distanse", &maxDist, Editor::panelPropertiesX, strID + "maxDist").y;
    drawPos.y -= Editor::CheckBox(drawPos, "Is realtime", &isRealTime, Editor::panelPropertiesX).y;

    EditorObject* obj = Editor::GetEditorObjectByID(regionID, true, false);
    drawPos.y -= Editor::ObjectSelector(drawPos, "Region", &obj, Editor::panelPropertiesX, strID + "region").y;
    if (obj != nullptr)
        regionID = obj->ID;
    else regionID = -1;

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

    newObj->mainLoopFuncPos = EventManager::OnAfterMainLoop.push_end([newObj] { newObj->OnAfterMainLoop(); });
    newObj->mesh = nullptr;

	return newObj;
}

void Radioactivity::Load(std::map<std::string, std::string>* props)
{
	EditorObject::Load(props);

    EditorSaveManager::FloatProp(props, "maxDist", &maxDist);
    EditorSaveManager::IntProp(props, "regionID", &regionID);
    isRealTime = (*props)["isRealTime"] == "1";

    startedRealtime = isRealTime;
}

void Radioactivity::Save()
{
	EditorObject::Save();

    EditorSaveManager::WriteProp("maxDist", maxDist);
    EditorSaveManager::WriteProp("isRealTime", isRealTime);
    EditorSaveManager::WriteProp("regionID", regionID);
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

void Radioactivity::GetObjectEvents(const ObjectEvent** res, int* resCount)
{
    *res = events;
    *resCount = RADIOACTIVITY_EVENT_COUNT;
}

void Radioactivity::ResetIngameState()
{
    isRealTime = startedRealtime;
}

Mesh* Radioactivity::GetMesh()
{
    if (mesh != nullptr) return mesh;

    vec3 editPos = GetEditPos();

    int raycastCount = 0;
    RaycastPoint rayPoints[MAX_RAY_COUNT] = { };
    int coll_id = 0;

    EditorSprite* region = nullptr;
    if (regionID != -1)
    {
        EditorObject* obj = Editor::GetEditorObjectByIDInObjectContext(this, regionID, Editor::enabled, false);

        if (obj != nullptr)
            region = (EditorSprite*)obj;
    }

    //auto begin = std::chrono::high_resolution_clock::now();

    // Get raycast points for rect colliders
    for (auto it = Collider::rectColliders.begin(); it != Collider::rectColliders.end(); it++)
    {
        if (!(*it)->enabled) continue; // Ignore if disabled

        vec2 delta = (*it)->GetPos() - vec2(editPos);
        float sqrDist = delta.x * delta.x + delta.y * delta.y;
        if (sqrDist > maxDist * maxDist) continue; // Ignore if too far

         // Grow the collider before getting the points, to make sure the raycast alway passes
        constexpr float RECT_COLLIDER_POINTS_DELTA = 0.05f;
        (*it)->size += vec2(RECT_COLLIDER_POINTS_DELTA, RECT_COLLIDER_POINTS_DELTA);
        std::vector<vec2> points = (*it)->GetPoints();
        (*it)->size -= vec2(RECT_COLLIDER_POINTS_DELTA, RECT_COLLIDER_POINTS_DELTA);

        for (int j = 0; j < 4; j++) // Add targets
        {
            CircleCollider coll(points[j], 0.01, false);
            if (region == nullptr || region->clickCollider->CollideWith(&coll).z != 0)
            {
                rayPoints[raycastCount].delta = points[j] - vec2(editPos);
                rayPoints[raycastCount].angle = Utility::GetVectorAngle(rayPoints[raycastCount].delta);
                rayPoints[raycastCount].coll_id = coll_id;
                raycastCount += 1;
            }
        }

        coll_id++;
    }

    // Get raycast points for circle colliders
    for (auto it = Collider::circleColliders.begin(); it != Collider::circleColliders.end(); it++)
    {
        if (!(*it)->enabled) continue; // Ignore if disabled

        vec2 delta = (*it)->GetPos() - vec2(editPos);
        float sqrDist = delta.x * delta.x + delta.y * delta.y;
        if (sqrDist > maxDist * maxDist) continue; // Ignore if too far

        constexpr float CIRCLE_COLLIDER_POINTS_DELTA = 0.025f;
        float radius = ((*it)->size / 2) + CIRCLE_COLLIDER_POINTS_DELTA; // A little bit bigger, to make sure the raycasts passes

        // Get points
        vec2 deltaPos = (*it)->GetPos() - vec2(editPos);
        vec2 tangent = glm::normalize(Rotate(deltaPos, 90));

        vec2 point1 = deltaPos + tangent * radius;
        vec2 point2 = deltaPos - tangent * radius;

        // Add targets
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

    // Array containing all colliders that we have already hit
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
            res = vec2(editPos) + glm::normalize(dir) * maxDist;
        }

        vec2 raycastHitDelta = res - vec2(editPos);
        float targetSqrDist = dir.x * dir.x + dir.y * dir.y;
        float raycastSqrDist = raycastHitDelta.x * raycastHitDelta.x + raycastHitDelta.y * raycastHitDelta.y;
        if (raycastSqrDist > targetSqrDist + 0.03) // If the ray hit behind the point, this is a corner
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
                hitColliders[hitCollidersCount] = rayPoints[i].coll_id;
                hitCollidersCount++;
            }
        }
        else // Normal procedure
        {
            CreateVertex(vertices, &vertexCount, res);
            if (i > 0) CreateTriangle(indices, &vertexCount, &faceCount, raycastCount);
            hitColliders[hitCollidersCount] = rayPoints[i].coll_id;
            hitCollidersCount++;
        }
    }

    //auto end = std::chrono::high_resolution_clock::now();
    //int duration = (int)std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    //std::printf("%d microseconds\n", duration);

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

void Radioactivity::OnAfterMainLoop()
{
    if (!enabled) return;

    // Do not refresh if camera is too far
    vec2 camDelta = Camera::position - vec2(GetEditPos());
    float camSqrDist = camDelta.x * camDelta.x + camDelta.y * camDelta.y;
    if (camSqrDist < std::pow(maxDist + Camera::size, 2))
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

        if (Player::ingameInstance != nullptr)
        {
            // Kill player
            vec2 dir = Player::ingameInstance->GetPos() - vec2(GetEditPos());
            Collider* coll;

            if (abs(dir.x) > 0.02) // TEST: prevent raycast error when almost vertical
            if (Collider::Raycast(GetEditPos(), dir, nullptr, nullptr, &coll))
            {
                if (coll == Player::ingameInstance->collider)
                {
                    Player::ingameInstance->Kill();
                }
            }
        }
    }
}

void Radioactivity::OnMainLoop()
{
	EditorObject::OnMainLoop();
    if (!enabled) return;
    
    if (editorSprite != nullptr)
    {
        editorSprite->position = GetEditPos();
        editorSprite->size = vec2(Editor::gizmoSize);
        ((CircleCollider*)clickCollider)->size = Editor::gizmoSize;
    }
}
