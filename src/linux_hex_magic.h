#if !defined(LINUX_HEX_MAGIC_H)

#include "hex_magic.h"
#include <SDL2/SDL.h>
#include <linux/limits.h>

struct LinuxOffscreenBuffer
{
    SDL_Texture *renderTexture;
    void *memory;
    int bytesPerPixel;
    int width;
    int height;
    int pitch;
};

struct LinuxAudioRingBuffer
{
    uint32 size;
    uint32 writeCursor;
    uint32 playCursor;
    void *data;
};

struct LinuxSoundOutput
{
    int samplesPerSecond;
    uint32 runningSampleIndex;
    int bytesPerSample;
    uint32 secondaryBufferSize;
    uint32 safetyBytes;
    int latencySampleCount;
};

struct LinuxDebugTimeMarker
{
    uint32 outputPlayCursor;
    uint32 outputWriteCursor;
    uint32 outputLocation;
    uint32 outputByteCount;
    uint32 expectedFlipCursor;

    uint32 flipPlayCursor;
    uint32 flipWriteCursor;
};

struct LinuxGameCode
{
    void *gameLib;
    struct timespec lastWriteTime;
    GameUpdateAndRender *updateAndRender;
    GameGetSoundSamples *getSoundSamples;

    bool32 isValid;
};

struct LinuxReplayBuffer
{
    int fileHandle;
    char fileName[PATH_MAX];
    void *memoryBlock;
};

struct LinuxState
{
    uint64 totalSize;
    void *gameMemoryBlock;

    char *executableFileName;
    char *onePastLastExecFileNameSlash;

    int windowWidth;
    int windowHeight;
    bool32 isFullscreen;

    bool32 showCursor;
    SDL_Cursor *cursor;
};

#define LINUX_HEX_MAGIC_H
#endif
