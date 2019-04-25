#include "scene_node.h"
#include "array.h"

v2 V2(glm::vec3 A)
{
    v2 result;
    result.x = A.x;
    result.y = A.y;
    return result;

}

v3 V3(glm::vec3 A)
{
    v3 result;
    result.x = A.x;
    result.y = A.y;
    result.z = A.z;
    return result;

}

struct SceneManager
{
    /* Determines the legnth of each square to partition */
    f32 unitLength;
    SceneNode *rootSceneNode;
    AABB aabb;

    GameMemory *perFrameMemory;
    GameMetadata *gameMetadata;
};

SceneNode *CreateSceneNode(GameMemory *gm, AABB *aabb)
{
    SceneNode *result = (SceneNode *)AllocateMemory(gm, sizeof(SceneNode));
    memset(result, 0, sizeof(SceneNode));
    result->northWest = nullptr;
    result->northEast = nullptr;
    result->southWest = nullptr;
    result->southEast = nullptr;

    result->aabb = *aabb;

    return result;
}

AABB MinMaxToSquareAABB(MinMax *minMax)
{
    AABB result = {};

    v2 dim = (minMax->max - minMax->min);
    v2 center = minMax->max - dim * 0.5f;

    v2 squareDim = {};
    squareDim.x = dim.x < dim.y ? dim.y : dim.x;
    squareDim.y = dim.y < dim.x ? dim.x : dim.y;

    result.center = center;
    result.halfDim = squareDim * 0.5f;

    return result;
}

void SubDivide(SceneManager *sm, SceneNode *sn)
{

    AABB aabbNorthWest = {};
    AABB aabbNorthEast = {};
    AABB aabbSouthWest = {};
    AABB aabbSouthEast = {};

    aabbNorthWest.halfDim =
    aabbNorthEast.halfDim =
    aabbSouthWest.halfDim =
    aabbSouthEast.halfDim = sn->aabb.halfDim * 0.5f;

    aabbNorthWest.center = sn->aabb.center + v2{-aabbNorthWest.halfDim.x, aabbNorthWest.halfDim.y};
    aabbNorthEast.center = sn->aabb.center + v2{aabbNorthWest.halfDim.x, aabbNorthWest.halfDim.y};
    aabbSouthWest.center = sn->aabb.center + v2{-aabbNorthWest.halfDim.x, -aabbNorthWest.halfDim.y};
    aabbSouthEast.center = sn->aabb.center + v2{aabbNorthWest.halfDim.x, -aabbNorthWest.halfDim.y};

    sn->northWest = CreateSceneNode(sm->perFrameMemory, &aabbNorthWest);
    sn->northEast = CreateSceneNode(sm->perFrameMemory, &aabbNorthEast);
    sn->southWest = CreateSceneNode(sm->perFrameMemory, &aabbSouthWest);
    sn->southEast = CreateSceneNode(sm->perFrameMemory, &aabbSouthEast);
}

AABB RectToAABB(Rect *rect)
{
    AABB result = {};
    result.center = V2(rect->basePosition);
    result.halfDim = v2{rect->width, rect->height} * 0.5f;
    return result;
}

f32 Randf32(u32 range)
{
    u64 randomNumber = __rdtsc();                                                            \
    u32 mod = randomNumber % range;
    return (f32)mod;
}

b32 InsertToScenePartition(SceneManager *sm, SceneNode *sn, Rect *rect)
{
    //dimension of each partition
    //insert to the specific partition
    //generate an ID for each specific dimension
    //
    ASSERT(sm->perFrameMemory != nullptr);

    if(TestAABBAABB(&sn->aabb, &RectToAABB(rect)) == false)
    {
        return false;
    }

    if (sn->rect == nullptr)
    {
#if 0
        AddDebugRect(sm->gameMetadata, &sn->aabb, v4{Randf32(255)/255.0f, Randf32(255)/255.0f, Randf32(255)/255.0f, 1.0f});
#endif
        sn->rect = rect;
        return true;
    }

    if(sn->northWest == nullptr)
    {
        SubDivide(sm, sn);
    }

#if 0
    AddDebugRect(sm->gameMetadata, &sn->northWest->aabb, COLOR_BLUE);
    AddDebugRect(sm->gameMetadata, &sn->northEast->aabb, COLOR_GREEN);
    AddDebugRect(sm->gameMetadata, &sn->southWest->aabb, COLOR_YELLOW);
    AddDebugRect(sm->gameMetadata, &sn->southEast->aabb, COLOR_RED);
#endif

    if(InsertToScenePartition(sm, sn->northWest, rect))
    {
        return true;
    }
    if(InsertToScenePartition(sm, sn->northEast, rect))
    {
        return true;
    }
    if(InsertToScenePartition(sm, sn->southWest, rect))
    {
        return true;
    }
    if(InsertToScenePartition(sm, sn->southEast, rect))
    {
        return true;
    }

    return false;
}

