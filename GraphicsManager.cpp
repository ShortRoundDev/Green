#include "GraphicsManager.h"

#include "Logger.h"
#include "SystemManager.h"
#include "Shader.h"
#include "Mesh.h"
#include "ILight.h"
#include "Texture.h"
#include "Sprite.h"

#include "MeshLightBuffer.h"
#include "WireframeDebugColor.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

static ::Logger logger = CreateLogger("GraphicsManager");

////////// PUBLIC //////////

bool GraphicsManager::start()
{
	///// WINDOWS /////
	if (!initWindow())
	{
		logger.err("Failed to initialize window!");
		return false;
	}
	if (!initDX())
	{
		logger.err("Failed to initialize DirectX!");
		return false;
	}

	ZeroMemory(&m_gBuffer, sizeof(m_gBuffer));

	m_gBuffer.pointradius = 0.0001f;
	m_gBuffer.dirLight.ambientA = { 21 / 255.0f, 9 / 255.0f, 3/ 255.0f, 72 / 255.0f };
	m_gBuffer.dirLight.ambientB = { 214 / 255.0f, 122 / 255.0f, 65 / 255.0f, 8 / 255.0f };
	m_gBuffer.dirLight.hardness = 1.3f;
	m_gBuffer.dirLight.ambientDirection = { 1, 0, -0.4f, 0.0f };

	///// IMGUI /////
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(getWindow());
	ImGui_ImplDX11_Init(m_device.Get(), m_deviceContext.Get());

	///// INPUT /////
	ZeroMemory(&m_keys, sizeof(m_keys));

	m_mouse = new Mouse;
	m_mouse->SetWindow(Graphics.getWindow());
	m_mouse->SetMode(Mouse::MODE_RELATIVE);

	m_mouseLook = true;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

	initQuad();

	return true;
}

bool GraphicsManager::shutDown()
{
	return true;
}

void GraphicsManager::clear()
{
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), (f32*)(&clearColor));
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void GraphicsManager::swap()
{
	//todo: add vsync option
	m_swapchain->Present(1, 0);
}

void GraphicsManager::bindGlobalBuffer()
{
	m_gBuffer.world		 = XMMatrixTranspose(m_world);
	m_gBuffer.projection = XMMatrixTranspose(m_projection);
	m_gBuffer.view		 = XMMatrixTranspose(m_view);
	m_gBuffer.camera	 = { m_camera.x, m_camera.y, m_camera.z, 1.0f };

	inverseTranspose(m_gBuffer.world, m_gBuffer.invWorld);

	//XMMATRIX tWorld = XMMatrixTranspose(m_gBuffer.world);

	D3D11_MAPPED_SUBRESOURCE gBufferResource;

	////////// CRITICAL SECTION //////////
	HRESULT result = m_deviceContext->Map(m_gBufferBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &gBufferResource);
	{
		if (FAILED(result))
		{
			logger.err("Failed to lock global buffer! Got 0x%x", result);
			return;
		}

		GlobalBuffer* localGBuffer = (GlobalBuffer*)gBufferResource.pData;
		CopyMemory(localGBuffer, &m_gBuffer, sizeof(GlobalBuffer));
	}
	m_deviceContext->Unmap(m_gBufferBuffer.Get(), 0);
	////////// END CRITICAL SECTION //////////

	m_deviceContext->VSSetConstantBuffers(0, 1, m_gBufferBuffer.GetAddressOf());
	m_deviceContext->PSSetConstantBuffers(0, 1, m_gBufferBuffer.GetAddressOf());

	m_frustum.load(1000.0f, m_projection, m_view);
}

bool GraphicsManager::update()
{
	for (int i = 0; i < 256; i++)
	{
		if (m_keys[i].down)
		{
			m_keys[i].edge = false;
		}
	}

	MSG msg = { };
	ZeroMemory(&msg, sizeof(msg));

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
		{
			return true;
		}
	}

	if (m_mouseLook)
	{
		auto state = m_mouse->GetState();
		m_mouseX = state.x;
		m_mouseY = state.y;
	}

	return false;
}

void GraphicsManager::draw()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void GraphicsManager::putShader(std::wstring name, Shader* shader)
{
	if (m_shaders.find(name) != m_shaders.end())
	{
		logger.err(L"Shader %s already exists!", name);
		return;
	}

	m_shaders[name] = shader;
}

