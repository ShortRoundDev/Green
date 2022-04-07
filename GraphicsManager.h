#pragma once
#include "IManager.h"

#include "GTypes.h"
#include "GlobalBuffer.h"

#include "Windows.h"
#include "d3d11.h"
#include <wrl/client.h>
#include <DirectXMath.h>
#include <DirectXTK/Mouse.h>

#include <unordered_map>

using namespace DirectX;
using namespace Microsoft::WRL;

struct KeyState
{
    bool down;
    bool edge;
    u64 lastDown;
};

////////// FORWARD DECLARATIONS //////////
class Shader;
class Texture;

class GraphicsManager : public IManager
{
public:
    virtual bool start();
    virtual bool shutDown();
    
    void clear();
    void swap();
    void bindGlobalBuffer();
    bool update();

    void putShader(std::wstring name, Shader* shader);
    Shader* getShader(std::wstring name);

    void putTexture(std::string name, Texture* texture);
    Texture* getTexture(std::string name);

    ID3D11Device* getDevice();
    ID3D11DeviceContext* getContext();

    HWND getWindow();

    i32 diffX();
    i32 diffY();
    bool keyDown(u32 key);

    void setViewMatrix(const XMMATRIX& view);

    void resetRenderTarget();
    void resetViewport();

    LRESULT CALLBACK messageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

private:
    ///// WIN32 STUFF /////
    HWND        m_window;
    HINSTANCE   m_instance;

    ///// DIRECTX STUFF /////
    ComPtr<IDXGISwapChain> m_swapchain;
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
    ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    ComPtr<ID3D11RasterizerState> m_rasterState;
    D3D11_VIEWPORT m_viewport;

    i32 m_numerator;
    i32 m_denominator;

    XMMATRIX m_projection;
    XMMATRIX m_ortho;
    XMMATRIX m_world;
    XMMATRIX m_view;
    XMFLOAT4 m_camera;

    ComPtr<ID3D11Buffer> m_gBufferBuffer;

    GlobalBuffer m_gBuffer;

    ///// FRAMEWORK STUFF /////
    std::unordered_map<std::wstring, Shader*> m_shaders;
    std::unordered_map<std::string, Texture*> m_textures;

    ///// INPUT STUFF /////
    
    i32 m_mouseX;
    i32 m_mouseY;

    KeyState m_keys[256];
    Mouse* m_mouse;

    void onKeyDown(u32 key);
    void onKeyUp(u32 key);

    bool initWindow();
    bool initDX();
    bool initInfrastructure();
    bool initSwapchain();
    bool initBackBuffer();
    bool initDepthStencilBuffer();
    bool initRasterizer();
    bool initShaders();
    bool initGlobalBuffer();

    inline void inverseTranspose(const XMMATRIX& world, XMMATRIX& invWorld);

};

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsh, WPARAM wParam, LPARAM lParam);

extern GraphicsManager Graphics;
