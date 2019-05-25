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

SceneManager *CreateSceneManager(GameMetadata *gameMetadata, GameMemory *gameMemory)
{
    SceneManager *result = (SceneManager *)AllocateMemory(gameMemory, sizeof(SceneManager));
    memset(result, 0, sizeof(SceneManager));
    result->perFrameMemory = gameMemory;
    result->gameMetadata = gameMetadata;

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
    result[2][2] = zFar / (zNear - zFar);
    result[2][3] = -1;

    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    result[3][2] = (zNear * zFar) / (zNear - zFar);
    result[3][3] = 0.0f;

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

void PushRectDynamicArrayToRenderGroupRectInfo(
        GameMetadata *gameMetadata, RenderGroup *perFrameRenderGroup, RectDynamicArray *rda)
{
    for (memory_index i = 0; i < rda->size; i++)
    {
        Rect *rect = rda->rects[i];
        rect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, rect->bitmapID);
        ASSERT(rect->bitmap);
        PushRenderGroupRectInfo(perFrameRenderGroup, rect);
    }
}

b32 IsWithinThreshold(v2 a, v2 b, f32 thresholdValueInScreenCoordinates)
{
    v2 result = abs(a - b);

    return result.x < thresholdValueInScreenCoordinates && result.y < thresholdValueInScreenCoordinates;
}

void ProcessMouseEditMode(GameMetadata *gm, Camera *camera, glm::mat4 *projection, SDL_Event &event)
{
    if (!gm->isEditMode)
    {
        return;
    }

    SDL_MouseButtonEvent mbe = event.button;
    v2 screenCoordinates = V2(GetScreenCoordinateFromMouse(event.motion));
    glm::vec3 infinitePlaneNormal = glm::vec3(0, 0, 1);

    glm::vec3 worldPos = {};

    switch (mbe.button)
    {
        /* AddNewRectToWorld */
        case SDL_BUTTON_LEFT:
            worldPos =
                GetWorldPointFromMouse(
                        camera,
                        projection,
                        screenCoordinates,
                        gm->screenResolution,
                        infinitePlaneNormal);
            if (mbe.state == SDL_PRESSED && gm->isLeftButtonReleased)
            {
                gm->screenCoordinates[0] = screenCoordinates;
                gm->leftMouseDrag[0] = worldPos;

                gm->isLeftButtonReleased = false;
            }
            else if (mbe.state == SDL_RELEASED)
            {
                gm->isLeftButtonReleased = true;

                gm->leftMouseDrag[1] = worldPos;
                gm->screenCoordinates[1] = screenCoordinates;

                f32 screenCoordinatesThresholdValue = 17.0f;
                if (IsWithinThreshold(gm->screenCoordinates[0], gm->screenCoordinates[1], screenCoordinatesThresholdValue))
                {
                    ARRAY_PUSH(glm::vec3, &gm->reservedMemory, gm->objectsToBeAddedTotheWorld, worldPos);
                }

                gm->createNewRect = true;
            }
            break;
        case SDL_BUTTON_RIGHT:
            if (mbe.state == SDL_PRESSED && gm->isLeftButtonReleased)
            {
                gm->isRightButtonReleased = false;
            }
            else if (mbe.state == SDL_RELEASED)
            {
                worldPos =
                    GetWorldPointFromMouse(
                            camera,
                            projection,
                            screenCoordinates,
                            gm->screenResolution,
                            infinitePlaneNormal);
                gm->rightMouseButton = worldPos;
                gm->isRightButtonReleased = true;

                gm->willSelectObject = true;
            }
            break;
        default:
            break;
    }


}

void UpdateMouseDrag(GameMetadata *gm, Camera *camera, glm::mat4 *projection, SDL_Event &event)
{
    if (!gm->isLeftButtonReleased)
    {
        SDL_MouseButtonEvent mbe = event.button;
        v2 screenCoordinates = V2(GetScreenCoordinateFromMouse(event.motion));

        glm::vec3 infinitePlaneNormal = glm::vec3(0, 0, 1);

        glm::vec3 worldPos =
            GetWorldPointFromMouse(
                    camera,
                    projection,
                    screenCoordinates,
                    gm->screenResolution,
                    infinitePlaneNormal);

        gm->leftMouseDrag[1] = worldPos;
    }
}

