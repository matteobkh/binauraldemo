/* Binaural Spatializer Demo
Matteo Bukhgalter 2025 */

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#include "gui.h"
#include "audio.h"
#include "filemanager.h"

#include <iostream>
#include <filesystem>
#include <thread>

namespace fs = std::filesystem;

#include <SDL2/SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL2/SDL_opengl.h>
#endif

#define SAMPLE_RATE (44100)

static int paErrorHandling(PaError err);

// Main code
int main(int, char**)
{
    // Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Binaural Spatializer Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 350, 470, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr)
    {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    //Enable drop file
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // -- Custom style - Pastel Blue --
    ImGuiStyle& style = ImGui::GetStyle();
    style.AntiAliasedFill = true;
    style.AntiAliasedLines = true;
    style.FrameRounding = 3.0f;
    ImVec4* colors = style.Colors;
    // Darker Background
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    // Buttons - Soft Pastel Blue
    colors[ImGuiCol_Button] = ImVec4(0.50f, 0.65f, 0.85f, 1.0f);  
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.70f, 0.90f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.60f, 0.80f, 1.0f);
    // Frame Borders & Elements - Lighter Blue
    colors[ImGuiCol_FrameBg] = ImVec4(0.40f, 0.55f, 0.75f, 0.40f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.45f, 0.60f, 0.80f, 0.55f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.50f, 0.65f, 0.85f, 0.70f);
    // Text - White
    colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    // ComboBox
    colors[ImGuiCol_Header] = ImVec4(0.50f, 0.65f, 0.85f, 1.0f);  
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.55f, 0.70f, 0.90f, 1.0f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.45f, 0.60f, 0.80f, 1.0f);
    // Checkbox
    colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.65f, 0.85f, 1.0f);
    // Sliders
    colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.65f, 0.85f, 1.0f);  
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.45f, 0.60f, 0.80f, 1.0f);
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.50f, 0.65f, 0.85f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.55f, 0.70f, 0.90f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.60f, 0.80f, 1.0f);
    
    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    io.Fonts->AddFontFromFileTTF("./libs/imgui/misc/fonts/Roboto-Medium.ttf", 15.0f);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Init portaudio
    ScopedPaHandler paInit;
    if(paInit.result() != paNoError) return paErrorHandling(paInit.result());

    AudioFileData emptyBuffer(std::vector<float>(44100, 0.0f)); // 1 second of silence at 44.1 kHz, 1 channel

    AudioEngine audioEngine(SAMPLE_RATE, emptyBuffer);
    openAudio(Pa_GetDefaultOutputDevice(), audioEngine);
    std::cout << std::endl; // because Pa_GetDefaultOutputDevice() logs without endl
    startAudio();

    // Main loop
    bool done = false;
    while (!done)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_DROPFILE) { // handle loading audio file from drag and drop
                char* droppedFilePath = event.drop.file;
                std::string pathStr(droppedFilePath);
                SDL_free(droppedFilePath); // free early — it's copied into pathStr

                std::string ext = fs::path(pathStr).extension().string();
                if (ext == ".wav" || ext == ".mp3" || ext == ".flac") {
                    if (audioEngine.play) {
                        audioEngine.play = false;
                    }

                    FileManager::loading = true;
                    FileManager::fileLoaded = false;
                    FileManager::currentFileName = fs::path(pathStr).filename().string();

                    // Launch background thread to load audio
                    std::thread([&, pathStr]() {
                        audioEngine.audioData = FileManager::LoadAudioFile(pathStr);

                        std::cout << "Audio file loaded!" << std::endl
                                << "\tFile name: " << FileManager::currentFileName << std::endl
                                << "\tSample rate: " << audioEngine.audioData.sampleRate << std::endl
                                << "\tChannels: " << audioEngine.audioData.nChannels << std::endl
                                << "\tSize (in samples): " << audioEngine.audioData.size << std::endl;

                        FileManager::fileLoaded = true;
                        FileManager::loading = false;
                    }).detach(); // fire and forget
                } else {
                    std::cerr << "Unsupported file type dropped: " << ext << "\n";
                }
            }
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        if (FileManager::fileLoaded) {
            GUI::mainInterface(audioEngine);
        } else {
            GUI::loadFile();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    stopAudio();
    closeAudio();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

static int paErrorHandling(PaError err) {
    fprintf(stderr, "An error occurred while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
    return 1;
}