Shader* GraphicsManager::getShader(std::wstring name)
{
	auto shader = m_shaders.find(name);
	if (shader == m_shaders.end())
	{
		return NULL;
	}

	return shader->second;
}

void GraphicsManager::putTexture(std::string name, Texture* texture)
{
	if (m_textures.find(name) != m_textures.end())
	{
		logger.err(L"Texture %s already exists!", name);
		return;
	}

	m_textures[name] = texture;

}

Texture* GraphicsManager::getTexture(std::string name)
{
	auto texture = m_textures.find(name);
	if (texture == m_textures.end())
	{
		return NULL;
	}

	return texture->second;
}

Texture* GraphicsManager::lazyLoadTexture(std::string path)
{
	auto texture = getTexture(path);
	if (texture == NULL)
	{
		texture = new Texture(path);
		putTexture(path, texture);
	}

	return texture;
}

Texture* GraphicsManager::lazyLoadTexture(std::string name, u8* data, sz size)
{
	auto texture = getTexture(name);
	if (texture == NULL)
	{
		texture = new Texture(data, size);
		putTexture(name, texture);
	}
	return texture;
}

ID3D11Device* GraphicsManager::getDevice()
{
	return m_device.Get();
}

ID3D11DeviceContext* GraphicsManager::getContext()
{
	return m_deviceContext.Get();
}

ID3D11InfoQueue* GraphicsManager::getInfo()
{
	return m_infoQueue.Get();
}

void GraphicsManager::printAndClearDebug(Logger* logger)
{
	if (!System.getVars().dxDebug)
	{
		return;
	}
	for (u64 i = 0; i < getInfo()->GetNumStoredMessages(); i++)
	{
		sz messageSize = 0;
		auto RES = getInfo()->GetMessage(i, NULL, &messageSize);
		D3D11_MESSAGE* message = (D3D11_MESSAGE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, messageSize);
		if (message == NULL)
		{
			logger->err("Failed to allocate message!");
			continue;
		}

		RES = getInfo()->GetMessage(i, message, &messageSize);
		if (RES == S_OK)
		{
			logger->err("%s", message->pDescription);
		}

		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, message);
	}
	getInfo()->ClearStoredMessages();
}

HWND GraphicsManager::getWindow()
{
	return m_window;
}

i32 GraphicsManager::diffX()
{
	if(m_mouseLook)
		return m_mouseX;
	return 0;
}

i32 GraphicsManager::diffY()
{
	if(m_mouseLook)
		return m_mouseY;
	return 0;
}

void GraphicsManager::setViewMatrix(const XMMATRIX& view)
{
	m_view = view;
}

void GraphicsManager::setCameraPos(const XMFLOAT3& pos)
{
	m_camera = pos;
}


void GraphicsManager::resetRenderTarget()
{
	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());
}

void GraphicsManager::resetViewport()
{
	m_deviceContext->RSSetViewports(1, &m_viewport);
}

void GraphicsManager::setShadowRasterizer(bool shadowRasterizer)
{
	if (shadowRasterizer)
	{
		m_deviceContext->RSSetState(m_shadowMapRasterizer.Get());
	}
	else
	{
		m_deviceContext->RSSetState(m_sceneRasterizer.Get());

	}
}

void GraphicsManager::setWireframe(bool on)
{
	if (on)
	{
		m_deviceContext->RSSetState(m_wireRasterizer.Get());
	}
	else
	{
		m_deviceContext->RSSetState(m_sceneRasterizer.Get());
	}
}

f32 GraphicsManager::getClientWidth()
{
	return m_clientWidth;
}

f32 GraphicsManager::getClientHeight()
{
	return m_clientHeight;
}

bool GraphicsManager::getMouseLook()
{
	return m_mouseLook;
}

