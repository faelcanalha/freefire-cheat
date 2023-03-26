#define IMGUI_DEFINE_MATH_OPERATORS


#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include <atlsecurity.h> 
#include <thread>
#include <ctime>
#include <iomanip>
#include <ShObjIdl_core.h>
#include <stdio.h>
#include <windows.h>
#include <windns.h>

#pragma comment(lib, "Dnsapi.lib")

typedef BOOL(WINAPI* DnsFlushResolverCacheFuncPtr)();

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
HRESULT WINAPI hook_reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
void RenderBlur(HWND hwnd);
void threadfunction();

#include "variable.h"
#include "bytes.h"
#include "vector/AnimVector.h"
#include "imgui/custom.hpp"
#include "auth/auth.hpp"
#include "bypass.h"
#include "Memory.h"


ImFont* ico;
ImFont* widgets;

bool copypst;
bool text_animation_hide0;

HMODULE dnsapi;

Memory memory;
Just::api just("948332057245925416", "1.0.0");

std::string messger;

Foudasi bypass;
bool OpenRegistry();
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    dnsapi = LoadLibrary("dnsapi.dll");
    if (dnsapi != NULL) dns = true;
    DnsFlushResolverCacheFuncPtr DnsFlushResolverCache = (DnsFlushResolverCacheFuncPtr)GetProcAddress(dnsapi, "DnsFlushResolverCache");
    if (DnsFlushResolverCache == NULL) {
        FreeLibrary(dnsapi);
    }

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, "x", NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, "x", WS_POPUP, 0, 0, 2, 2, NULL, NULL, wc.hInstance, NULL);
   
    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    RECT rc = { 0 };
    GetWindowRect(hwnd, &rc);

    // Show the window
    ShowWindow(hwnd, SW_HIDE);
    UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


    ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\verdana.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    widgets = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\verdana.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());

    ImFont* my_icon2 = io.Fonts->AddFontFromMemoryTTF(&icon_custom, sizeof icon_custom, 32, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ImFont* Nev = io.Fonts->AddFontFromMemoryTTF(&Nevan, sizeof Nevan, 35, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ImFont* func = io.Fonts->AddFontFromMemoryTTF(&D0, sizeof D0, 23, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ico = io.Fonts->AddFontFromMemoryTTF(icon, sizeof(icon), 22.f, NULL, io.Fonts->GetGlyphRangesCyrillic());

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImVec4* colors = ImGui::GetStyle().Colors;
    ImGuiStyle& style = ImGui::GetStyle();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.FrameRounding = 3.0f;
        style.FramePadding = ImVec2(8, 8);
        style.WindowPadding = ImVec2(0, 0);

        colors[ImGuiCol_WindowBg] = ImColor(24, 24, 35); // set background color
        colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);   // set border window
        colors[ImGuiCol_FrameBg] = ImColor(32, 32, 48);
        colors[ImGuiCol_Text] = ImColor(223, 224, 225);
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);


    DWORD window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

    RECT screen_rect;
    GetWindowRect(GetDesktopWindow(), &screen_rect);
    auto x = float(screen_rect.right - WIDTH) / 2.f;
    auto y = float(screen_rect.bottom - HEIGHT) / 2.f;

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame(); {
            ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Once);

            ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT));

            ImGui::SetNextWindowBgAlpha(1.0f);

            HWND hw = FindWindow(NULL, LOADER_BRAND);

            if (GetAsyncKeyState(VK_HOME) & 1) { ShowWindow(hw, SW_MINIMIZE); }
            if (GetAsyncKeyState(VK_DELETE) & 1) { exit(0); }

            if (!authenticed) {
                ImGui::Begin(LOADER_BRAND, &loader_active, window_flags); {

                    if (particles) dot_draw();

                    HWND hw = FindWindow(NULL, LOADER_BRAND);
                    SetWindowRgn(hw, CreateRoundRectRgn(0, 0, WIDTH, HEIGHT, 15, 15), true);

                    const auto& CurrentWindowPos = ImGui::GetWindowPos();
                    const auto& pWindowDrawList = ImGui::GetWindowDrawList();
                    const auto& pForegroundDrawList = ImGui::GetForegroundDrawList();

                    if (!loader_animation)
                        loader_animation = GetTickCount();

                    if (GetTickCount() > loader_animation + (1 * 15))
                    {

                        if (text_animation_hide) {
                            if (text_animation < 0.80f)
                                text_animation += 0.03f / ImGui::GetIO().Framerate * 60.f;

                            if (text_animation >= 0.80f)
                                text_animation_hide = false;
                        }
                        else {
                            if (text_animation > 0.00f)
                                text_animation -= 0.03f / ImGui::GetIO().Framerate * 60.f;
                            if (text_animation <= 0.00f)
                                text_animation_hide = true;
                        }
                        loader_animation = 0;
                    }

                    if (text_animation_hide0) {
                        if (text_animation0 < 1.00f)
                            text_animation0 += 0.03f / ImGui::GetIO().Framerate * 60.f;
                        if (text_animation0 >= 1.00f)
                            text_animation_hide0 = false;
                    }
                    else {
                        if (text_animation0 > 0.00f)
                            text_animation0 -= 0.03f / ImGui::GetIO().Framerate * 60.f;

                        if (text_animation0 <= 0) copypst = false;
                    }

                    pWindowDrawList->AddText(Nev, 35.f, ImVec2(270 + CurrentWindowPos.x, 80 + CurrentWindowPos.y), ImColor(0.39f, 0.35f, 1.00f, text_animation), "SOFT.CC");

                    ImGui::PushFont(my_icon2);
                    ImGui::SetCursorPosX(720);
                    if (Custom::AnimButton("Z##exit1", ImVec2(35, 35))) exit(0);
                    ImGui::PopFont();

                    ImGuiWindow* window = ImGui::GetCurrentWindow();

                    if (copypst)
                        pWindowDrawList->AddText(ImVec2(285 + CurrentWindowPos.x, 330 + CurrentWindowPos.y), ImColor(1.00, 1.00, 1.00, text_animation0), just.response.message.c_str());



                    ImGui::BeginGroup(); {
                        ImGui::SetCursorPos({ 275, 200 });
                        Custom::BeginChild(" ", ImVec2(208, 300), true); {

                            ImGui::InputTextWithHint("##id", "Discord ID", id, IM_ARRAYSIZE(id));
                            ImGui::SetCursorPosY(55);
                            if (Custom::Configuration("Authorization", ImVec2(187, 30))) {
                                just.login(id);
                                if (just.response.sucess) {
                                    authenticed = true;
                                }
                                else {
                                    copypst = true;
                                    text_animation_hide0 = true;
                                }
                            }

                        }
                        Custom::EndChild();
                    }
                    ImGui::EndGroup();


                }
                ImGui::End();
            }
            if (hideTaskbar) {
                if (var::hideTaskbar) {
                    ITaskbarList* pTaskList = NULL;
                    HRESULT initRet = CoInitialize(NULL);
                    HRESULT createRet = CoCreateInstance(CLSID_TaskbarList,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_ITaskbarList,
                        (LPVOID*)&pTaskList);

                    if (createRet == S_OK)
                    {

                        pTaskList->DeleteTab(GetActiveWindow());

                        pTaskList->Release();
                    }

                    CoUninitialize();
                }
                else {
                    ITaskbarList* pTaskList = NULL;
                    HRESULT initRet = CoInitialize(NULL);
                    HRESULT createRet = CoCreateInstance(CLSID_TaskbarList,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_ITaskbarList,
                        (LPVOID*)&pTaskList);

                    if (createRet == S_OK)
                    {
                        pTaskList->AddTab(GetActiveWindow());

                        pTaskList->Release();
                    }

                    CoUninitialize();
                }
            }
            if (streamMode) {
                if (var::streamMode) {
                    SetWindowDisplayAffinity(GetActiveWindow(), WDA_EXCLUDEFROMCAPTURE);
                }
                else {
                    SetWindowDisplayAffinity(GetActiveWindow(), WDA_NONE);
                }
            }
            if (authenticed) {

                ImGui::Begin(LOADER_BRAND, &loader_active, window_flags); {

                    ImGuiStyle* style = &ImGui::GetStyle();

                    style->WindowPadding = ImVec2(0, 10);
                    style->ItemSpacing = ImVec2(10, 10);

                    RenderBlur(hwnd);   
                    if (particles) dot_draw();

                    ImGuiWindow* window = ImGui::GetCurrentWindow();
                    const auto& CurrentWindowPos = ImGui::GetWindowPos();
                    const auto& pWindowDrawList = ImGui::GetWindowDrawList();
                    const auto& pForegroundDrawList = ImGui::GetForegroundDrawList();

                    ImGui::PushFont(my_icon2);
                    ImGui::SetCursorPos({ 10, 470});
                    if (Custom::AnimButton("Z##exit1", ImVec2(35, 35))) exit(0);
                    ImGui::PopFont();


                    if (!loader_animation)
                        loader_animation = GetTickCount();

                    if (GetTickCount() > loader_animation + (1 * 15))
                    {

                        if (text_animation_hide) {
                            if (text_animation < 0.80f)
                                text_animation += 0.03f / ImGui::GetIO().Framerate * 60.f;

                            if (text_animation >= 0.80f)
                                text_animation_hide = false;
                        }
                        else {
                            if (text_animation > 0.00f)
                                text_animation -= 0.03f / ImGui::GetIO().Framerate * 60.f;
                            if (text_animation <= 0.00f)
                                text_animation_hide = true;
                        }
                        loader_animation = 0;
                    }


                    pForegroundDrawList->AddText(Nev, 15.f, CurrentWindowPos + ImVec2((ImGui::GetWindowWidth() - ImGui::CalcTextSize("SOFT.CC").x) / 2.f, 15), ImColor(0.39f, 0.35f, 1.00f, text_animation), "SOFT.CC");
                    pWindowDrawList->AddRectFilled(CurrentWindowPos + ImVec2(0, 0), CurrentWindowPos + ImVec2(WIDTH, 45), ImColor(32, 32, 48), 10.f, ImDrawCornerFlags_Top); // background
                    pForegroundDrawList->AddLine(CurrentWindowPos + ImVec2(171, 45), CurrentWindowPos + ImVec2(171, HEIGHT), ImColor(32, 32, 48), 1.f);


                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7, 7));
                    ImGui::SetCursorPos(ImVec2(7, 53));
                    ImGui::BeginGroup();
                    {

                        if (Custom::Tab(0 == tabs, "B", "Security", ImVec2(157, 35))) tabs = 0;
                        if (Custom::Tab(1 == tabs, "A", "Aim", ImVec2(157, 35))) tabs = 1;
                        if (Custom::Tab(2 == tabs, "C", "Visuals", ImVec2(157, 35))) tabs = 2;
                        if (Custom::Tab(3 == tabs, "E", "Weapon", ImVec2(157, 35))) tabs = 3;
                        if (Custom::Tab(4 == tabs, "F", "Misc", ImVec2(157, 35))) tabs = 4;
                        if (Custom::Tab(5 == tabs, "D", "Settings", ImVec2(157, 35))) tabs = 5;

                    }
                    ImGui::EndGroup();
                    ImGui::PopStyleVar();

                    tab_alpha = ImClamp(tab_alpha + (6.f * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);
                    tab_add = ImClamp(tab_add + (std::round(350.f) * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);

                    if (tab_alpha == 0.f && tab_add == 0.f)
                        active_tab = tabs;

                    ImGui::SetCursorPos(ImVec2(180, 55));

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha* style->Alpha); // open smooth tabs

                    std::string aja = "Status:" + messger;

                    if (text_animation_hide0) {
                        if (text_animation0 < 1.00f)
                            text_animation0 += 0.03f / ImGui::GetIO().Framerate * 60.f;
                        if (text_animation0 >= 1.00f)
                            text_animation_hide0 = false;
                    }
                    else {
                        if (text_animation0 > 0.00f)
                            text_animation0 -= 0.01f / ImGui::GetIO().Framerate * 40.f;

                        if (text_animation0 <= 0) copypst = false;
                    }

                    if (copypst)
                        pForegroundDrawList->AddText(ImVec2(180 + CurrentWindowPos.x, 490 + CurrentWindowPos.y), ImColor(0.20f, 1.00f, 0.20f, text_animation0), messger.c_str());
                    else
                        pForegroundDrawList->AddText(ImVec2(180 + CurrentWindowPos.x, 490 + CurrentWindowPos.y), ImColor(1.00f, 0.20f, 0.20f, text_animation0), messger.c_str());

                    if (active_tab == 0) {

                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild("Security", ImVec2(590, 453), true);
                            {
                                antBlacklist = Custom::Checkbox("Bypass BlackList", &var::antBlacklist);
                                if (Custom::Configuration("Bypass Screen Share", ImVec2({ 580, 35}))) {
                                    bypass.destructAll();
                                }
                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();
                    }

                    if (active_tab == 1) {

                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild("Aim's", ImVec2(590, 453), true);
                            {
                                aimbotHead = Custom::Checkbox("Aimbot - Head", &var::aimbotHead);
                                aimbotNeck = Custom::Checkbox("Aimbot - Neck", &var::aimbotNeck);
                                aimbotTrick = Custom::Checkbox("Aimbot - Trick", &var::aimbotTrick);
                                aimbotScope = Custom::Checkbox("Aimbot - Scope", &var::aimbotScope);
                                aimbotSniper = Custom::Checkbox("Aimbot - Sniper", &var::aimbotSniper);
                                aimfovLegit = Custom::Checkbox("Aimfov - Legit", &var::aimfovLegit);
                                aimfovFull = Custom::Checkbox("Aimfov - Full", &var::aimfovFull);
                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();
                    }

                    if (active_tab == 2) {

                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild("Visuals", ImVec2(590, 453), true);
                            {
                                espName = Custom::Checkbox("ESP Name", &var::espName);
                                espArrow = Custom::Checkbox("ESP Arrow", &var::espArrow);
                                espLaser = Custom::Checkbox("ESP Laser", &var::espLaser);
                                AntenaHandFem = Custom::Checkbox("Antena Hand - F", &var::AntenaHandFem);
                                AntenaHandMasc = Custom::Checkbox("Antena Hand - M", &var::AntenaHandMasc);
                                fakedamage = Custom::Checkbox("Fake Damage", &var::fakedamage);
                                increaseVision = Custom::Checkbox("Increase Vision", &var::increaseVision);
                                onlyRed = Custom::Checkbox("Only Red", &var::onlyRed);
                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();
                    }

                    if (active_tab == 3) {

                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild("Weapons", ImVec2(590, 453), true);
                            {
                                norecoil = Custom::Checkbox("No Recoil", &var::norecoil);
                                fastSwitchSniper = Custom::Checkbox("Fast Switch - Sniper", &var::fastSwitchSniper);
                                precision = Custom::Checkbox("Precision", &var::precision);
                                aimlock = Custom::Checkbox("Aimlock", &var::aimlock);
                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();

                    }

                    if (active_tab == 4) {

                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild("Misc", ImVec2(590, 453), true);
                            {
                                bypassEmulador = Custom::Checkbox("Bypass Emulator - All (Except 4.240)", &var::bypassEmulador);
                                bypassEmulador280 = Custom::Checkbox("Bypass Emulator - 4.280", &var::bypassEmulador280);
                                alokFix64bits = Custom::Checkbox("Alok Fix - 64Bits", &var::alokFix64bits);
                                fly = Custom::Checkbox("Fly", &var::fly);
                                fastMedkit = Custom::Checkbox("Fast Medkit", &var::fastMedkit);
                                magicBullets = Custom::Checkbox("Magic Bullet", &var::magicBullets);

                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();

                    }

                    if (active_tab == 5) {
                        ImGui::BeginGroup();
                        {
                            ImGui::BeginChild("Login Information", ImVec2(568, 100), true);
                            {

                                std::string expiry = "Expiry: ";
                                std::tm t{};
                                std::stringstream ss(just.user.expiry);
                                ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");

                                char buffer[80];
                                std::strftime(buffer, sizeof(buffer), "%d/%m/%Y", &t);

                                expiry += buffer;

                                std::string hwid = "HWID: " + just.user.hwid;

                                ImGui::SetCursorPosX(25);
                                ImGui::Text(expiry.c_str());

                                ImGui::SetCursorPosX(25);
                                ImGui::Text(hwid.c_str());

                            }
                            ImGui::EndChild();

                            ImGui::BeginChild("Settings", ImVec2(590, 453), true);
                            {
                                streamMode = Custom::Checkbox("Stream Mode", &var::streamMode);
                                hideTaskbar = Custom::Checkbox("Hide Taskbar", &var::hideTaskbar);
                                PauseAndResume = Custom::Checkbox("Pause and Resume", &var::PauseAndResume);
                                Custom::Checkbox("Particles", &particles);

                                ImGui::SetCursorPosX(25);
                                ImGui::Text("Minimize Process: ");
                                ImGui::SameLine();
                                ImGui::TextColored(ImColor(100, 88, 255, 255), "HOME");

                                ImGui::SetCursorPosX(25);
                                ImGui::Text("Close Process: ");
                                ImGui::SameLine();
                                ImGui::SetCursorPosX(175);
                                ImGui::TextColored(ImColor(100, 88, 255, 255), "DEL");
                            }
                            ImGui::EndChild();
                        }
                        ImGui::EndGroup();
                    }
                    ImGui::PopStyleVar(); // close smooth tabs

                    if (bypassEmulador280 || bypassEmulador || antBlacklist || norecoil || aimbotHead || aimbotNeck || aimbotTrick || aimbotScope || aimbotSniper || aimfovLegit || aimfovFull || alokFix64bits || fakedamage || increaseVision || onlyRed || fastSwitchSniper || precision || aimlock || fly || fastMedkit || magicBullets || espName || espLaser || espArrow || AntenaHandFem || AntenaHandMasc || PauseAndResume) {

                        std::thread t(threadfunction);
                        t.detach();
                    }
                }
                ImGui::End();
            }

            DnsFlushResolverCacheFuncPtr DnsFlushResolverCache = (DnsFlushResolverCacheFuncPtr)GetProcAddress(dnsapi, "DnsFlushResolverCache");
            if (DnsFlushResolverCache != NULL) {

                BOOL result = DnsFlushResolverCache();
                if (result) {
                    copypst = true;
                    text_animation_hide0 = true;
                    messger = "dns cleaned";
                }
            }


            FreeLibrary(dnsapi);
        }

        // Rendering
        ImGui::EndFrame();

        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            hook_reset(g_pd3dDevice, &g_d3dpp);
        }
        if (!loader_active) {
            msg.message = WM_QUIT;
        }
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

HRESULT WINAPI hook_reset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    D3DDEVICE_CREATION_PARAMETERS d3dx9{ 0 };
    ImGui_ImplDX9_InvalidateDeviceObjects();
    auto hook = pDevice->Reset(&g_d3dpp);
    ImGui_ImplDX9_CreateDeviceObjects();
    return hook;
}

void RenderBlur(HWND hwnd)
{
    struct ACCENTPOLICY
    {
        int na;
        int nf;
        int nc;
        int nA;
    };
    struct WINCOMPATTRDATA
    {
        int na;
        PVOID pd;
        ULONG ul;
    };

    const HINSTANCE hm = LoadLibrary("user32.dll");
    if (hm)
    {
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            ACCENTPOLICY policy = { 3, 0, 0, 0 }; // and even works 4,0,155,0 (Acrylic blur)
            WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(hwnd, &data);
        }
        FreeLibrary(hm);
    }
}


