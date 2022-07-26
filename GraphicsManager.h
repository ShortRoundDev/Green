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

#include "imgui.h"

using namespace DirectX;
using namespace Microsoft::WRL;

constexpr u32 SHADOW_RES = 512;

struct OrthoView
{
    XMMATRIX ortho;
};

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
    void draw();

    void putShader(std::wstring name, Shader* shader);
    Shader* getShader(std::wstring name);

    void putTexture(std::string name, Texture* texture);
    Texture* getTexture(std::string name);
    Texture* lazyLoadTexture(std::string name);

    ID3D11Device* getDevice();
    ID3D11DeviceContext* getContext();

    HWND getWindow();

    i32 diffX();
    i32 diffY();
    bool keyDown(u32 key);
    bool keyDownEdge(u32 key);

    void setViewMatrix(const XMMATRIX& view);
    void setCameraPos(const XMFLOAT3& pos);

    void resetRenderTarget();
    void resetViewport();
    void setShadowRasterizer(bool shadowRasterizer);
    void setWireframe(bool on);

    GlobalBuffer m_gBuffer;

    f32 getClientWidth();
    f32 getClientHeight();

    bool getMouseLook();
    void setMouseLook(bool mouseLook);

    XMFLOAT4 clearColor = { 4.0f / 255.0f, 3.0f / 255.0f, 30.0f / 255.0f, 1.0f };

    LRESULT CALLBACK messageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam);

    void drawQuad();

    XMMATRIX m_projection;
    XMMATRIX m_ortho;
    XMMATRIX m_world;
    XMMATRIX m_view;
    XMFLOAT3 m_camera;

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
    ComPtr<ID3D11RasterizerState> m_sceneRasterizer;
    ComPtr<ID3D11RasterizerState> m_wireRasterizer;
    ComPtr<ID3D11RasterizerState> m_shadowMapRasterizer;

    D3D11_VIEWPORT m_viewport;

    i32 m_numerator;
    i32 m_denominator;


    ComPtr<ID3D11Buffer> m_gBufferBuffer;

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
    bool initQuad();

    ComPtr<ID3D11Buffer> m_quadVertexBuffer;
    ComPtr<ID3D11Buffer> m_quadIndexBuffer;

    f32 m_clientWidth;
    f32 m_clientHeight;

    bool m_mouseLook;

    inline void inverseTranspose(const XMMATRIX& world, XMMATRIX& invWorld);
};

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsh, WPARAM wParam, LPARAM lParam);

extern GraphicsManager Graphics;
