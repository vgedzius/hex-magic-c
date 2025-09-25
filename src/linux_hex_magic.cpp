#include <SDL2/SDL.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <linux/limits.h>

#include "hex_magic_platform.h"
#include "linux_hex_magic.h"

global bool32 globalIsRunning;
global bool32 globalPause;
global LinuxOffscreenBuffer globalBackBuffer;
global LinuxAudioRingBuffer audioBuffer;
global uint64 globalPerfCountFrequency;

internal void CatStrings(size_t sourceACount, const char *sourceA, size_t sourceBCount,
                         const char *sourceB, size_t destCount, char *dest)
{
    for (uint32 index = 0; index < sourceACount; ++index)
    {
        *dest++ = *sourceA++;
    }

    for (uint32 index = 0; index < sourceBCount; ++index)
    {
        *dest++ = *sourceB++;
    }

    *dest++ = 0;
}

internal void LinuxGetExecutableFileName(LinuxState *state, char *args[])
{
    state->executableFileName           = args[0];
    state->onePastLastExecFileNameSlash = state->executableFileName;

    for (char *scan = state->executableFileName; *scan; ++scan)
    {
        if (*scan == '/')
        {
            state->onePastLastExecFileNameSlash = scan + 1;
        }
    }
}

internal int StringLength(const char *string)
{
    int count = 0;
    while (*string++)
    {
        count++;
    }

    return count;
}

internal void LinuxBuildExecDirFileName(LinuxState *state, const char *fileName, int destCount,
                                        char *dest)
{
    CatStrings(state->onePastLastExecFileNameSlash - state->executableFileName,
               state->executableFileName, StringLength(fileName), fileName, destCount, dest);
}

internal void LinuxResizeBackBuffer(LinuxOffscreenBuffer *buffer, SDL_Renderer *renderer, int width,
                                    int height)
{
    if (buffer->renderTexture)
    {
        SDL_DestroyTexture(buffer->renderTexture);
    }

    if (buffer->memory)
    {
        munmap(buffer->memory, buffer->width * buffer->height * buffer->bytesPerPixel);
    }

    buffer->width         = width;
    buffer->height        = height;
    buffer->bytesPerPixel = 4;
    buffer->pitch         = width * buffer->bytesPerPixel;

    buffer->renderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                              SDL_TEXTUREACCESS_STREAMING, width, height);

    if (!buffer->renderTexture)
    {
        printf("Failed to create texture: %s\n", SDL_GetError());
        // TODO terminate or smth?
    }

    buffer->memory = mmap(0, buffer->width * buffer->height * buffer->bytesPerPixel,
                          PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (buffer->memory == MAP_FAILED)
    {
        printf("Failed to allocate backbuffer memory\n");
        // TODO terminate or smth?
    }
}

internal void LinuxFillSoundBuffer(LinuxSoundOutput *soundOutput,
                                   GameSoundOutputBuffer *sourceBuffer, uint32 byteToLock,
                                   uint32 bytesToWrite)
{
    SDL_LockAudio();

    void *region1      = (uint8 *)audioBuffer.data + byteToLock;
    uint32 region1Size = bytesToWrite;

    if (region1Size + byteToLock > audioBuffer.size)
    {
        region1Size = audioBuffer.size - byteToLock;
    }

    void *region2      = audioBuffer.data;
    uint32 region2Size = bytesToWrite - region1Size;

    uint32 region1SampleCount = region1Size / soundOutput->bytesPerSample;
    int16 *destSample         = (int16 *)region1;
    int16 *sourceSample       = sourceBuffer->samples;
    for (uint32 sampleIndex = 0; sampleIndex < region1SampleCount; ++sampleIndex)
    {
        *destSample++ = *sourceSample++;
        *destSample++ = *sourceSample++;
        ++soundOutput->runningSampleIndex;
    }

    uint32 region2SampleCount = region2Size / soundOutput->bytesPerSample;
    destSample                = (int16 *)region2;
    for (uint32 sampleIndex = 0; sampleIndex < region2SampleCount; ++sampleIndex)
    {
        *destSample++ = *sourceSample++;
        *destSample++ = *sourceSample++;
        ++soundOutput->runningSampleIndex;
    }

    SDL_UnlockAudio();
}