void GraphicsManager::setMouseLook(bool mouseLook)
{
	m_mouseLook = mouseLook;
	m_mouse->SetMode(mouseLook ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	::ShowCursor(!mouseLook);
}

void GraphicsManager::drawQuad()
{
	UINT stride = sizeof(GVertex);
	UINT offset = 0;

	m_deviceContext->IASetVertexBuffers(0, 1, m_quadVertexBuffer.GetAddressOf(), &stride, &offset);
	m_deviceContext->IASetIndexBuffer(m_quadIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceContext->DrawIndexed(6, 0, 0);
}

LRESULT CALLBACK GraphicsManager::messageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_KEYDOWN:
		onKeyDown((UINT)wparam);
		return 0;
	case WM_KEYUP:
		onKeyUp((UINT)wparam);
		return 0;
	case WM_ACTIVATEAPP:
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::ProcessMessage(umsg, wparam, lparam);
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

////////// PRIVATE //////////

bool GraphicsManager::keyDown(u32 key)
{
	if (key < 0 || key >= 256)
	{
		return false;
	}

	return m_keys[key].down;
}

bool GraphicsManager::keyDownEdge(u32 key)
{
	return keyDown(key) && m_keys[key].edge;
}

void GraphicsManager::onKeyDown(u32 key)
{
	if (!m_keys[key].down)
	{
		m_keys[key].edge = true;
		//keys[key].lastDown = startOffset - GetTickCount64();
	}
	m_keys[key].down = true;

}

void GraphicsManager::onKeyUp(u32 key)
{
	if (m_keys[key].down)
	{
		m_keys[key].edge = true;
		//keys[key].lastDown = 0;
	}
	m_keys[key].down = false;
}

bool GraphicsManager::initWindow()
{

	auto& vars = System.getVars();
	auto title = vars.title.c_str();

	m_instance = GetModuleHandle(NULL);
	WNDCLASSEX wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_instance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = title;
	wc.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&wc))
	{
		return false;
	}

	u16 screenWidth, screenHeight;

	if (vars.fullScreen)
	{
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		screenWidth = vars.width;
		screenHeight = vars.height;
	}

	int posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2,
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	m_window = CreateWindowEx(
		WS_EX_APPWINDOW,
		title,
		title,
		WS_TILEDWINDOW,
		posX, posY,
		screenWidth, screenHeight,
		NULL,
		NULL,
		m_instance,
		NULL
	);

	ShowWindow(m_window, SW_SHOW);
	SetForegroundWindow(m_window);
	SetFocus(m_window);

	RECT rect;
	::GetClientRect(getWindow(), &rect);
	m_clientWidth = (f32)rect.right;
	m_clientHeight = (f32)rect.bottom;

	HRESULT result = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

bool GraphicsManager::initDX()
{
	if (!initInfrastructure())
	{
		logger.err("Failed to initialize DX Infrastructure!");
		return false;
	}

	if (!initSwapchain())
	{
		logger.err("Failed to initialize swapchain!");
		return false;
	}
	
	if (!initBackBuffer())
	{
		logger.err("Failed to initialize back buffer!");
		return false;
	}

	if (!initDepthStencilBuffer())
	{
		logger.err("Failed to initialize Depth/Stencil buffer!");
		return false;
	}

	if (!initRasterizer())
	{
		logger.err("Failed to initialize Rasterizer!");
		return false;
	}

	if (!initShaders())
	{
		logger.err("Failed to initialize shaders!");
		return false;
	}

	if (!initGlobalBuffer())
	{
		logger.err("Failed to initialize global cBuffer!");
		return false;
	}

	ZeroMemory(&m_gBuffer, sizeof(GlobalBuffer));

	return true;
}

bool GraphicsManager::initInfrastructure()
{
	ComPtr<IDXGIFactory> factory = NULL;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
	{
		return false;
	}

	IDXGIAdapter* adapter = NULL;
	if (FAILED(factory->EnumAdapters(0, &adapter)))
	{
		return false;
	}

	ComPtr<IDXGIOutput> output = NULL;
	if (FAILED(adapter->EnumOutputs(0, &output)))
	{
		return false;
	}

	//Count total
	UINT numModes = 0;
	if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL)))
	{
		return false;
	}

	//Actually enumerate now
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	ZeroMemory(displayModeList, numModes * sizeof(DXGI_MODE_DESC));
	if (FAILED(output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList)))
	{
		return false;
	}

	auto& vars = System.getVars();

	for (UINT i = 0; i < numModes; i++)
	{
		if ((displayModeList[i].Width == (UINT)vars.width) && (displayModeList[i].Height == (UINT)vars.height))
		{
			m_numerator = displayModeList[i].RefreshRate.Numerator;
			m_denominator = displayModeList[i].RefreshRate.Denominator;
			break;
		}
	}

	delete[] displayModeList;

	return true;
}

