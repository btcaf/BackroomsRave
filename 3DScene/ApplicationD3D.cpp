/*
* The module implementing the sample D3D12 application for JNP3 course at MIMUW.
*
* It initializes Direct3D 12 library and all the relevant components,
* for displaying a real time animation of a textured triangle.
*
* Checking and appropriate response to failed HRESULT
* function calls not implemented!
*
* Author:   Przemysaw Rutka
* Version:  1.0
* Date:     08.01.2024
*/

#include <cassert>
#include <utility>
#include <array>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "ApplicationD3D.h"
#include "Camera.h"
#include "util.h"
#include "Scene.h"
#include "SceneConfig.h"
#include "types.h"

#ifndef NDEBUG
// Activates D3D12 debug layer in the InitDirect3D function
#define DX12_DEBUG
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace {
#include "vertex_shader.h"
#include "pixel_shader.h"

    using ::std::array;

    using ::Microsoft::WRL::ComPtr;

    using DirectX::XMFLOAT4X4;
    using DirectX::XMFLOAT4;
    using DirectX::XMStoreFloat4x4;
    using DirectX::XMMatrixIdentity;
    using DirectX::XMMATRIX;
    using DirectX::XMMatrixRotationZ;
    using DirectX::XMMatrixRotationY;
    using DirectX::XMMatrixMultiply;
    using DirectX::XMMatrixTranslation;
    using DirectX::XMMatrixPerspectiveFovLH;

    // Minimum feature level of Direct3D 12
    constexpr D3D_FEATURE_LEVEL MIN_FEATURE_LEVEL = D3D_FEATURE_LEVEL_12_0;
    // Use D3D_FEATURE_LEVEL_11_1 whenever D3D_FEATURE_LEVEL_12_0 does not work

    // Timer settings
    constexpr UINT_PTR ID_TIMER = 1001;
    constexpr UINT INTERVAL = 15;

    // Animation time change settings
    //constexpr FLOAT TIME_STEP = 1.0f / 128.0f;
    //FLOAT angle = 0.0f;

    // Components used to run the Direct3D 12
    constexpr UINT FB_COUNT = 2;    // the number of frame buffers
    SIZE_T back_buffer_idx;         // current back buffer index

    ComPtr<ID3D12Device> d3d12_device = nullptr;
    ComPtr<IDXGIFactory7> dxgi_factory = nullptr;
    ComPtr<ID3D12CommandQueue> cmd_queue = nullptr;
    ComPtr<IDXGISwapChain3> swap_chain = nullptr;
    ComPtr<ID3D12DescriptorHeap> rtv_heap = nullptr; // render target view heap
    ComPtr<ID3D12Resource> frame_buffers[FB_COUNT] = {};
    ComPtr<ID3D12CommandAllocator> cmd_allocators[FB_COUNT] = {};
    ComPtr<ID3D12GraphicsCommandList> cmd_list = nullptr;
    ComPtr<ID3D12RootSignature> root_signature = nullptr;

    D3D12_VIEWPORT viewport = {
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width = 0.0f,
        .Height = 0.0f,
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f
    };

    D3D12_RECT scissor_rect = {};

    D3D12_RESOURCE_BARRIER resource_barrier = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition = {
            .pResource = nullptr,
            .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
            .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
            .StateAfter = D3D12_RESOURCE_STATE_PRESENT
        },
    };

    // Depth buffer
    ComPtr<ID3D12DescriptorHeap> dsv_heap = nullptr;
    ComPtr<ID3D12Resource> depth_buffer = nullptr;

    // Graphic pipeline state object
    ComPtr<ID3D12PipelineState> pipeline_state = nullptr;

    // Vertex buffer
    ComPtr<ID3D12Resource> vertex_buffer = nullptr;
    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view = {};

    // Descriptor heap for cbv and srv
    ComPtr<ID3D12DescriptorHeap> descriptor_heap = nullptr;

    // Constant buffer for vertex shader
    ComPtr<ID3D12Resource> vs_const_buffer = nullptr;
    constexpr UINT CONST_BUFFER_ALIGN = 256;
    /*struct vs_const_buffer_t { // must be a multiplicity of 256 bytes
        XMFLOAT4X4 matWorldViewProj;
        XMFLOAT4 padding[(CONST_BUFFER_ALIGN
            - sizeof(XMFLOAT4X4)) / sizeof(XMFLOAT4)];
    };*/
    // for lighting
    struct vs_const_buffer_t {
        XMFLOAT4X4 matWorldViewProj;
        XMFLOAT4X4 matWorldView;
        XMFLOAT4X4 matView;
        XMFLOAT4 colMaterial;
        XMFLOAT4 colLight;
        XMFLOAT4 pointLight;
        XMFLOAT4 ambientLight;
        XMFLOAT4 padding[(CONST_BUFFER_ALIGN
            			- 3 * sizeof(XMFLOAT4X4) - 4 * sizeof(XMFLOAT4)) / sizeof(XMFLOAT4)];
    };
    constexpr size_t VS_CONST_BUFFER_SIZE = sizeof(vs_const_buffer_t);
    vs_const_buffer_t vs_const_buffer_cpu_data;
    UINT* vs_const_buffer_data = nullptr;

    // CPU - GPU synchronization
    ComPtr<ID3D12Fence> sync_fence = nullptr;
    HANDLE fence_event;
    UINT64 fence_values[FB_COUNT] = { 0, 0 };

    // Camera
    Camera camera;

    // Geometric data
    SceneConfig scene_config;
    Scene scene = Scene(scene_config);
    auto vertices = scene.get_vertices();


    // Texture resource
    ComPtr<ID3D12Resource> texture_resource = nullptr;

    // Color constants
    constinit FLOAT const clear_color[] = { 0.875f, 0.875f, 0.875f, 1.0f };

    // Helper functions

    /*
     * Creates the root signature that links constant buffer
     * with vertex shader and (texture) shader resource
     * with pixel shader
     */
    void InitRootSignature() {
        D3D12_DESCRIPTOR_RANGE descriptor_ranges[] = {
            {
                .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
                .NumDescriptors = 1,
                .BaseShaderRegister = 0,
                .RegisterSpace = 0,
                .OffsetInDescriptorsFromTableStart =
                     D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
            },
            {
                .RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                .NumDescriptors = 1,
                .BaseShaderRegister = 0,
                .RegisterSpace = 0,
                .OffsetInDescriptorsFromTableStart =
                    D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
            }
        };
        D3D12_ROOT_PARAMETER root_params[] = {
            {
                .ParameterType =
                     D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { 1, &descriptor_ranges[0]},
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX
            },
            {
                .ParameterType =
                     D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
                .DescriptorTable = { 1, &descriptor_ranges[1]},
                .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
            }
        };
        D3D12_STATIC_SAMPLER_DESC tex_sampler_desc = {
            .Filter = D3D12_FILTER_MIN_MAG_MIP_POINT,
            //D3D12_FILTER_MIN_MAG_MIP_LINEAR
        .AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP,
        .MipLODBias = 0,
        .MaxAnisotropy = 0,
        .ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER,
        .BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
        .MinLOD = 0.0f,
        .MaxLOD = D3D12_FLOAT32_MAX,
        .ShaderRegister = 0,
        .RegisterSpace = 0,
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL
        };

        D3D12_ROOT_SIGNATURE_DESC root_signature_desc = {
            .NumParameters = _countof(root_params),
            .pParameters = root_params,
            .NumStaticSamplers = 1,
            .pStaticSamplers = &tex_sampler_desc,
            .Flags
                = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
                  | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
                  | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
                  | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
        };
        ComPtr<ID3DBlob> signature, error;
        D3D12SerializeRootSignature(
            &root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1,
            &signature, &error);
        d3d12_device->CreateRootSignature(
            0, signature->GetBufferPointer(),
            signature->GetBufferSize(), IID_PPV_ARGS(&root_signature));
    }

    /*
     * Initializes depth buffer for depth test
     */
    void InitDepthBuffer() {
        // Depth stencil descriptor heap settings
        D3D12_DESCRIPTOR_HEAP_DESC const dsv_heap_desc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .NumDescriptors = 1,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
            .NodeMask = 0
        };

        // Create depth stencil view descriptor heap.
        d3d12_device->CreateDescriptorHeap(
            &dsv_heap_desc, IID_PPV_ARGS(&dsv_heap));

        // Create depth buffer resource
        D3D12_HEAP_PROPERTIES dsv_heap_prop = {
            .Type = D3D12_HEAP_TYPE_DEFAULT,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 1,
            .VisibleNodeMask = 1
        };
        D3D12_RESOURCE_DESC dsv_resource_desc = {
           .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
           .Alignment = 0,
           .Width = static_cast<UINT64>(viewport.Width),
           .Height = static_cast<UINT>(viewport.Height),
           .DepthOrArraySize = 1,
           .MipLevels = 0,
           .Format = DXGI_FORMAT_D32_FLOAT,
           .SampleDesc = {.Count = 1, .Quality = 0 },
           .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
           .Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
        };
        D3D12_CLEAR_VALUE depth_clear_value = {
            .Format = DXGI_FORMAT_D32_FLOAT,
            .DepthStencil = {.Depth = 1.0f, .Stencil = 0 }
        };
        d3d12_device->CreateCommittedResource(
            &dsv_heap_prop,
            D3D12_HEAP_FLAG_NONE,
            &dsv_resource_desc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &depth_clear_value,
            IID_PPV_ARGS(&depth_buffer)
        );

        // Create depth buffer view
        D3D12_DEPTH_STENCIL_VIEW_DESC const depth_buffer_view = {
            .Format = DXGI_FORMAT_D32_FLOAT,
            .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D,
            .Flags = D3D12_DSV_FLAG_NONE,
            .Texture2D = {}
        };
        d3d12_device->CreateDepthStencilView(
            depth_buffer.Get(),
            &depth_buffer_view,
            dsv_heap->GetCPUDescriptorHandleForHeapStart()
        );
    }


    /*
     * Creates a pipeline state object (PSO) for the application.
     */
    void InitPipelineState() {
        // Color blending state settings
        D3D12_BLEND_DESC blend_desc = {
            .AlphaToCoverageEnable = FALSE,
            .IndependentBlendEnable = FALSE,
            .RenderTarget = {
                {
                    .BlendEnable = FALSE,
                    .LogicOpEnable = FALSE,
                    .SrcBlend = D3D12_BLEND_ONE,
                    .DestBlend = D3D12_BLEND_ZERO,
                    .BlendOp = D3D12_BLEND_OP_ADD,
                    .SrcBlendAlpha = D3D12_BLEND_ONE,
                    .DestBlendAlpha = D3D12_BLEND_ZERO,
                    .BlendOpAlpha = D3D12_BLEND_OP_ADD,
                    .LogicOp = D3D12_LOGIC_OP_NOOP,
                    .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
                }
            }
        };

        // Rasterizer state settings
        D3D12_RASTERIZER_DESC rasterizer_desc = {
            .FillMode = D3D12_FILL_MODE_SOLID,  // D3D12_FILL_MODE_WIREFRAME
            .CullMode = D3D12_CULL_MODE_BACK,   // D3D12_CULL_MODE_NONE
            .FrontCounterClockwise = FALSE,
            .DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
            .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
            .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
            .DepthClipEnable = TRUE,
            .MultisampleEnable = FALSE,
            .AntialiasedLineEnable = FALSE,
            .ForcedSampleCount = 0,
            .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
        };

        // Depth and stencil tests state settings
        D3D12_DEPTH_STENCIL_DESC depth_desc = {
            .DepthEnable = TRUE,
            .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
            .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
            .StencilEnable = FALSE,
            .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
            .StencilWriteMask = D3D12_DEFAULT_STENCIL_READ_MASK,
            .FrontFace = {
                .StencilFailOp = D3D12_STENCIL_OP_KEEP,
                .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
                .StencilPassOp = D3D12_STENCIL_OP_KEEP,
                .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS
             },
            .BackFace = {
                .StencilFailOp = D3D12_STENCIL_OP_KEEP,
                .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
                .StencilPassOp = D3D12_STENCIL_OP_KEEP,
                .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS
             }
        };


        // Vertex input layout settings
        D3D12_INPUT_ELEMENT_DESC input_elements_desc[] = {
            {
                .SemanticName = "POSITION",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0
            },
            {
                .SemanticName = "COLOR",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0
            },
            {
                .SemanticName = "TEXCOORD",
                .SemanticIndex = 0,
                .Format = DXGI_FORMAT_R32G32_FLOAT,
                .InputSlot = 0,
                .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                .InstanceDataStepRate = 0
            },
            {
				.SemanticName = "NORMAL",
				.SemanticIndex = 0,
				.Format = DXGI_FORMAT_R32G32B32_FLOAT,
				.InputSlot = 0,
				.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
				.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
				.InstanceDataStepRate = 0
			}
        };

        // Graphic pipeline state settings
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_state_desc = {
            .pRootSignature = root_signature.Get(),
            .VS = { vs_main, sizeof(vs_main) },  // vertex shader bytecode
            .PS = { ps_main, sizeof(ps_main) },  // pixel shader bytecode
            .DS = {},
            .HS = {},
            .GS = {},
            .StreamOutput = {},
            .BlendState = blend_desc,
            .SampleMask = UINT_MAX,
            .RasterizerState = rasterizer_desc,
            .DepthStencilState = depth_desc,
            .InputLayout
                = { input_elements_desc, _countof(input_elements_desc)},
            .IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
            .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
            .NumRenderTargets = 1,
            .RTVFormats = { DXGI_FORMAT_R8G8B8A8_UNORM },
            .DSVFormat = DXGI_FORMAT_D32_FLOAT,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .NodeMask = 0,
            .CachedPSO = {},
            .Flags = D3D12_PIPELINE_STATE_FLAG_NONE
        };

        d3d12_device->CreateGraphicsPipelineState(
            &pipeline_state_desc, IID_PPV_ARGS(&pipeline_state));
    }

    /*
     * Creates a Direct3D 12 command list
     */
    void InitCommandList() {
        d3d12_device->CreateCommandList(
            0, D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmd_allocators[back_buffer_idx].Get(),
            nullptr, IID_PPV_ARGS(&cmd_list));
        cmd_list->Close();
    }

    /*
     * Creates a vertex buffer and fills it with geometry data.
     */
    void BuildVertexBuffer() {
        // Create vertex buffer resource
        D3D12_HEAP_PROPERTIES heap_prop = {
            .Type = D3D12_HEAP_TYPE_UPLOAD,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 1,
            .VisibleNodeMask = 1
        };
        D3D12_RESOURCE_DESC resource_desc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = vertices.size() * sizeof(vertex_t),
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE
        };
        d3d12_device->CreateCommittedResource(
            &heap_prop, D3D12_HEAP_FLAG_NONE,
            &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr, IID_PPV_ARGS(&vertex_buffer));

        // Copy geometry to the vertex buffer
        UINT8* vertex_data = nullptr;
        D3D12_RANGE read_range = { 0, 0 };
        vertex_buffer->Map(
            0, &read_range, reinterpret_cast<void**>(&vertex_data));
        memcpy(vertex_data, vertices.data(), vertices.size() * sizeof(vertex_t));
        vertex_buffer->Unmap(0, nullptr);

        // Initialize a vertex buffer view
        vertex_buffer_view.BufferLocation
            = vertex_buffer->GetGPUVirtualAddress();
        vertex_buffer_view.SizeInBytes = vertices.size() * sizeof(vertex_t);
        vertex_buffer_view.StrideInBytes = sizeof(vertex_t);
    }

    /*
     * Creates a descriptor heap for constant buffer (vertex shader)
     * and texture shader resource (pixel shader).
     */
    void BuildDescriptorHeap() {
        D3D12_DESCRIPTOR_HEAP_DESC cbv_heap_desc = {
            .Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .NumDescriptors = 2,
            .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
            .NodeMask = 0
        };
        d3d12_device->CreateDescriptorHeap(
            &cbv_heap_desc, IID_PPV_ARGS(&descriptor_heap));
    }

    /*
     * Creates a constant buffer for vertex shader and fills it
     * with some initial data (identity matrix).
     */
    void BuildVSConstBuffer() {
        // Create constant buffer resource
        D3D12_HEAP_PROPERTIES heap_prop = {
            .Type = D3D12_HEAP_TYPE_UPLOAD,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 1,
            .VisibleNodeMask = 1
        };
        D3D12_RESOURCE_DESC resource_desc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
            .Alignment = 0,
            .Width = VS_CONST_BUFFER_SIZE,
            .Height = 1,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_UNKNOWN,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
            .Flags = D3D12_RESOURCE_FLAG_NONE
        };
        d3d12_device->CreateCommittedResource(
            &heap_prop, D3D12_HEAP_FLAG_NONE,
            &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr, IID_PPV_ARGS(&vs_const_buffer));

        // Create a constant buffer view
        D3D12_CONSTANT_BUFFER_VIEW_DESC vs_const_buffer_view = {
            .BufferLocation = vs_const_buffer->GetGPUVirtualAddress(),
            .SizeInBytes = VS_CONST_BUFFER_SIZE
        };
        d3d12_device->CreateConstantBufferView(
            &vs_const_buffer_view,
            descriptor_heap->GetCPUDescriptorHandleForHeapStart());

        // Map and initialize the constant buffer.
        // Do not unmap this until the appplication closes.
        D3D12_RANGE read_range = { 0, 0 };
        vs_const_buffer->Map(
            0, &read_range, reinterpret_cast<void**>(&vs_const_buffer_data));
        XMStoreFloat4x4(
            &vs_const_buffer_cpu_data.matWorldViewProj, XMMatrixIdentity());
        XMStoreFloat4x4(
			&vs_const_buffer_cpu_data.matWorldView, XMMatrixIdentity());
        XMStoreFloat4x4(
            &vs_const_buffer_cpu_data.matView, XMMatrixIdentity());
        vs_const_buffer_cpu_data.colMaterial = { 0.0f, 0.0f, 0.0f, 1.0f };
        vs_const_buffer_cpu_data.colLight = { 1.0f, 1.0f, 1.0f, 1.0f };
        vs_const_buffer_cpu_data.pointLight = { 4.0f, 2.0f, 14.0f, 0.0f };
        vs_const_buffer_cpu_data.ambientLight = { 0.15f, 0.15f, 0.f, 1.0f };
        memcpy(vs_const_buffer_data, &vs_const_buffer_cpu_data,
            sizeof vs_const_buffer_cpu_data);
    }

    /*
    * Creates a CPU - GPU synchronizing object.
    */
    void InitFence() {
        d3d12_device->CreateFence(fence_values[back_buffer_idx],
            D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&sync_fence));
        ++fence_values[back_buffer_idx];

        fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        assert(fence_event != nullptr);
    }

    /*
     * Waits until GPU processor finishes rendering.
     */
    void WaitForGPU() {
        // Schedule the fence update to the specified value.
        cmd_queue->Signal(
            sync_fence.Get(), fence_values[back_buffer_idx]);

        // Specifies the event that is raised when the fence reaches
        // the specified value.
        sync_fence->SetEventOnCompletion(
            fence_values[back_buffer_idx], fence_event);
        // Wait until the fence has been processed.
        WaitForSingleObject(fence_event, INFINITE);

        // Increment the fence value for the current frame.
        ++fence_values[back_buffer_idx];
    }

    /*
     * Builds and initializes texture resource for GPU.
     * Creates its view.
     *
     * MUST BE CALLED AFTER CreateCommandList (without Close),
     * InitFence AND InitPipelineStates.
     */
    void BuildTextureResource() {
        hr_check(CoInitializeEx(nullptr, COINIT_MULTITHREADED));
        UINT const bmp_px_size = 4;
        UINT bmp_width = 0;
        UINT bmp_height = 0;
        BYTE* bmp_bits = LoadBitmapFromFile(TEXT("assets/backrooms.png"), bmp_width, bmp_height);

        // Texture resource
        D3D12_HEAP_PROPERTIES tex_heap_prop = {
            .Type = D3D12_HEAP_TYPE_DEFAULT,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 1,
            .VisibleNodeMask = 1
        };
        D3D12_RESOURCE_DESC tex_resource_desc = {
            .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
            .Alignment = 0,
            .Width = bmp_width,
            .Height = bmp_height,
            .DepthOrArraySize = 1,
            .MipLevels = 1,
            .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
            .SampleDesc = {.Count = 1, .Quality = 0 },
            .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
            .Flags = D3D12_RESOURCE_FLAG_NONE
        };
        d3d12_device->CreateCommittedResource(
            &tex_heap_prop, D3D12_HEAP_FLAG_NONE,
            &tex_resource_desc, D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr, IID_PPV_ARGS(&texture_resource));

        // Helper buffer for reading texture to GPU
        ComPtr<ID3D12Resource> texture_upload_buffer = nullptr;

        UINT64 RequiredSize = 0;
        auto Desc = texture_resource.Get()->GetDesc();
        ID3D12Device* pDevice = nullptr;
        texture_resource.Get()->GetDevice(
            __uuidof(*pDevice), reinterpret_cast<void**>(&pDevice)
        );
        pDevice->GetCopyableFootprints(
            &Desc, 0, 1, 0, nullptr, nullptr, nullptr, &RequiredSize);
        pDevice->Release();

        D3D12_HEAP_PROPERTIES tex_upload_heap_prop = {
            .Type = D3D12_HEAP_TYPE_UPLOAD,
            .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
            .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
            .CreationNodeMask = 1,
            .VisibleNodeMask = 1
        };
        D3D12_RESOURCE_DESC tex_upload_resource_desc = {
           .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
           .Alignment = 0,
           .Width = RequiredSize,
           .Height = 1,
           .DepthOrArraySize = 1,
           .MipLevels = 1,
           .Format = DXGI_FORMAT_UNKNOWN,
           .SampleDesc = {.Count = 1, .Quality = 0 },
           .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
           .Flags = D3D12_RESOURCE_FLAG_NONE
        };
        d3d12_device->CreateCommittedResource(
            &tex_upload_heap_prop, D3D12_HEAP_FLAG_NONE,
            &tex_upload_resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr, IID_PPV_ARGS(&texture_upload_buffer));

        // Copy texture surface data to the helper buffer
        D3D12_SUBRESOURCE_DATA texture_data = {
            .pData = bmp_bits,
            .RowPitch = bmp_width * bmp_px_size,
            .SlicePitch = bmp_width * bmp_height * bmp_px_size
        };

        cmd_list->Reset(cmd_allocators[back_buffer_idx].Get(),
            pipeline_state.Get());

        UINT const MAX_SUBRESOURCES = 1;
        RequiredSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layouts[MAX_SUBRESOURCES];
        UINT NumRows[MAX_SUBRESOURCES];
        UINT64 RowSizesInBytes[MAX_SUBRESOURCES];
        Desc = texture_resource.Get()->GetDesc();
        pDevice = nullptr;
        texture_resource.Get()->GetDevice(
            __uuidof(*pDevice), reinterpret_cast<void**>(&pDevice)
        );
        pDevice->GetCopyableFootprints(
            &Desc, 0, 1, 0, Layouts, NumRows, RowSizesInBytes, &RequiredSize);
        pDevice->Release();

        UINT8* map_tex_data = nullptr;
        texture_upload_buffer->Map(0, nullptr,
            reinterpret_cast<void**>(&map_tex_data));

        D3D12_MEMCPY_DEST DestData = {
            .pData = map_tex_data + Layouts[0].Offset,
            .RowPitch = Layouts[0].Footprint.RowPitch,
            .SlicePitch = SIZE_T(Layouts[0].Footprint.RowPitch)
                                * SIZE_T(NumRows[0])
        };
        for (UINT z = 0; z < Layouts[0].Footprint.Depth; ++z) {
            auto pDestSlice = static_cast<UINT8*>(DestData.pData)
                + DestData.SlicePitch * z;
            auto pSrcSlice = static_cast<const UINT8*>(texture_data.pData)
                + texture_data.SlicePitch * LONG_PTR(z);
            for (UINT y = 0; y < NumRows[0]; ++y) {
                memcpy(pDestSlice + DestData.RowPitch * y,
                    pSrcSlice + texture_data.RowPitch * LONG_PTR(y),
                    static_cast<SIZE_T>(RowSizesInBytes[0]));
            }
        }

        texture_upload_buffer->Unmap(0, nullptr);

        // Ask GPU to copy texture data from the helper buffer
        // to the texture resource
        D3D12_TEXTURE_COPY_LOCATION Dst = {
            .pResource = texture_resource.Get(),
            .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
            .SubresourceIndex = 0
        };
        D3D12_TEXTURE_COPY_LOCATION Src = {
            .pResource = texture_upload_buffer.Get(),
            .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
            .PlacedFootprint = Layouts[0]
        };
        cmd_list->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);

        D3D12_RESOURCE_BARRIER tex_upload_resource_barrier = {
            .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
            .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
            .Transition = {
                .pResource = texture_resource.Get(),
                .Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                .StateBefore = D3D12_RESOURCE_STATE_COPY_DEST,
                .StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
        };
        cmd_list->ResourceBarrier(1, &tex_upload_resource_barrier);

        cmd_list->Close();
        ID3D12CommandList* tmp_cmd_list = cmd_list.Get();
        cmd_queue->ExecuteCommandLists(1, &tmp_cmd_list);

        // Shader resource view for texture
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {
            .Format = tex_resource_desc.Format,
            .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
            .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
            .Texture2D = {
                .MostDetailedMip = 0,
                .MipLevels = 1,
                .PlaneSlice = 0,
                .ResourceMinLODClamp = 0.0f
            },
        };
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle =
            descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        cpu_desc_handle.ptr += d3d12_device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        d3d12_device->CreateShaderResourceView(
            texture_resource.Get(), &srv_desc, cpu_desc_handle);

        WaitForGPU();   // MUST NOT DELETE texture_upload_buffer
        // BEFORE COPYING IS FINISHED
    }

    /*
     * Prepares for rendering the next animation frame,
     * keeping CPU - GPU synchronized.
     */
    void PrepareNextFrame() {
        const UINT64 current_fence_value = fence_values[back_buffer_idx];
        cmd_queue->Signal(sync_fence.Get(), current_fence_value);

        // Update the frame index.
        back_buffer_idx = swap_chain->GetCurrentBackBufferIndex();

        // If the next frame is not ready to be rendered yet,
        // wait until it is ready.
        if (sync_fence->GetCompletedValue() < fence_values[back_buffer_idx]) {
            sync_fence->SetEventOnCompletion(
                fence_values[back_buffer_idx], fence_event);
            WaitForSingleObject(fence_event, INFINITE);
        }

        // Set the fence value for the next frame.
        fence_values[back_buffer_idx] = current_fence_value + 1;
    }

    /*
     * Initializes all program's graphic resources.
     */
    void InitGraphicsResources() {
        InitRootSignature();
        InitDepthBuffer();
        InitPipelineState();
        InitCommandList();
        BuildVertexBuffer();
        BuildDescriptorHeap();
        BuildVSConstBuffer();
        InitFence();
        BuildTextureResource();
        WaitForGPU();
    }

    /*
     * Prepares a list of commands to be executed.
     */
    void PrepareCommandList() {
        cmd_allocators[back_buffer_idx]->Reset();

        cmd_list->Reset(
            cmd_allocators[back_buffer_idx].Get(),
            pipeline_state.Get());

        cmd_list->SetGraphicsRootSignature(root_signature.Get());

        ID3D12DescriptorHeap* ppHeaps[] = { descriptor_heap.Get() };
        cmd_list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle =
            descriptor_heap->GetGPUDescriptorHandleForHeapStart();
        cmd_list->SetGraphicsRootDescriptorTable(
            0, gpu_desc_handle);
        gpu_desc_handle.ptr +=
            d3d12_device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        cmd_list->SetGraphicsRootDescriptorTable(
            1, gpu_desc_handle);

        cmd_list->RSSetViewports(1, &viewport);
        cmd_list->RSSetScissorRects(1, &scissor_rect);

        D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle =
            rtv_heap->GetCPUDescriptorHandleForHeapStart();
        cpu_desc_handle.ptr += back_buffer_idx *
            d3d12_device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Use the back buffer as a render target.
        resource_barrier.Transition.pResource
            = frame_buffers[back_buffer_idx].Get();
        resource_barrier.Transition.StateBefore
            = D3D12_RESOURCE_STATE_PRESENT;
        resource_barrier.Transition.StateAfter
            = D3D12_RESOURCE_STATE_RENDER_TARGET;
        cmd_list->ResourceBarrier(1, &resource_barrier);

        D3D12_CPU_DESCRIPTOR_HANDLE cpu_depth_desc_handle
            = dsv_heap->GetCPUDescriptorHandleForHeapStart();

        cmd_list->OMSetRenderTargets(1, &cpu_desc_handle, FALSE, &cpu_depth_desc_handle);

        cmd_list->ClearRenderTargetView(
            cpu_desc_handle, clear_color, 0, nullptr
        );
        cmd_list->ClearDepthStencilView(
            cpu_depth_desc_handle,
            D3D12_CLEAR_FLAG_DEPTH,
            1.0, 0, 0, nullptr
        );

        cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmd_list->IASetVertexBuffers(0, 1, &vertex_buffer_view);
        cmd_list->DrawInstanced(vertices.size(), 1, 0, 0);

        // Use the back buffer to be present.
        resource_barrier.Transition.StateBefore
            = D3D12_RESOURCE_STATE_RENDER_TARGET;
        resource_barrier.Transition.StateAfter
            = D3D12_RESOURCE_STATE_PRESENT;
        cmd_list->ResourceBarrier(1, &resource_barrier);

        cmd_list->Close();
    }

    /*
     * Renders a single animation frame
     */
    void RenderFrame() {
        PrepareCommandList();

        // Execute command list
        ID3D12CommandList* tmp_cmd_list = cmd_list.Get();
        cmd_queue->ExecuteCommandLists(1, &tmp_cmd_list);

        // Present the frame buffer
        swap_chain->Present(1, 0);

        // Synchronize with the previous frame
        PrepareNextFrame();
    }
};