internal void LinuxDisplayBufferInWindow(LinuxState *state, LinuxOffscreenBuffer *buffer,
                                         SDL_Renderer *renderer)
{
    void *pixels;
    int pitch;
    SDL_Rect sourceRect = {};
    SDL_Rect destRect   = {};

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_LockTexture(buffer->renderTexture, NULL, &pixels, &pitch);
    memcpy(pixels, buffer->memory, buffer->width * buffer->height * buffer->bytesPerPixel);
    SDL_UnlockTexture(buffer->renderTexture);

    sourceRect.w = buffer->width;
    sourceRect.h = buffer->height;

    if (state->isFullscreen)
    {
        destRect.w = state->windowWidth;
        destRect.h = state->windowHeight;
    }
    else
    {
        destRect = sourceRect;
    }

    SDL_RenderCopy(renderer, buffer->renderTexture, &sourceRect, &destRect);
    SDL_RenderPresent(renderer);
}

internal void LinuxAudioCallback(void *userData, unsigned char *audioData, int32 length)
{
    LinuxAudioRingBuffer *ringBuffer = (LinuxAudioRingBuffer *)userData;

    int region1Size = length;
    int region2Size = 0;

    if (ringBuffer->playCursor + length > ringBuffer->size)
    {
        region1Size = ringBuffer->size - ringBuffer->playCursor;
        region2Size = length - region1Size;
    }

    memcpy(audioData, (uint8 *)(ringBuffer->data) + ringBuffer->playCursor, region1Size);
    memcpy(&audioData[region1Size], ringBuffer->data, region2Size);

    ringBuffer->playCursor  = (ringBuffer->playCursor + length) % ringBuffer->size;
    ringBuffer->writeCursor = (ringBuffer->playCursor + 2048) % ringBuffer->size;
}