RectDynamicArray *CreateRDAForNewWorldObjects(GameMetadata *gm)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    RectDynamicArray *result = CreateRectDynamicArray(perFrameMemory);

    for (memory_index i = 0; i < ARRAY_LIST_SIZE(gm->objectsToBeAddedTotheWorld); i++)
    {
        glm::vec3 pos = gm->objectsToBeAddedTotheWorld[i];
        Rect *rect = CreateRectangle(perFrameMemory, V3(pos), COLOR_BLUE_TRANSPARENT, 1, 1);
        rect->bitmapID = 0;
        PushBack(result, rect);
    }

    return result;
}

void SetUVForCharacter(Rect *rect, s32 character)
{
    Bitmap *bitmap = rect->bitmap;
    ASSERT(bitmap != nullptr);

    Vertex *vTopRight = &(rect->vertices[0]);
    Vertex *vBottomRight = &(rect->vertices[1]);
    Vertex *vBottomLeft = &(rect->vertices[2]);
    Vertex *topLeft = &(rect->vertices[3]);

    FontBitmapInfo fontInfo = GetFontPixelInfo(character);
    v2 start = fontInfo.pixelStart;
    v2 end = fontInfo.pixelEnd;

    vTopRight->vUv = PixelToUV(
            end,
            bitmap->width, bitmap->height);
    vBottomRight->vUv = PixelToUV(
            v2{end.x, 0},
            bitmap->width, bitmap->height);
    vBottomLeft->vUv = PixelToUV(
            v2{start.x, 0},
            bitmap->width, bitmap->height);
    topLeft->vUv = PixelToUV(
            start,
            bitmap->width, bitmap->height);
}

void PushStringRectToRenderGroup(RenderGroup *perFrameRenderGroup, GameMetadata *gameMetadata, GameMemory *gameMemory, v3 startingPosition, f32 scale, const char *string)
{
    v3 position = startingPosition;
    f32 screenWidth = gameMetadata->screenResolution.v[0];
    f32 screenHeight = gameMetadata->screenResolution.v[1];

    f32 extraPixelPadding = 0;
    f32 pixelWidthOfA = 83 * 0.5f;
    for(memory_index i = 0; i < strlen(string); i++)
    {
        s32 character = string[i];
        if (character == ' ')
        {
            position.x += pixelWidthOfA / screenWidth * scale;
            continue;
        }

        FontBitmapInfo fontInfo = GetFontPixelInfo(character);
        v2 start = fontInfo.pixelStart;
        v2 end = fontInfo.pixelEnd;
        f32 xWidth = end.x - start.x;

        if (character == 'a' || character == 'f' || character == 'd' || character == 'i' || character == 'c' || character == 's' || character == 'l' || character == 'I')
        {
            position.x -= 50 / screenWidth * scale;
        }
        else if (character == 't' || character == 'y')
        {
            position.x -= 20 / screenWidth * scale;
        }
        else if (character == '/')
        {
            position.x -= 40 / screenWidth * scale;
        }

        Rect *editModeRect =
            CreateRectangle(gameMemory, position, COLOR_WHITE, xWidth / screenWidth * scale, gameMetadata->fontBitmap.height / screenHeight * scale);
        editModeRect->bitmapID = gameMetadata->fontBitmap.bitmapID;
        editModeRect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "font");

        if (character == 'I')
        {
            extraPixelPadding = 25;
        }
        else if (character == 'E')
        {
            extraPixelPadding = 40;
        }
        else if (character == '.')
        {
            extraPixelPadding = 60;
        }

        position.x += (xWidth + extraPixelPadding) / screenWidth * scale;

        SetUVForCharacter(editModeRect, character);
        PushRenderGroupRectInfo(perFrameRenderGroup, editModeRect);
    }
}