bool GraphicsManager::initSwapchain()
{
	auto& vars = System.getVars();

	DXGI_SWAP_CHAIN_DESC swapchainDesc = { };
	ZeroMemory(&swapchainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapchainDesc.BufferCount = 1;
	swapchainDesc.BufferDesc.Width = static_cast<u32>(m_clientWidth);
	swapchainDesc.BufferDesc.Height = static_cast<u32>(m_clientHeight);
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainDesc.BufferDesc.RefreshRate.Numerator = m_numerator; // or 0 if !vsync
	swapchainDesc.BufferDesc.RefreshRate.Denominator = m_denominator; // or 1 if !vsync
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.OutputWindow = m_window;
	swapchainDesc.SampleDesc.Count = 4;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.Windowed = !vars.fullScreen;
	swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapchainDesc.Flags = 0;

	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	if (
		FAILED(
			D3D11CreateDeviceAndSwapChain(
				NULL,
				D3D_DRIVER_TYPE_HARDWARE,
				NULL,
				vars.dxDebug ? (BYTE)D3D11_CREATE_DEVICE_DEBUG : 0,
				&featureLevel,
				1,
				D3D11_SDK_VERSION,
				&swapchainDesc,
				m_swapchain.GetAddressOf(),
				m_device.GetAddressOf(),
				NULL,
				m_deviceContext.GetAddressOf()
			)
		)
	)
	{
		return false;
	}

	if (vars.dxDebug)
	{
		if (FAILED(m_device->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)m_infoQueue.GetAddressOf())))
		{
			logger.warn("No debug info queue available");
		}
	}
	return true;
}

bool GraphicsManager::initBackBuffer()
{
	ComPtr<ID3D11Texture2D> backBuffer;
	if (FAILED(m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(backBuffer.GetAddressOf()))))
	{
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;

	if (FAILED(m_device->CreateRenderTargetView(backBuffer.Get(), &rtvDesc, m_renderTargetView.GetAddressOf())))
	{
		return false;
	}

	return true;
}

bool GraphicsManager::initDepthStencilBuffer()
{
	auto& vars = System.getVars();
	D3D11_TEXTURE2D_DESC depthBufferDesc = { };
	ZeroMemory(&depthBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthBufferDesc.Width = static_cast<u32>(m_clientWidth);
	depthBufferDesc.Height = static_cast<u32>(m_clientHeight);
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 4;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	if (FAILED(m_device->CreateTexture2D(&depthBufferDesc, NULL, m_depthStencilBuffer.GetAddressOf())))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { };
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xff;
	depthStencilDesc.StencilWriteMask = 0xff;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	if (FAILED(m_device->CreateDepthStencilState(&depthStencilDesc, m_depthStencilState.GetAddressOf())))
	{
		return false;
	}

	m_deviceContext->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = { };
	ZeroMemory(&depthStencilViewDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.GetAddressOf())))
	{
		return false;
	}

	m_deviceContext->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), m_depthStencilView.Get());

	return true;
}

bool GraphicsManager::initRasterizer()
{
	auto& vars = System.getVars();

	D3D11_RASTERIZER_DESC rasterDesc = { };
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.MultisampleEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.FrontCounterClockwise = false;

	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_sceneRasterizer.GetAddressOf())))
	{
		return false;
	}

	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FillMode = D3D11_FILL_WIREFRAME;

	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_wireRasterizer.GetAddressOf())))
	{
		return false;
	}


	rasterDesc.DepthBias = 5;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 4.0f;
	//rasterDesc.CullMode = D3D11_CULL_FRONT;
	rasterDesc.FillMode = D3D11_FILL_SOLID;

	if (FAILED(m_device->CreateRasterizerState(&rasterDesc, m_shadowMapRasterizer.GetAddressOf())))
	{
		return false;
	}

	m_deviceContext->RSSetState(m_sceneRasterizer.Get());

	ZeroMemory(&m_viewport, sizeof(D3D11_VIEWPORT));
	m_viewport.Width = m_clientWidth;
	m_viewport.Height = m_clientHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	m_deviceContext->RSSetViewports(1, &m_viewport);
	float fov = (float)M_PI_4_F;
	float aspect = m_clientWidth / m_clientHeight;

	m_projection = XMMatrixPerspectiveFovLH(fov, aspect, 0.1f, 10000.0f);
	m_ortho = XMMatrixOrthographicLH((float)vars.width, (float)vars.height, 0.1f, 10000.0f);

	m_world = XMMatrixIdentity();

	return true;
}