void CreateScenePartition(SceneManager *sm, RectStorage *rs)
{
    sm->aabb = MinMaxToSquareAABB(&rs->aabbMinMax);

#if 0
    AddDebugRect(sm->gameMetadata, &sm->aabb, COLOR_YELLOW);
#endif

    sm->rootSceneNode = CreateSceneNode(sm->perFrameMemory, &sm->aabb);

    for(memory_index i = 0; i < rs->rda.size; i++)
    {
        InsertToScenePartition(sm, sm->rootSceneNode, rs->rda.rects[i]);
    }
}

void QueryRange(SceneManager *sm, SceneNode *sn, Rect ***arr, AABB *range)
{
    if (sn == nullptr)
    {
        return;
    }

    if (TestAABBAABB(&sn->aabb, range) && sn->rect != nullptr)
    {
        ARRAY_PUSH(Rect **, sm->perFrameMemory, *arr, sn->rect);
    }

    QueryRange(sm, sn->northWest, arr, range);
    QueryRange(sm, sn->northEast, arr, range);
    QueryRange(sm, sn->southWest, arr, range);
    QueryRange(sm, sn->southEast, arr, range);

}

Rect **GetRectsWithInRange(SceneManager *sm, AABB *range)
{
    Rect **arr = nullptr;
    ARRAY_CREATE(Rect **, sm->perFrameMemory, arr);

    QueryRange(sm, sm->rootSceneNode, &arr, range);

    return arr;
}

f32 DegToRad(f32 deg)
{
    return  deg * (f32)M_PI / 180.0f;
}

glm::mat4 PerspectiveProjectionMatrix(f32 fov, f32 aspectRatio, f32 zNear, f32 zFar)
{
    /* XXX: This perspective projection matrix is used when the Z-Axis goes from -1 to 1 */

    f32 f = 1.0f / tan(DegToRad(fov) * 0.5f);
    glm::mat4 result = glm::mat4(0.0f);

    result[0][0] = f / aspectRatio;
    result[0][1] = 0.0f;
    result[0][2] = 0.0f;
    result[0][3] = 0.0f;

    result[1][0] = 0.0f;
    result[1][1] = f;
    result[1][2] = 0.0f;
    result[1][3] = 0.0f;

    result[2][0] = 0.0f;
    result[2][1] = 0.0f;
    result[2][2] = (zFar + zNear) / (zNear - zFar);
    result[2][3] = (2.0f * zFar * zNear) / (zNear - zFar);

    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    result[3][2] = -1;
    result[3][3] = 0.0f;

    return result;

}

glm::mat4 InversePerspectiveProjectionMatrix(f32 fov, f32 aspectRatio, f32 zNear, f32 zFar)
{
    /* XXX: This inverse perspective projection matrix is used when the Z-Axis
     * goes from -1 to 1
     */
    f32 f = 1.0f / tan(DegToRad(fov) * 0.5f);
    glm::mat4 result = glm::mat4(0.0f);

    result[0][0] = aspectRatio / f;
    result[0][1] = 0.0f;
    result[0][2] = 0.0f;
    result[0][3] = 0.0f;

    result[1][0] = 0.0f;
    result[1][1] = 1 / f;
    result[1][2] = 0.0f;
    result[1][3] = 0.0f;

    result[2][0] = 0.0f;
    result[2][1] = 0.0f;
    result[2][2] = 0;
    result[2][3] = -1;

    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    result[3][2] = (zNear - zFar) / (2.0f * zFar * zNear);
    result[3][3] = (zFar + zNear) / (2.0f * zFar * zNear);

    return result;
}