void InitDirect3D(HWND hwnd) {
    UINT dxgi_factory_flag = 0;

#ifdef DX12_DEBUG
    // Activate the debug layer of Direct3D 12
    ComPtr<ID3D12Debug> debug_controller = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)))) {
        debug_controller->EnableDebugLayer();
        dxgi_factory_flag |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif /* DX12_DEBUG */

    // Create a Direct3D 12 device.
    D3D12CreateDevice(
        nullptr, MIN_FEATURE_LEVEL, IID_PPV_ARGS(&d3d12_device));

    // Create a DirectX Graphics Infrastructure factory.
    CreateDXGIFactory2(
        dxgi_factory_flag, IID_PPV_ARGS(&dxgi_factory));

    // Command queue settings.
    D3D12_COMMAND_QUEUE_DESC const cq_desc = {
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Priority = 0,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
        .NodeMask = 0
    };

    // Create a command queue.
    d3d12_device->CreateCommandQueue(
        &cq_desc, IID_PPV_ARGS(&cmd_queue));

    // Swap chain settings
    DXGI_SWAP_CHAIN_DESC1 const sc_desc = {
      .Width = 0,
      .Height = 0,
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .Stereo = false,
      .SampleDesc = {.Count = 1, .Quality = 0},
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      .BufferCount = FB_COUNT,
      .Scaling = DXGI_SCALING_STRETCH,
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
      .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
      .Flags = 0
    };

    // Create a swap chain associated with a window handle (HWND).
    ComPtr<IDXGISwapChain1> swap_chain1;
    dxgi_factory->CreateSwapChainForHwnd(cmd_queue.Get(),
        hwnd, &sc_desc, nullptr, nullptr, &swap_chain1);
    swap_chain1.As(&swap_chain);

    // Get first back buffer index.
    back_buffer_idx = swap_chain->GetCurrentBackBufferIndex();

    // Update scissor rect and viewport.
    GetClientRect(hwnd, &scissor_rect);
    viewport.Width = static_cast<FLOAT>(scissor_rect.right)
        - static_cast<FLOAT>(scissor_rect.left);
    viewport.Height = static_cast<FLOAT>(scissor_rect.bottom)
        - static_cast<FLOAT>(scissor_rect.top);

    // A render target descriptor heap settings
    D3D12_DESCRIPTOR_HEAP_DESC const rtv_heap_desc = {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        .NumDescriptors = FB_COUNT,
        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        .NodeMask = 0
    };

    // Create a render target view descriptor heap.
    d3d12_device->CreateDescriptorHeap(
        &rtv_heap_desc, IID_PPV_ARGS(&rtv_heap));

    // Create render target views and command allocators
    // for each frame buffer
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle
        = rtv_heap->GetCPUDescriptorHandleForHeapStart();
    for (UINT k = 0; k < FB_COUNT; ++k) {
        cpu_desc_handle.ptr += static_cast<SIZE_T>(k) *
            d3d12_device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        swap_chain->GetBuffer(k, IID_PPV_ARGS(&frame_buffers[k]));
        d3d12_device->CreateRenderTargetView(
            frame_buffers[k].Get(), nullptr, cpu_desc_handle);
        d3d12_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmd_allocators[k]));
    }

    InitGraphicsResources();
}