bool GraphicsManager::initShaders()
{
	/*auto status = RegisterShaders(device.Get(), &shaders);
	if (status != )
	{
		return GraphicsManagerStatus::D3D_SHADER_INIT_ERROR;
	}

	return GraphicsManagerStatus::OK;*/

	putShader(L"World", new Shader(
		m_device.Get(),
		L"WorldVertex.cso",
		L"WorldPixel.cso",
		sizeof(MeshLightBuffer)
	));

	putShader(L"SpotLight", new Shader(
		m_device.Get(),
		L"SpotLightShadowMapVertex.cso",
		L"SpotLightShadowMapPixel.cso",
		sizeof(LightSpaceBuffer)
	));

	putShader(L"Sprite", new Shader(
		m_device.Get(),
		L"SpriteVertex.cso",
		L"SpritePixel.cso",
		sizeof(SpriteBuffer)
	));

	putShader(L"Octree", new Shader(
		m_device.Get(),
		L"OctreeVertex.cso",
		L"OctreePixel.cso",
		sizeof(WireframeDebugColor)
	));

	putShader(L"DepthBlur", new Shader(
		m_device.Get(),
		L"PostProcessVertex.cso",
		//L"SpotLightShadowMapVertex.cso",
		L"GaussianPixel.cso",
		sizeof(OrthoView)
	));
	
	return true;
}
bool GraphicsManager::initGlobalBuffer()
{
	D3D11_BUFFER_DESC matrixBufferDesc;
	ZeroMemory(&matrixBufferDesc, sizeof(D3D11_BUFFER_DESC));
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(GlobalBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	HRESULT result = m_device->CreateBuffer(&matrixBufferDesc, NULL, m_gBufferBuffer.GetAddressOf());
	if (FAILED(result))
	{
		return false;
	}

	ZeroMemory(&m_gBuffer, sizeof(GlobalBuffer));
	return true;
}

bool GraphicsManager::initQuad()
{
	GVertex quadVertexData[4] = {
		GVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f),
		GVertex(4096, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f),
		GVertex(4096, 4096, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f),
		GVertex(0.0f, 4096, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f)
	};

	D3D11_BUFFER_DESC vertexDesc = {0};
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.ByteWidth = (UINT)(4 * sizeof(GVertex));
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.CPUAccessFlags = 0;
	vertexDesc.MiscFlags = 0;
	vertexDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData = { 0 };
	vertexData.pSysMem = quadVertexData;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;
	
	HRESULT result = Graphics.getDevice()->CreateBuffer(&vertexDesc, &vertexData, m_quadVertexBuffer.GetAddressOf());
	if (FAILED(result))
	{
		logger.err("Failed to create vertex buffer!");
		return false;
	}

	u32 indices[6] = {
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC indexDesc = { 0 };
	indexDesc.Usage = D3D11_USAGE_DEFAULT;
	indexDesc.ByteWidth = (u32)(sizeof(u32) * 6);
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = 0;
	indexDesc.MiscFlags = 0;
	indexDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData = { 0 };
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = Graphics.getDevice()->CreateBuffer(&indexDesc, &indexData, m_quadIndexBuffer.GetAddressOf());
	if (FAILED(result))
	{
		logger.err("Failed to create index buffer!");
		return false;
	}
	return true;
}


inline void GraphicsManager::inverseTranspose(const XMMATRIX& world, XMMATRIX& invWorld)
{
	CopyMemory(&invWorld, &world, sizeof(XMMATRIX));
	invWorld.r[3] = XMVectorSet(0, 0, 0, 1.0f);
	XMVECTOR determinant = XMMatrixDeterminant(invWorld);
	invWorld = XMMatrixTranspose(XMMatrixInverse(&determinant, invWorld));
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		System.setRunning(false);
		return 0;
	default:
		return Graphics.messageHandler(hWnd, uMsg, wParam, lParam);
	}
}
