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

AABB RectToAABB(Rect *rect)
{
    AABB result = {};
    result.center = V2(rect->basePosition);
    result.halfDim = v2{rect->width, rect->height} * 0.5f;
    return result;
}

f32 Randf32(u32 range)
{
    u64 randomNumber = __rdtsc();
    u32 mod = randomNumber % range;
    return (f32)mod;
}


f32 Radians(f32 deg)
{
    return  deg * (f32)M_PI / 180.0f;
}

glm::mat4 PerspectiveProjectionMatrix(f32 fov, f32 aspectRatio, f32 zNear, f32 zFar)
{
    /* XXX: (Vulkan) This perspective projection matrix is used when the z-far <-> z-near goes from 0 to 1 */
    f32 f = 1.0f / tan(fov * 0.5f);
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

glm::vec3 ScreenSpaceToNormalizedDeviceSpace(v2 pixelCoords, v2 screenResolution)
{
    glm::vec2 normalizedMME = {pixelCoords.x / screenResolution.x, pixelCoords.y / screenResolution.y};
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
        ScreenSpaceToNormalizedDeviceSpace(screenCoords, screenResolution);

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

/* TODO: The float values between GetRayFromMouse and UnProject might be slightly different.
 * Might need to do an accuracy check.
 */
glm::vec3 UnProject(Camera *camera, glm::mat4 *projection, v2 screenCoords, v2 screenResolution)
{
    /* XXX: Both `UnProject` and `GetRayFromMouse` equivalently does the same
     * thing - doing an UnProject. In just a slightly different approach. One
     * might seen more intuitive than the other.
     */
    glm::vec3 rayNormalizedDeviceCoords =
        ScreenSpaceToNormalizedDeviceSpace(screenCoords, screenResolution);

    glm::mat4 inPV = glm::inverse( *projection * camera->view);

    /* XXX: Based on our viewport, our znear -> zfar goes from 0 to 1 (I think) */
    glm::vec4 zNear = glm::vec4(rayNormalizedDeviceCoords.xy, 0.0f, 1.0f);
    glm::vec4 zFar = glm::vec4(rayNormalizedDeviceCoords.xy, 1.0f, 1.0f);

    glm::vec4 rayZNearXYZW = inPV * zNear;
    glm::vec4 rayZFarXYZW = inPV * zFar;

    /* XXX: Perspective divide - This will make things further smaller in terms
     * of points and and the things closer bigger.
     * For example (not accurate):
     * 1,   1,   1      -> 1.5,  1.5,  1.5
     * 100, 100, 100    -> 0.01, 0.01, 0.01,
     */
    glm::vec3 rayZNear = glm::vec3(rayZNearXYZW.xyz) / rayZNearXYZW.w;
    glm::vec3 rayZFar = glm::vec3(rayZFarXYZW.xyz) / rayZFarXYZW.w;

#if 0
    /* XXX: Needed if we actaully care about the max distance of a ray. Might
     * be useful for object look up
     */.
    f32 d = glm::length(glm::cross(rayZNear, rayZFar)) / glm::length(rayZFar -
            rayZNear);
#endif

    return glm::normalize(rayZFar - rayZNear);
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
#if 0
    glm::vec3 rayWorld = GetRayFromMouse(camera, projection, screenCoords, screenResolution);
#else
    glm::vec3 rayWorld = UnProject(camera, projection, screenCoords, screenResolution);
#endif

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