internal void LinuxInitSound(uint32 samplesPerSecond, uint32 bufferSize)
{
    SDL_AudioSpec audioSpec = {0};

    audioSpec.freq     = samplesPerSecond;
    audioSpec.format   = AUDIO_S16LSB;
    audioSpec.channels = 2;
    audioSpec.samples  = 1024;
    audioSpec.callback = &LinuxAudioCallback;
    audioSpec.userdata = &audioBuffer;

    audioBuffer.size = bufferSize;
    audioBuffer.data =
        mmap(0, bufferSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // TODO technicaly we can still run the game without the audio. Should
    // handle this properly
    if (!audioBuffer.data)
    {
        printf("Failed to allocate memory for the audio");
    }

    SDL_OpenAudio(&audioSpec, 0);

    if (audioSpec.format != AUDIO_S16LSB)
    {
        printf("Could not aquire desired audio format. Actual format: %i\n", audioSpec.format);
    }
}

internal void LinuxProcessKeyboardMessage(GameButtonState *newState, bool32 isPressed)
{
    if (newState->endedDown != isPressed)
    {
        newState->endedDown = isPressed;
        ++newState->halfTransitionCount;
    }
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(debugPlatformFreeFileMemory)
{
    if (memory)
    {
        free(memory);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(debugPlatformReadEntireFile)
{
    DebugReadFileResult result = {};
    int fileHandle             = open(fileName, O_RDONLY);
    if (fileHandle == -1)
    {
        return result;
    }

    struct stat fileStatus;
    if (fstat(fileHandle, &fileStatus) == -1)
    {
        close(fileHandle);
        return result;
    }

    result.contentsSize = fileStatus.st_size;
    result.contents     = malloc(result.contentsSize);
    if (!result.contents)
    {
        result.contentsSize = 0;
        close(fileHandle);
        return result;
    }

    uint32 bytesToRead      = result.contentsSize;
    uint8 *nextByteLocation = (uint8 *)result.contents;
    while (bytesToRead)
    {
        ssize_t bytesRead = read(fileHandle, nextByteLocation, bytesToRead);
        if (bytesRead == -1)
        {
            debugPlatformFreeFileMemory(thread, result.contents);
            result.contents     = 0;
            result.contentsSize = 0;
            close(fileHandle);

            return result;
        }

        bytesToRead -= bytesRead;
        nextByteLocation += bytesRead;
    }

    close(fileHandle);
    return result;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debugPlatformWriteEntireFile)
{
    int fileHandle = open(fileName, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fileHandle == -1)
        return false;

    uint32 bytesToWrite     = memorySize;
    uint8 *nextByteLocation = (uint8 *)memory;
    while (bytesToWrite)
    {
        ssize_t bytesWritten = write(fileHandle, nextByteLocation, bytesToWrite);
        if (bytesWritten == -1)
        {
            close(fileHandle);
            return false;
        }
        bytesToWrite -= bytesWritten;
        nextByteLocation += bytesWritten;
    }

    close(fileHandle);

    return true;
}

internal int LinuxGetWindowRefreshRate(SDL_Window *window)
{
    SDL_DisplayMode mode;
    int displayIndex = SDL_GetWindowDisplayIndex(window);

    int defaultRefreshRate = 60;
    if (SDL_GetDesktopDisplayMode(displayIndex, &mode) != 0)
    {
        return defaultRefreshRate;
    }

    if (mode.refresh_rate == 0)
    {
        return defaultRefreshRate;
    }

    return mode.refresh_rate;
}

inline real32 LinuxGetSecondsElapsed(uint64 oldCounter, uint64 currentCounter)
{
    real32 result = (real32)(currentCounter - oldCounter) / globalPerfCountFrequency;
    return result;
}

internal void LinuxDebugDrawVertical(LinuxOffscreenBuffer *backBuffer, int x, int top, int bottom,
                                     uint32 color)
{
    if (top <= 0)
    {
        top = 0;
    }

    if (bottom > backBuffer->height)
    {
        bottom = backBuffer->height;
    }

    if ((x >= 0) && (x < backBuffer->width))
    {
        uint8 *pixel =
            (uint8 *)backBuffer->memory + x * backBuffer->bytesPerPixel + top * backBuffer->pitch;
        for (int y = top; y < bottom; ++y)
        {
            *(uint32 *)pixel = color;
            pixel += backBuffer->pitch;
        }
    }
}

inline void LinuxDebugDrawSoundBufferMarker(LinuxOffscreenBuffer *backBuffer,
                                            LinuxSoundOutput *soundOutput, real32 c, int padX,
                                            int top, int bottom, uint32 value, uint32 color)
{
    real32 xReal32 = c * (real32)value;
    int x          = padX + (int)xReal32;
    LinuxDebugDrawVertical(backBuffer, x, top, bottom, color);
}

#if 0
internal void LinuxDebugSyncDisplay(LinuxOffscreenBuffer *backBuffer, uint32 markerCount,
                                    LinuxDebugTimeMarker *markers, uint32 currentMarkerIndex,
                                    LinuxSoundOutput *soundOutput, real32 targetSecondsPerFrame)
{
    int padX = 16;
    int padY = 16;

    int lineHight = 64;

    real32 c = (real32)(backBuffer->width - 2 * padX) / (real32)soundOutput->secondaryBufferSize;
    for (uint32 markerIndex = 0; markerIndex < markerCount; ++markerIndex)
    {
        LinuxDebugTimeMarker *thisMarker = &markers[markerIndex];
        Assert(thisMarker->outputPlayCursor < soundOutput->secondaryBufferSize);
        Assert(thisMarker->outputWriteCursor < soundOutput->secondaryBufferSize);
        Assert(thisMarker->outputLocation < soundOutput->secondaryBufferSize);
        Assert(thisMarker->outputByteCount < soundOutput->secondaryBufferSize);
        Assert(thisMarker->flipPlayCursor < soundOutput->secondaryBufferSize);
        Assert(thisMarker->flipWriteCursor < soundOutput->secondaryBufferSize);

        uint32 playColor         = 0xFFFFFFFF;
        uint32 writeColor        = 0xFFFF0000;
        uint32 expectedFlipColor = 0xFFFFFF00;
        uint32 playWindowColor   = 0xFFFF00FF;

        int top    = padY;
        int bottom = padY + lineHight;
        if (markerIndex == currentMarkerIndex)
        {
            top += lineHight + padY;
            bottom += lineHight + padY;

            int firstTop = top;

            LinuxDebugDrawSoundBufferMarker(backBuffer, soundOutput, c, padX, top, bottom,
                                            thisMarker->outputPlayCursor, playColor);

            LinuxDebugDrawSoundBufferMarker(backBuffer, soundOutput, c, padX, top, bottom,
                                            thisMarker->outputWriteCursor, writeColor);

            top += lineHight + padY;
            bottom += lineHight + padY;

            LinuxDebugDrawSoundBufferMarker(backBuffer, soundOutput, c, padX, top, bottom,
                                            thisMarker->outputLocation, playColor);

            LinuxDebugDrawSoundBufferMarker(
                backBuffer, soundOutput, c, padX, top, bottom,
                thisMarker->outputLocation + thisMarker->outputByteCount, writeColor);

            top += lineHight + padY;
            bottom += lineHight + padY;

            LinuxDebugDrawSoundBufferMarker(backBuffer, soundOutput, c, padX, firstTop, bottom,
                                            thisMarker->expectedFlipCursor, expectedFlipColor);
        }

        LinuxDebugDrawSoundBufferMarker(backBuffer, soundOutput, c, padX, top, bottom,
                                        thisMarker->flipPlayCursor, playColor);

        LinuxDebugDrawSoundBufferMarker(
            backBuffer, soundOutput, c, padX, top, bottom,
            thisMarker->flipPlayCursor + 480 * soundOutput->bytesPerSample, playWindowColor);

        LinuxDebugDrawSoundBufferMarker(backBuffer, soundOutput, c, padX, top, bottom,
                                        thisMarker->flipWriteCursor, writeColor);
    }
}
#endif

inline struct timespec LinuxGetLastWriteTime(char *fileName)
{
    timespec lastWriteTime = {};

    struct stat fileStat;
    if (stat(fileName, &fileStat) == 0)
    {
        lastWriteTime = fileStat.st_mtim;
    }

    return lastWriteTime;
}

internal LinuxGameCode LinuxLoadGameCode(char *gameSoFileName)
{
    LinuxGameCode result = {};

    result.gameLib       = dlopen(gameSoFileName, RTLD_NOW);
    result.lastWriteTime = LinuxGetLastWriteTime(gameSoFileName);

    if (result.gameLib)
    {
        result.updateAndRender =
            (GameUpdateAndRender *)dlsym(result.gameLib, "gameUpdateAndRender");

        if (!result.updateAndRender)
        {
            printf("Failed to load GameUpdateAndRender: %s\n", dlerror());
        }

        result.getSoundSamples =
            (GameGetSoundSamples *)dlsym(result.gameLib, "gameGetSoundSamples");

        if (!result.getSoundSamples)
        {
            printf("Failed to load GameGetSoundSamples: %s\n", dlerror());
        }

        result.isValid = result.updateAndRender && result.getSoundSamples;
    }
    else
    {
        printf("Failed to load game library: %s\n", dlerror());
    }

    if (!result.isValid)
    {
        result.updateAndRender = 0;
        result.getSoundSamples = 0;
    }

    return result;
}

internal void LinuxUnloadGameCode(LinuxGameCode *gameCode)
{
    if (gameCode->gameLib)
    {
        dlclose(gameCode->gameLib);

        gameCode->gameLib         = 0;
        gameCode->isValid         = false;
        gameCode->updateAndRender = 0;
        gameCode->getSoundSamples = 0;
    }
}

void ToggleFullscreen(LinuxState *state, SDL_Window *window)
{
    uint32 fullscreenFlag        = SDL_WINDOW_FULLSCREEN_DESKTOP;
    bool32 isCurrentlyFullscreen = SDL_GetWindowFlags(window) & fullscreenFlag;
    bool32 newState              = state->isFullscreen ? 0 : fullscreenFlag;

    SDL_SetWindowFullscreen(window, newState);
    state->isFullscreen = newState;
}

internal void LinuxProcessEvents(SDL_Window *window, SDL_Renderer *renderer, LinuxState *state,
                                 GameKeyboardInput *keyboardController)
{
    SDL_Event e;
    while (SDL_PollEvent(&e) > 0)
    {
        switch (e.type)
        {
            case SDL_WINDOWEVENT:
            {
                switch (e.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    {
                        SDL_GL_GetDrawableSize(window, &state->windowWidth, &state->windowHeight);
                        printf("Resize to: %dx%d\n", state->windowWidth, state->windowHeight);
                    }
                    break;

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    {
                        printf("Enter window\n");
                    }
                    break;

                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    {
                        printf("Leave window\n");
                    }
                    break;
                }
            }
            break;

            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                SDL_Keycode vkCode = e.key.keysym.sym;
                bool32 isDown      = e.key.state == SDL_PRESSED;

                if (!e.key.repeat)
                {
                    if (vkCode == 'w')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->moveUp, isDown);
                    }
                    else if (vkCode == 'a')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->moveLeft, isDown);
                    }
                    else if (vkCode == 's')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->moveDown, isDown);
                    }
                    else if (vkCode == 'd')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->moveRight, isDown);
                    }
                    else if (vkCode == SDLK_UP)
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->moveUp, isDown);
                    }
                    else if (vkCode == SDLK_LEFT)
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->moveLeft, isDown);
                    }
                    else if (vkCode == SDLK_DOWN)
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->moveDown, isDown);
                    }
                    else if (vkCode == SDLK_RIGHT)
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->moveRight, isDown);
                    }
                    else if (vkCode == SDLK_RETURN && isDown && (e.key.keysym.mod & KMOD_ALT))
                    {
                        ToggleFullscreen(state, window);
                    }
                    else if (vkCode == SDLK_ESCAPE)
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->cancel, isDown);
                    }