glm::vec3 ScreenSpaceToNormalizedDeviceSpace(v2 pixelCoords, f32 screenWidth, f32 screenHeight)
{
    glm::vec2 normalizedMME = {pixelCoords.x / screenWidth, pixelCoords.y / screenHeight};
    glm::vec2 result = 2.0f * normalizedMME - 1.0f;
    /* XXX:We have to flip this to the negative value due to our viewport being
     * viewed in the negative direction.  Check OpenGL NDC coordinates for more
     * details.
     */
    result.y = -result.y;

    return glm::vec3(result, 1.0f);
}

glm::vec4 ClipSpaceToViewSpace(glm::vec4 ndc, glm::mat4 *projection)
{
    glm::mat4 invProjection = glm::inverse(*projection);
    return invProjection * ndc;
}

/* This the returned value will be normalized */
glm::vec3 ViewSpaceToWorldSpaceRaycast(Camera *camera, glm::vec4 rayEye)
{
    glm::vec3 rayWorld = (glm::inverse(camera->view) * rayEye).xyz;
    rayWorld = glm::normalize(rayWorld);

#if 0
    printf("coords: X %f, Y %f coords: Z %f\n", rayWorld.x, rayWorld.y, rayWorld.z);
#endif

    return rayWorld;
}

glm::vec3 GetRayFromMouse(Camera *camera, glm::mat4 *projection, v2 screenCoords, v2 screenResolution)
{
    glm::vec3 rayNormalizedDeviceCoords =
        ScreenSpaceToNormalizedDeviceSpace(screenCoords, screenResolution.x, screenResolution.y);

    /* XXX: There's usually a reverse of a perspective division here, but we're
     * unprojecting a ray instead of a point, we don't need to do that process.
     * The idea is that we multiply by the `w` value to our rayclip xyz, if we
     * do have to reverse the perspective division. */
    /* XXX: We're using -1.0f for the Z-axis to face forward. This is because
     * Z-far goes towards the negative direction for OpenGL. (May apply to our
     * current setup due to the vkViewport changes we made.
     */
    /* To clip space */
    glm::vec4 rayClip = glm::vec4(rayNormalizedDeviceCoords.xy, -1.0f, 0.0f);
    /* XXX: We're resetting the z-axis here, because we're not unprojecting it.
     * Make the ray face forward with -1 on the z-axis.
     * Use 0 for w, since it's not needed either.
     */
    glm::vec4 rayEye = glm::vec4(ClipSpaceToViewSpace(rayClip, projection).xy, -1.0f, 0.0f);
    glm::vec3 rayWorld = ViewSpaceToWorldSpaceRaycast(camera, rayEye);

    return rayWorld;
}

glm::vec3 GetWorldPointFromMouse(
        Camera *camera,
        glm::mat4 *projection,
        v2 screenCoords,
        v2 screenResolution,
        glm::vec3 infinitePlaneNormal)
{

    /* XXX: arbitrary position since it's infinite anyways. */
    glm::vec3 infinitePlanePos = glm::vec3(0, 0, 0);
    glm::vec3 rayWorld = GetRayFromMouse(camera, projection, screenCoords, screenResolution);

    /* XXX: rayWorld is just a normalized vector of the ray, which means that it doesn't have a starting location.
     * We're setting the camera's position as the starting location. This is
     * because we're viewing from the camera to the projected world space.
     */
    glm::vec3 rayStart = camera->pos;
    glm::vec3 rayEnd = camera->pos + rayWorld;
    glm::vec3 rayDelta = rayEnd - rayStart;
    glm::vec3 rayToPlaneDelta = infinitePlanePos - rayStart;
    f32 scale = glm::dot(infinitePlaneNormal, rayToPlaneDelta) / glm::dot(infinitePlaneNormal, rayDelta);

    glm::vec3 intersection = rayStart + scale * rayDelta;
    glm::vec3 worldPos = intersection;

    return worldPos;
}
