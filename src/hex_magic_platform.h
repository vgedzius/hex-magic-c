#if !defined(HEX_MAGIC_PLATFORM_H)

#include <stdint.h>
#include <cstddef>

#if !defined(COMPILER_GCC)
#define COMPILER_GCC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_LLVM && !COMPILER_GCC
#if __GNUC__
#undef COMPILER_GCC
#define COMPILER_GCC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32_t bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

#define internal static
#define local static
#define global static

#define PI32 3.14159265359f

#if HEX_MAGIC_SLOW
#define Assert(expression)                                                                         \
    if (!(expression))                                                                             \
    {                                                                                              \
        *(volatile int *)0 = 0;                                                                    \
    }
#else
#define Assert(expression)
#endif

#define Kilobytes(value) ((value) * 1024LL)
#define Megabytes(value) (Kilobytes(value) * 1024LL)
#define Gigabytes(value) (Megabytes(value) * 1024LL)
#define Terabytes(value) (Gigabytes(value) * 1024LL)

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

inline uint32 SafeTruncateUInt64(uint64 value)
{
    Assert(value <= 0xFFFFFFFF);
    uint32 result = (uint32)value;

    return result;
}

typedef size_t MemoryIndex;

struct ThreadContext
{
    int placeholder;
};

#if HEX_MAGIC_INTERNAL
struct DebugReadFileResult
{
    uint32 contentsSize;
    void *contents;
};

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(ThreadContext *thread, void *memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory);

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name)                                                      \
    DebugReadFileResult name(ThreadContext *thread, char *fileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name)                                                     \
    bool32 name(ThreadContext *thread, char *fileName, uint32 memorySize, void *memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile);

#endif

struct GameOffscreenBuffer
{
    void *memory;
    int32 bytesPerPixel;
    int32 width;
    int32 height;
    int32 pitch;
};

struct GameSoundOutputBuffer
{
    int32 samplesPerSecond;
    int32 sampleCount;
    int16 *samples;
};

struct GameButtonState
{
    int halfTransitionCount;
    bool32 endedDown;
};

struct GameKeyboardInput
{
    union
    {
        GameButtonState buttons[10];
        struct
        {
            GameButtonState moveUp;
            GameButtonState moveDown;
            GameButtonState moveLeft;
            GameButtonState moveRight;

            GameButtonState cancel;

            GameButtonState toggleMode;
            GameButtonState nextBiome;
            GameButtonState addHero;

            GameButtonState save;
            GameButtonState load;
        };
    };
};

struct GameMouseInput
{
    int32 mouseX, mouseY;

    union
    {
        GameButtonState buttons[5];
        struct
        {
            GameButtonState lButton;
            GameButtonState mButton;
            GameButtonState rButton;
            GameButtonState x1Button;
            GameButtonState x2Button;
        };
    };
};

struct GameInput
{
    real32 dtForFrame;

    GameKeyboardInput keyboard;
    GameMouseInput mouse;
};

struct GameMemory
{
    bool32 isInitialized;

    uint64 permanentStorageSize;
    void *permanentStorage;

    uint64 transientStorageSize;
    void *transientStorage;

    DEBUGPlatformFreeFileMemory *debugPlatformFreeFileMemory;
    DEBUGPlatformReadEntireFile *debugPlatformReadEntireFile;
    DEBUGPlatformWriteEntireFile *debugPlatformWriteEntireFile;
};

#define GAME_UPDATE_AND_RENDER(name)                                                               \
    void name(ThreadContext *thread, GameMemory *memory, GameInput *input,                         \
              GameOffscreenBuffer *buffer)

typedef GAME_UPDATE_AND_RENDER(GameUpdateAndRender);

#define GAME_GET_SOUND_SAMPLES(name)                                                               \
    void name(ThreadContext *thread, GameMemory *memory, GameSoundOutputBuffer *soundBuffer)

typedef GAME_GET_SOUND_SAMPLES(GameGetSoundSamples);

#define HEX_MAGIC_PLATFORM_H
#endif