#if HEX_MAGIC_INTERNAL
                    else if (vkCode == 'l')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->load, isDown);
                    }
                    else if (vkCode == 'm')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->save, isDown);
                    }
                    else if (vkCode == 'e')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->toggleMode, isDown);
                    }
                    else if (vkCode == 'b')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->nextBiome, isDown);
                    }
                    else if (vkCode == 'h')
                    {
                        LinuxProcessKeyboardMessage(&keyboardController->addHero, isDown);
                    }
                    else if (vkCode == 'p')
                    {
                        if (isDown)
                        {
                            globalPause = !globalPause;
                        }
                    }
#endif
                }
            }
            break;

            case SDL_QUIT:
            {
                globalIsRunning = false;
            }
            break;
        }

        // TODO https://www.youtube.com/watch?v=J3y1x54vyIQ here Casey shows
        // how to get game pad input. I'll be skipping this for now as I do
        // not have controller handy to test this whole thing, and can't
        // really be bothered to simulate these inputs using somehting like
        // uiinput or so. Perhaps I'll revisit this at some point later.
        //
        // Another installement of controller inputs is discussed here
        // int blueOffset, int greenOffset, int toneHz
    }
}

int main(int argc, char *args[])
{
    LinuxState linuxState = {};

    globalPerfCountFrequency = SDL_GetPerformanceFrequency();
    int initWidth            = 1280;
    int initHeight           = 720;

    LinuxGetExecutableFileName(&linuxState, args);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        printf("Failed to initialise graphics: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window =
        SDL_CreateWindow("Hex magic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, initWidth,
                         initHeight, SDL_WINDOW_RESIZABLE);

    if (!window)
    {
        printf("Failed to create window: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer =
        SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
        printf("Failed to create renderer: %s", SDL_GetError());
        return 1;
    }

    linuxState.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
#if HEX_MAGIC_INTERNAL
    linuxState.showCursor = true;
#endif
    if (!linuxState.cursor)
    {
        linuxState.showCursor = false;
        printf("Failed to create cursor: %s", SDL_GetError());
    }

    int monitorRefreshHz         = LinuxGetWindowRefreshRate(window);
    real32 gameUpdateHz          = monitorRefreshHz / 2.0f;
    real32 targetSecondsPerFrame = 1.0f / (real32)gameUpdateHz;

    LinuxResizeBackBuffer(&globalBackBuffer, renderer, initWidth, initHeight);

    LinuxSoundOutput soundOutput = {};

    soundOutput.samplesPerSecond    = 48000;
    soundOutput.runningSampleIndex  = 0;
    soundOutput.bytesPerSample      = sizeof(int16) * 2;
    soundOutput.secondaryBufferSize = soundOutput.samplesPerSecond * soundOutput.bytesPerSample;
    soundOutput.latencySampleCount  = 2 * (soundOutput.samplesPerSecond / gameUpdateHz);
    soundOutput.safetyBytes         = (int)(((real32)soundOutput.samplesPerSecond *
                                     (real32)soundOutput.bytesPerSample / gameUpdateHz) /
                                    3.0f);

    LinuxInitSound(soundOutput.samplesPerSecond, soundOutput.secondaryBufferSize);

    SDL_PauseAudio(0);

    int16 *samples = (int16 *)mmap(0, soundOutput.secondaryBufferSize, PROT_READ | PROT_WRITE,
                                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    globalIsRunning = true;

#if HEX_MAGIC_INTERNAL
    void *baseAddress = (void *)Terabytes(2);
#else
    void *baseAddress = (void *)0;
#endif

    GameMemory gameMemory                   = {};
    gameMemory.permanentStorageSize         = Megabytes(64);
    gameMemory.transientStorageSize         = Gigabytes(1);
    gameMemory.debugPlatformFreeFileMemory  = debugPlatformFreeFileMemory;
    gameMemory.debugPlatformReadEntireFile  = debugPlatformReadEntireFile;
    gameMemory.debugPlatformWriteEntireFile = debugPlatformWriteEntireFile;

    linuxState.totalSize       = gameMemory.permanentStorageSize + gameMemory.transientStorageSize;
    linuxState.gameMemoryBlock = mmap(baseAddress, (size_t)linuxState.totalSize,
                                      PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    gameMemory.permanentStorage = linuxState.gameMemoryBlock;
    gameMemory.transientStorage =
        (uint8 *)gameMemory.permanentStorage + gameMemory.permanentStorageSize;

    if (!samples || !gameMemory.permanentStorage || !gameMemory.transientStorage)
    {
        printf("Could not initialize game memory\n");
        return 1;
    }

    GameInput input[2]  = {};
    GameInput *newInput = &input[0];
    GameInput *oldInput = &input[1];

    uint64 lastCounter     = SDL_GetPerformanceCounter();
    uint64 flpWallClock    = SDL_GetPerformanceCounter();
    uint32 fps             = 0;
    real64 currentSecond   = 0.0f;
    real32 secondsPerFrame = 0.0f;

    uint32 debugTimeMarkerIndex               = 0;
    LinuxDebugTimeMarker debugTimeMarkers[30] = {};

    uint32 audioLatencyBytes   = 0;
    real32 audioLatencySeconds = 0;
    bool32 soundIsValid        = false;

    char gameSoName[PATH_MAX];
    LinuxBuildExecDirFileName(&linuxState, "hex_magic.so", sizeof(gameSoName), gameSoName);

    LinuxGameCode game = LinuxLoadGameCode(gameSoName);

    uint64 lastCycleCount = __rdtsc();
    while (globalIsRunning)
    {
        if (linuxState.showCursor)
        {
            SDL_SetCursor(linuxState.cursor);
            SDL_ShowCursor(SDL_ENABLE);
        }
        else
        {
            SDL_ShowCursor(SDL_DISABLE);
        }

        newInput->dtForFrame = secondsPerFrame;

        struct timespec newGameCodeWriteTime = LinuxGetLastWriteTime(gameSoName);
        if (newGameCodeWriteTime.tv_sec != game.lastWriteTime.tv_sec)
        {
            LinuxUnloadGameCode(&game);
            game = LinuxLoadGameCode(gameSoName);
            printf("Hot reload\n");
        }

        GameKeyboardInput *oldKeyboardInput = &oldInput->keyboard;
        GameKeyboardInput *newKeyboardInput = &newInput->keyboard;
        *newKeyboardInput                   = {};

        for (uint32 buttonIndex = 0; buttonIndex < ArrayCount(newKeyboardInput->buttons);
             ++buttonIndex)
        {
            newKeyboardInput->buttons[buttonIndex].endedDown =
                oldKeyboardInput->buttons[buttonIndex].endedDown;
        }

        LinuxProcessEvents(window, renderer, &linuxState, newKeyboardInput);

        if (!globalPause)
        {
            newInput->mouseZ = 0;

            uint32 buttons = SDL_GetMouseState(&newInput->mouseX, &newInput->mouseY);
            LinuxProcessKeyboardMessage(&newInput->mouseButtons[0], buttons & SDL_BUTTON_LMASK);
            LinuxProcessKeyboardMessage(&newInput->mouseButtons[1], buttons & SDL_BUTTON_MMASK);
            LinuxProcessKeyboardMessage(&newInput->mouseButtons[2], buttons & SDL_BUTTON_RMASK);
            LinuxProcessKeyboardMessage(&newInput->mouseButtons[3], buttons & SDL_BUTTON_X1MASK);
            LinuxProcessKeyboardMessage(&newInput->mouseButtons[4], buttons & SDL_BUTTON_X2MASK);

            ThreadContext thread = {};

            GameOffscreenBuffer buffer = {};
            buffer.memory              = globalBackBuffer.memory;
            buffer.width               = globalBackBuffer.width;
            buffer.height              = globalBackBuffer.height;
            buffer.pitch               = globalBackBuffer.pitch;
            buffer.bytesPerPixel       = globalBackBuffer.bytesPerPixel;

            if (game.updateAndRender)
            {
                game.updateAndRender(&thread, &gameMemory, newInput, &buffer);
            }

            uint64 audioWallClock          = SDL_GetPerformanceCounter();
            real32 fromBeginToAudioSeconds = LinuxGetSecondsElapsed(flpWallClock, audioWallClock);

            uint32 playCursor  = audioBuffer.playCursor;
            uint32 writeCursor = audioBuffer.writeCursor;

            if (!soundIsValid)
            {
                soundOutput.runningSampleIndex = writeCursor / soundOutput.bytesPerSample;
                soundIsValid                   = true;
            }

            uint32 byteToLock = (soundOutput.runningSampleIndex * soundOutput.bytesPerSample) %
                                soundOutput.secondaryBufferSize;

            uint32 expectedSoundBytesPerFrame =
                (int)((real32)(soundOutput.samplesPerSecond * soundOutput.bytesPerSample) /
                      gameUpdateHz);

            real32 secondsLeftUntilFlip = targetSecondsPerFrame - fromBeginToAudioSeconds;

            uint32 expectebBytesUntilFlip =
                (uint32)((secondsLeftUntilFlip / targetSecondsPerFrame) *
                         (real32)expectedSoundBytesPerFrame);

            uint32 expectedFrameBoundaryByte = playCursor + expectebBytesUntilFlip;
            uint32 safeWriteCuror            = writeCursor;
            if (safeWriteCuror < playCursor)
            {
                safeWriteCuror += soundOutput.secondaryBufferSize;
            }
            Assert(safeWriteCuror >= playCursor);
            safeWriteCuror += soundOutput.safetyBytes;

            bool32 audioCardIsLowLatency = safeWriteCuror < expectedFrameBoundaryByte;

            uint32 targetCursor = 0;
            if (audioCardIsLowLatency)
            {
                targetCursor = expectedFrameBoundaryByte + expectedSoundBytesPerFrame;
            }
            else
            {
                targetCursor =
                    audioBuffer.writeCursor + expectedSoundBytesPerFrame + soundOutput.safetyBytes;
            }
            targetCursor = targetCursor % soundOutput.secondaryBufferSize;

            uint32 bytesToWrite = 0;
            if (byteToLock > targetCursor)
            {
                bytesToWrite = soundOutput.secondaryBufferSize - byteToLock;
                bytesToWrite += targetCursor;
            }
            else
            {
                bytesToWrite = targetCursor - byteToLock;
            }

            GameSoundOutputBuffer soundBuffer = {};
            soundBuffer.samplesPerSecond      = soundOutput.samplesPerSecond;
            soundBuffer.sampleCount           = bytesToWrite / soundOutput.bytesPerSample;
            soundBuffer.samples               = samples;

            if (game.getSoundSamples)
            {
                game.getSoundSamples(&thread, &gameMemory, &soundBuffer);
            }

#if HEX_MAGIC_INTERNAL
            LinuxDebugTimeMarker *marker = &debugTimeMarkers[debugTimeMarkerIndex];
            marker->outputPlayCursor     = playCursor;
            marker->outputWriteCursor    = writeCursor;
            marker->outputLocation       = byteToLock;
            marker->outputByteCount      = bytesToWrite;
            marker->expectedFlipCursor   = expectedFrameBoundaryByte;

            // uint32 unwrappedWriteCursor = audioBuffer.writeCursor;
            // if (unwrappedWriteCursor < audioBuffer.playCursor)
            // {
            //     unwrappedWriteCursor += soundOutput.secondaryBufferSize;
            // }
            // audioLatencyBytes   = unwrappedWriteCursor - audioBuffer.playCursor;
            // audioLatencySeconds = (real32)audioLatencyBytes /
            // (real32)soundOutput.bytesPerSample
            // /
            //                       (real32)soundOutput.samplesPerSecond;
            //
            // printf("BTL:%u TC:%u BTW: %u - PC:%u WC:%u DELTA:%u (%fs)\n", byteToLock,
            // targetCursor,
            //        bytesToWrite, playCursor, writeCursor, audioLatencyBytes,
            //        audioLatencySeconds);
#endif
            LinuxFillSoundBuffer(&soundOutput, &soundBuffer, byteToLock, bytesToWrite);

            uint64 endCounter = SDL_GetPerformanceCounter();
            secondsPerFrame   = LinuxGetSecondsElapsed(lastCounter, endCounter);
            real64 msPerFrame = secondsPerFrame * 1000.0f;
            lastCounter       = endCounter;

            currentSecond += secondsPerFrame;
            fps++;

#if HEX_MAGIC_INTERNAL
            uint64 endCycleCount = __rdtsc();
            uint64 cyclesElapsed = endCycleCount - lastCycleCount;
            lastCycleCount       = endCycleCount;

            real64 mcPerFrame = (real64)cyclesElapsed / (1000 * 1000);

            if (currentSecond > 1.0f)
            {
                printf("%.02fms/f, %df/s, %.02fMc/f\n", msPerFrame, fps, mcPerFrame);

                currentSecond = 0.0f;
                fps           = 0;
            }

            {
                // LinuxDebugSyncDisplay(&globalBackBuffer, ArrayCount(debugTimeMarkers),
                //                       debugTimeMarkers, debugTimeMarkerIndex - 1,
                //                       &soundOutput, targetSecondsPerFrame);
            }
#endif

            LinuxDisplayBufferInWindow(&linuxState, &globalBackBuffer, renderer);

            flpWallClock = SDL_GetPerformanceCounter();

#if HEX_MAGIC_INTERNAL
            {
                Assert(debugTimeMarkerIndex < ArrayCount(debugTimeMarkers));
                LinuxDebugTimeMarker *marker = &debugTimeMarkers[debugTimeMarkerIndex];

                marker->flipPlayCursor  = audioBuffer.playCursor;
                marker->flipWriteCursor = audioBuffer.writeCursor;
            }
#endif

            GameInput *temp = newInput;
            newInput        = oldInput;
            oldInput        = temp;

#if HEX_MAGIC_INTERNAL
            ++debugTimeMarkerIndex;
            if (debugTimeMarkerIndex == ArrayCount(debugTimeMarkers))
            {
                debugTimeMarkerIndex = 0;
            }
#endif
        }
    }

    SDL_CloseAudio();

    return 0;
}