void InitTimer(HWND hwnd) {
    SetTimer(hwnd, ID_TIMER, INTERVAL, nullptr);
}

void OnTimer() {
    // Change animation time.
    camera.update();
    

    // Compute transformation matrices.
    XMMATRIX wvp_matrix;

    XMMATRIX vp_matrix = XMMatrixMultiply(
        camera.get_view_matrix(),                                 // View
        XMMatrixPerspectiveFovLH(                                 // Projection
            45.0f, viewport.Width / viewport.Height, 0.5f, 50.0f)
    );
    wvp_matrix = XMMatrixMultiply(
        XMMatrixIdentity(),                                       // World
        vp_matrix
    );
    wvp_matrix = XMMatrixTranspose(wvp_matrix);

    // Update the constant buffer of vertex shader.
    XMStoreFloat4x4(&vs_const_buffer_cpu_data.matWorldViewProj, wvp_matrix);
    XMStoreFloat4x4(&vs_const_buffer_cpu_data.matWorldView, XMMatrixTranspose(camera.get_view_matrix()));
    XMStoreFloat4x4(&vs_const_buffer_cpu_data.matView, XMMatrixTranspose(camera.get_view_matrix()));
    memcpy(vs_const_buffer_data, &vs_const_buffer_cpu_data,
        sizeof(vs_const_buffer_cpu_data));
}

void ReleaseTimer(HWND hwnd) {
    KillTimer(hwnd, ID_TIMER);
}

void OnPaint() {
    RenderFrame();
}

void EndDirect3D() {
    WaitForGPU();
}