void threadfunction() {
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

    // Security
    if (antBlacklist) {
        if (var::antBlacklist) {
            memory.AntiBlacklist();
        }
        else {
            copypst = false;
            text_animation_hide0 = true;
            messger = "Function cannot be disabled";
        }
    }

    // Aim's
    if (aimbotHead) {
        if (var::aimbotHead) {
            memory.AimbotHead(true);
        }
        else {
            memory.AimbotHead(false);
        }
    }

    if (aimbotNeck) {
        if (var::aimbotNeck) {
            memory.AimbotNeck(true);
        }
        else {
            memory.AimbotNeck(false);
        }
    }

    if (aimbotTrick) {
        if (var::aimbotTrick) {
            memory.AimbotTrick(true);
        }
        else {
            memory.AimbotTrick(false);
        }
    }

    if (aimbotScope) {
        if (var::aimbotScope) {
            memory.AimbotScope(true);
        }
        else {
            memory.AimbotScope(false);
        }
    }

    if (aimbotSniper) {
        if (var::aimbotSniper) {
            memory.AimbotSniper(true);
        }
        else {
            memory.AimbotSniper(false);
        }
    }

    if (aimfovLegit) {
        if (var::aimfovLegit) {
            memory.AimfovLegit(true);
        }
        else {
            memory.AimfovLegit(false);
        }
    }

    if (aimfovFull) {
        if (var::aimfovFull) {
            memory.AimfovFull(true);
        }
        else {
            memory.AimfovFull(false);
        }
    }

    // Visuals
    if (espName) {
        if (var::espName) {
            memory.ESPName(true);
        }
        else {
            memory.ESPName(false);
        }
    }

    if (espArrow) {
        if (var::espArrow) {
            memory.ESPArrow(true);
        }
        else {
            memory.ESPArrow(false);
        }
    }

    if (espLaser) {
        if (var::espLaser) {
            memory.ESPLaser(true);
        }
        else {
            memory.ESPLaser(false);
        }
    }

    if (AntenaHandFem) {
        if (var::AntenaHandFem) {
            memory.AntenaHandF(true);
        }
        else {
            memory.AntenaHandF(false);
        }
    }

    if (AntenaHandMasc) {
        if (var::AntenaHandMasc) {
            memory.AntenaHandM(true);
        }
        else {
            memory.AntenaHandM(false);
        }
    }

    if (fakedamage) {
        if (var::fakedamage) {
            memory.FakeDamage(true);
        }
        else {
            memory.FakeDamage(false);
        }
    }

    if (increaseVision) {
        if (var::increaseVision) {
            memory.IncreaseVision(true);
        }
        else {
            memory.IncreaseVision(false);
        }
    }

    if (onlyRed) {
        if (var::onlyRed) {
            memory.OnlyRed(true);
        }
        else {
            memory.OnlyRed(false);
        }
    }


    // Weapon
    if (norecoil) {
        if (var::norecoil) {
            memory.NoRecoil(true);
        }
        else {
            memory.NoRecoil(false);
        }
    }

    if (fastSwitchSniper) {
        if (var::fastSwitchSniper) {
            memory.FastWitchSniper(true);
        }
        else {
            memory.FastWitchSniper(false);
        }
    }

    if (precision) {
        if (var::precision) {
            memory.Precision(true);
        }
        else {
            memory.Precision(false);
        }
    }

    if (aimlock) {
        if (var::aimlock) {
            memory.Aimlock(true);
        }
        else {
            memory.Aimlock(false);
        }
    }

    // Misc
    if (bypassEmulador) {
        if (var::bypassEmulador) {
            memory.BypassEmulador();
        }
        else {
            copypst = false;
            text_animation_hide0 = true;
            messger = "Function cannot be disabled";
        }
    }

    if (bypassEmulador280) {
        if (var::bypassEmulador280) {
            memory.BypassEmulador280();
        }
        else {
            copypst = false;
            text_animation_hide0 = true;
            messger = "Function cannot be disabled";
        }
    }

    if (fly) {
        if (var::fly) {
            memory.Fly(true);
        }
        else {
            memory.Fly(false);
        }
    }

    if (fastMedkit) {
        if (var::fastMedkit) {
            memory.FastMedkit(true);
        }
        else {
            memory.FastMedkit(false);
        }
    }

    if (magicBullets) {
        if (var::magicBullets) {
            memory.MagicBullets(true);
        }
        else {
            memory.MagicBullets(false);
        }
    }




    if (bypassSS) {
        if (bypassSS) {
            if (dns) {
                
            }
        }
    }
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, false);
    ImGui::PopStyleVar();
}

bool OpenRegistry() {
    HKEY hKey;
    LPCTSTR sk = TEXT(just.response.key.c_str());

    LONG openRes = RegOpenKeyEx(HKEY_CURRENT_USER, sk, 0, KEY_ALL_ACCESS, &hKey);

    if (openRes == ERROR_SUCCESS) {
        return true;
    }
    else {
        return false;
    }

    RegCloseKey(hKey);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        just.init();
        if (just.response.sucess) {
            if (OpenRegistry()) {
                DisableThreadLibraryCalls(hinstDLL);
                CreateThread(nullptr, NULL, (LPTHREAD_START_ROUTINE)WinMain, nullptr, NULL, nullptr);
            }
        }
        return TRUE;
    }
    return TRUE;
}