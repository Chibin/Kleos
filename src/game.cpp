#include "game.h"
#include "game_memory.h"
#include "game_metadata.h"
#include "logger.h"
#include "main.h"
#include <GL/glew.h>

#include "renderer/vulkan/my_vulkan.cpp"

void MainGameLoop(SDL_Window *mainWindow, RenderAPI *renderAPI)
{
    /* sanity check */
    ASSERT(sizeof(real32) == sizeof(GLfloat));

    /* TODO: this is probably not the right spot for this */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    bool continueRunning = true;

    struct GameMetadata gameMetadata = {};
    gameMetadata.font = nullptr;
    gameMetadata.maxBlockSize = GIGABYTE(1);
    gameMetadata.base = (u8 *)malloc(gameMetadata.maxBlockSize);

    u32 tempSize = MEGABYTE(50);
    gameMetadata.temporaryMemory.base = (u8 *)AllocateMemory(&gameMetadata, tempSize);
    gameMetadata.temporaryMemory.maxSize = tempSize;

    u32 reservedSize = MEGABYTE(900);
    gameMetadata.reservedMemory.base = (u8 *)AllocateMemory(&gameMetadata, reservedSize);
    gameMetadata.reservedMemory.maxSize = reservedSize;

    gameMetadata.screenResolution = v2{ SCREEN_WIDTH, SCREEN_HEIGHT };
    gameMetadata.initFromGameUpdateAndRender = false;

    gameMetadata.bitmapSentinelNode = {};
    gameMetadata.bitmapSentinelNode.next = &gameMetadata.bitmapSentinelNode;
    gameMetadata.bitmapSentinelNode.prev = &gameMetadata.bitmapSentinelNode;

    gameMetadata.frameAnimationSentinelNode = {};
    gameMetadata.frameAnimationSentinelNode.next = &gameMetadata.frameAnimationSentinelNode;
    gameMetadata.frameAnimationSentinelNode.prev = &gameMetadata.frameAnimationSentinelNode;

    gameMetadata.isLeftButtonReleased = true;
    gameMetadata.createNewRect = false;

    gameMetadata.isEditMode = true;

    FindFile(GetProgramPath(), "render*dll");

    /* We still want to initialize vulkan. This is so that we have the
     * capability to switch between OpenGl and Vulkan.
     */
    VulkanContext *vc = nullptr;
    vc = VulkanSetup(&mainWindow);
    gameMetadata.vulkanContext = vc;

    while (continueRunning && !vc->quit)
    {
        continueRunning = ((renderAPI->updateAndRender)(&gameMetadata) != 0);
    }
}
