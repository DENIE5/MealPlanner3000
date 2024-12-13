#include <iostream>
#include <ctime>
#include <windows.h>
#include <cstdlib>
#include <limits>
#include <d3d11.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <tchar.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

// Global variables for DirectX initialization
ID3D11Device* g_Device = nullptr;
ID3D11DeviceContext* g_DeviceContext = nullptr;
IDXGISwapChain* g_SwapChain = nullptr;
HWND g_Hwnd = nullptr;

//back buffer
ID3D11RenderTargetView* g_RenderTargetView = nullptr;

// Function to initialize ImGui
void InitializeImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(g_Hwnd);  // Initialize Win32 backend
    ImGui_ImplDX11_Init(g_Device, g_DeviceContext);  // Initialize DirectX 11 backend
}

// Function to clean up ImGui
void CleanupImGui()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

// Function to render ImGui UI
void RenderImGui()
{
    //clear the back buffer otherwise u get an glitchy background as the screen is never cleared
    static const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; //clear so no background
    g_DeviceContext->ClearRenderTargetView(g_RenderTargetView, clear_color);

    // Start a new ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Example ImGui UI
    ImGui::Begin("Hello, world!");
    ImGui::Text("This is a simple ImGui window.");
    ImGui::End();

    // Render the ImGui frame
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

// Window procedure function to handle messages
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
        return true;

    switch (uMsg)
    {
    case WM_SIZE:
        if (g_Device != nullptr && wParam != SIZE_MINIMIZED)
        {
            // Resize swap chain when the window size changes
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_CLOSE:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Function to create the window and initialize DirectX
bool InitializeWindowAndDX(HINSTANCE hInstance, int nCmdShow)
{
    // Register the window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("MealPicker"), NULL };
    RegisterClassEx(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN); // X (width)
    int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Y (height)

    //make the dx window insiable
    g_Hwnd = CreateWindowExW(
        WS_EX_LAYERED,  // Layered, Always on top, Transparent to mouse
        wc.lpszClassName,
        _T("Meal Picker"),
        WS_POPUP,  // No border or title bar
        0,
        0,
        screenWidth,  //screen x
        screenHeight,   //screen y
        NULL,
        NULL,
        wc.hInstance,
        NULL
    );
    SetLayeredWindowAttributes(g_Hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Initialize DirectX
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = screenWidth;
    swapChainDesc.BufferDesc.Height = screenHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = g_Hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &g_SwapChain, &g_Device, &featureLevel, &g_DeviceContext);

    if (FAILED(hr))
    {
        std::cerr << "Failed to create Direct3D device and swap chain." << std::endl;
        return false;
    }

    // u need an back buffer otherwise imgui wont show
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    g_Device->CreateRenderTargetView(pBackBuffer, NULL, &g_RenderTargetView);
    pBackBuffer->Release();

    g_DeviceContext->OMSetRenderTargets(1, &g_RenderTargetView, NULL);

    ShowWindow(g_Hwnd, nCmdShow);
    UpdateWindow(g_Hwnd);
    return true;
}

// Meal selection function
void chooseMeal() {
    std::string beef[7] = { "Shepherd's Pie", "Mac n Cheese + Steak", "Meatball Bake", "Chilli con carne", "Noodles + Beef + Broccoli", "Burgers", "Beef Stew" };
    std::string chicken[9] = { "Teriyaki Chicken", "Roast Chicken", "Pesto Pasta + Goujons", "Chicken Wraps", "Chicken Curry", "Chicken Breast + Rice + Veg", "Spanish Chicken + Peppers + Rice", "Herb Chicken + Roast potatoes", "Chicken + cacik + rice" };
    std::string pork[7] = { "Teriyaki Pork", "Gammon Steaks + Sweet Potatoes", "Sausages + Cauliflower + Potatoes", "Chipos + Chips + Apple sauce", "Pork Chops + Mash + Peas", "BBQ chipos", "Hot dogs" };
    std::string lamb[7] = { "Lamb + Mash + Aubergine", "Kavurma", "Roast Lamb", "Merguez + Couscous", "Lamb + Beans + Potatoes", "Lamb patties + couscous", "Fasulyia" };
    std::string fish[9] = { "Salmon + pasta + peas", "Fish fingers + mash + peas", "Salmon + Beans + potatoes", "Fish and chips", "White fish + Asparagus + Potatoes", "White fish + Salad + Potatoes", "Salmon + Noodles + Broccoli", "Tuna steaks + sweet potatoes + salad", "Fishcakes + Chips" };
    std::string pasta[7] = { "Bolognese", "Pasta + Aubergine", "Pesto pasta", "Carbonara", "Prawn Spaghetti", "Pasta bake", "Ravioli" };
    std::string soup[8] = { "Butternut Squash soup", "Carrot soup", "Lentil Soup", "Vegetable soup", "Potato and Fish soup", "Broccoli and Brie Soup", "Minestrone", "Leek and peas soup" };

    // Seed once per run
    static bool seeded = false;
    if (!seeded) {
        std::srand(std::time(0));
        seeded = true;
    }

    char choice;
    do {
        // Randomly selecting meals
        int beefIndex = std::rand() % 7;
        int chickenIndex = std::rand() % 9;
        int porkIndex = std::rand() % 7;
        int lambIndex = std::rand() % 7;
        int fishIndex = std::rand() % 9;
        int pastaIndex = std::rand() % 7;
        int soupIndex = std::rand() % 8;

        // Print the randomly selected meals
        std::cout << "\nRandomly Selected Meals:\n\n";
        std::cout << "Monday: " << beef[beefIndex] << "\n\n";
        std::cout << "Tuesday: " << chicken[chickenIndex] << "\n\n";
        std::cout << "Wednesday: " << pork[porkIndex] << "\n\n";
        std::cout << "Thursday: " << lamb[lambIndex] << "\n\n";
        std::cout << "Friday: " << fish[fishIndex] << "\n\n";
        std::cout << "Saturday: " << pasta[pastaIndex] << "\n\n";
        std::cout << "Sunday: " << soup[soupIndex] << "\n\n";

        // Ask the user if they want to reroll
        std::cout << "\nDo you want to reroll the meals? N will return to the main menu! (y/n): ";
        std::cin >> choice;

        // Clear the input buffer to avoid unwanted issues with std::cin
        std::cin.ignore();

    } while (choice == 'y' || choice == 'Y'); // Continue rerolling if user inputs 'y' or 'Y'
}

void customRand() {
    // Meal categories arrays of arrays
    std::string categories[7][9] = {
        { "Shepherd's Pie", "Mac n Cheese + Steak", "Meatball Bake", "Chilli con carne", "Noodles + Beef + Broccoli", "Burgers", "Beef Stew" }, // beef
        { "Teriyaki Chicken", "Roast Chicken", "Pesto Pasta + Goujons", "Chicken Wraps", "Chicken Curry", "Chicken Breast + Rice + Veg", "Spanish Chicken + Peppers + Rice", "Herb Chicken + Roast potatoes", "Chicken + cacik + rice" }, // chicken
        { "Teriyaki Pork", "Gammon Steaks + Sweet Potatoes", "Sausages + Cauliflower + Potatoes", "Chipos + Chips + Apple sauce", "Pork Chops + Mash + Peas", "BBQ chipos", "Hot dogs" }, // pork
        { "Lamb + Mash + Aubergine", "Kavurma", "Roast Lamb", "Merguez + Couscous", "Lamb + Beans + Potatoes", "Lamb patties + couscous", "Fasulyia" }, // lamb
        { "Salmon + pasta + peas", "Fish fingers + mash + peas", "Salmon + Beans + potatoes", "Fish and chips", "White fish + Asparagus + Potatoes", "White fish + Salad + Potatoes", "Salmon + Noodles + Broccoli", "Tuna steaks + sweet potatoes + salad", "Fishcakes + Chips" }, // fish
        { "Bolognese", "Pasta + Aubergine", "Pesto pasta", "Carbonara", "Prawn Spaghetti", "Pasta bake", "Ravioli" }, // pasta
        { "Butternut Squash soup", "Carrot soup", "Lentil Soup", "Vegetable soup", "Potato and Fish soup", "Broccoli and Brie Soup", "Minestrone", "Leek and peas soup" } // soup
    };

    int categorySizes[7] = { 7, 9, 7, 7, 9, 7, 8 };
    std::string days[7] = { "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

    std::cout << "Choose a category for each day:\n\n";
    std::cout << "[0] - Beef\n[1] - Chicken\n[2] - Pork\n[3] - Lamb\n[4] - Fish\n[5] - Pasta\n[6] - Soup\n";


    char rerollChoice;

    do {
        for (int i = 0; i < 7; ++i) {
            int choice;
            std::cout << days[i] << ": ";
            std::cin >> choice;

            // Validate input
            while (choice < 0 || choice >= 7) {
                std::cout << "Invalid choice. Please enter a number between 0 and 6: ";
                std::cin >> choice;
            }

            int mealIndex = std::rand() % categorySizes[choice];
            std::cout << days[i] << ": " << categories[choice][mealIndex] << "\n\n";

        }

        // Ask the user if they want to reroll
        std::cout << "\nDo you want to reroll the meals? N will return to the main menu! (y/n): ";
        std::cin >> rerollChoice;


        // Clear the input buffer to avoid unwanted issues with std::cin

    } while (rerollChoice == 'y' || rerollChoice == 'Y');

}

// Main function
int main()
{
    // Initialize window and DirectX
    if (!InitializeWindowAndDX(GetModuleHandle(NULL), SW_SHOW))
        return -1;

    // Initialize ImGui
    InitializeImGui();

    // Main loop
    bool running = true;
    MSG msg;
    while (running)
    {
        // Handle messages and events
        while (PeekMessage(&msg, g_Hwnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                running = false;
        }

        // Render ImGui UI
        RenderImGui();

        // Render your DirectX 11 frame (swap buffers or present)
        g_SwapChain->Present(1, 0);  // DirectX 11 present call
    }

    // Clean up ImGui resources before exiting
    CleanupImGui();

    return 0;
}
