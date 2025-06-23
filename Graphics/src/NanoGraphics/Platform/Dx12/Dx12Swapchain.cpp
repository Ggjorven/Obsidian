#include "ngpch.h"
#include "Dx12Swapchain.hpp"

#include "NanoGraphics/Core/Logging.hpp"
#include "NanoGraphics/Core/Information.hpp"
#include "NanoGraphics/Core/Window.hpp"
#include "NanoGraphics/Utils/Profiler.hpp"

#include "NanoGraphics/Platform/Dx12/Dx12Device.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Image.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12Resources.hpp"
#include "NanoGraphics/Platform/Dx12/Dx12CommandList.hpp"

#if defined(NG_PLATFORM_DESKTOP)
	#define GLFW_EXPOSE_NATIVE_WIN32
	#include <GLFW/glfw3.h>
	#include <GLFW/glfw3native.h>
#endif

namespace Nano::Graphics::Internal
{

    ////////////////////////////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////////////////////////////
	Dx12Swapchain::Dx12Swapchain(const Device& device, const SwapchainSpecification& specs)
		: m_Device(*api_cast<const Dx12Device*>(&device)), m_Specification(specs)
	{
		// Swapchain
		{
			DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
			swapchainDesc.BufferCount = static_cast<UINT>(Information::FramesInFlight);
			swapchainDesc.Width = specs.WindowTarget->GetSize().x;
			swapchainDesc.Height = specs.WindowTarget->GetSize().y;
			swapchainDesc.Format = FormatToDx12FormatMapping(specs.RequestedFormat).RTVFormat;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			swapchainDesc.SampleDesc.Count = 1;

			#if defined(NG_PLATFORM_DESKTOP)
				HWND hwnd = glfwGetWin32Window(static_cast<GLFWwindow*>(m_Specification.WindowTarget->GetNativeWindow()));
			#endif

			// Note: Fullscreen is currently not a thing.
			IDXGISwapChain1* tempSwapchain;
			DX_VERIFY(m_Device.GetContext().GetIDXGIFactory()->CreateSwapChainForHwnd(m_Device.GetContext().GetD3D12CommandQueue(CommandQueue::Present), hwnd, &swapchainDesc, nullptr, nullptr, &tempSwapchain));
			tempSwapchain->AddRef();

			DX_VERIFY(tempSwapchain->QueryInterface(IID_PPV_ARGS(&m_Swapchain)));
			m_Swapchain->AddRef();
			tempSwapchain->Release(); // Release the temp
		}

		// Images
		{
			for (size_t i = 0; i < m_Images.size(); i++)
			{
				new (m_Images[i].GetInternalBytes()) Image(device);

				std::string debugName = std::format("Swapchain image for: {0}", m_Specification.DebugName);
				ImageSpecification imageSpecs = ImageSpecification()
					.SetImageFormat(m_Specification.RequestedFormat)
					.SetWidthAndHeight(m_Specification.WindowTarget->GetSize().x, m_Specification.WindowTarget->GetSize().y)
					.SetImageDimension(ImageDimension::Image2D)
					.SetIsRenderTarget(true)
					.SetDebugName(debugName);
				
				Dx12Image& dxImage = *api_cast<Dx12Image*>(&m_Images[i].Get());

				ID3D12Resource* resource;
				DX_VERIFY(m_Swapchain->GetBuffer(static_cast<UINT>(i), IID_PPV_ARGS(&resource)));
				resource->AddRef();

				dxImage.SetInternalData(imageSpecs, resource);

				ImageSubresourceSpecification imageViewSpec = ImageSubresourceSpecification(0, ImageSubresourceSpecification::AllMipLevels, 0, ImageSubresourceSpecification::AllArraySlices);
				(void)dxImage.GetSubresourceView(imageViewSpec, ImageSubresourceViewUsage::RTV, ImageDimension::Image2D, m_Specification.RequestedFormat); // Note: Makes sure to already lazy initialize the image view
			}
		}

		// Synchronization objects
		{
			DX_VERIFY(m_Device.GetContext().GetD3D12Device()->CreateFence(m_CurrentFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
			m_Fence->AddRef();

			for (auto& [value, event] : m_WaitFenceValuesAndEvents)
			{
				value = m_CurrentFenceValue;
				event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			}
		}

		if constexpr (Information::Validation)
		{
			if (!m_Specification.DebugName.empty())
			{
				m_Device.GetContext().SetDebugName(m_Fence, std::format("Fence for: {0}", m_Specification.DebugName));
			}
		}
	}

	Dx12Swapchain::~Dx12Swapchain()
	{
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Destruction methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12Swapchain::FreePool(CommandListPool& pool) const
	{
		Dx12CommandListPool& dxPool = *api_cast<Dx12CommandListPool*>(&pool);
		m_Device.GetContext().Destroy([allocator = dxPool.GetD3D12CommandAllocator()]()
		{
			allocator->Release();
		});
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Methods
	////////////////////////////////////////////////////////////////////////////////////
	void Dx12Swapchain::Resize(uint32_t width, uint32_t height)
	{
		Resize(width, height, m_Specification.VSync, m_Specification.RequestedFormat, m_Specification.RequestedColourSpace);
	}

	void Dx12Swapchain::Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace)
	{
		m_Specification.VSync = vsync;
		m_Specification.RequestedFormat = colourFormat;
		m_Specification.RequestedColourSpace = colourSpace;

		// Wait for all previous frames
		{
			DX_VERIFY(m_Device.GetContext().GetD3D12CommandQueue(CommandQueue::Present)->Signal(m_Fence, ++m_CurrentFenceValue));
			if (m_Fence->GetCompletedValue() < m_CurrentFenceValue)
			{
				DX_VERIFY(m_Fence->SetEventOnCompletion(m_CurrentFenceValue, m_WaitFenceValuesAndEvents[m_CurrentFrame].second));
				WaitForSingleObject(m_WaitFenceValuesAndEvents[m_CurrentFrame].second, INFINITE);
			}
		}

		// Update swapchain
		{
			// Destroy subresources views
			for (size_t i = 0; i < m_Images.size(); i++)
				m_Device.DestroyImage(m_Images[i].Get());

			auto a = FormatToDx12FormatMapping(colourFormat).RTVFormat;
			DX_VERIFY(m_Swapchain->ResizeBuffers(static_cast<UINT>(Information::FramesInFlight), width, height, FormatToDx12FormatMapping(colourFormat).RTVFormat, 0));

			m_Device.GetContext().OutputMessages();
			
			for (size_t i = 0; i < m_Images.size(); i++)
			{
				Dx12Image& dxImage = *api_cast<Dx12Image*>(&m_Images[i].Get());

				std::string debugName = std::format("Swapchain image for: {0}", m_Specification.DebugName);
				ImageSpecification specs = ImageSpecification()
					.SetImageFormat(m_Specification.RequestedFormat)
					.SetWidthAndHeight(m_Specification.WindowTarget->GetSize().x, m_Specification.WindowTarget->GetSize().y)
					.SetImageDimension(ImageDimension::Image2D)
					.SetIsRenderTarget(true)
					.SetDebugName(debugName);

				ID3D12Resource* resource;
				DX_VERIFY(m_Swapchain->GetBuffer(i, IID_PPV_ARGS(&resource)));

				dxImage.SetInternalData(specs, resource);

				ImageSubresourceSpecification imageViewSpec = ImageSubresourceSpecification(0, ImageSubresourceSpecification::AllMipLevels, 0, ImageSubresourceSpecification::AllArraySlices);
				(void)dxImage.GetSubresourceView(imageViewSpec, ImageSubresourceViewUsage::RTV, ImageDimension::Image2D, m_Specification.RequestedFormat); // Note: Makes sure to already lazy initialize the image view
			}
		}
	}

	void Dx12Swapchain::AcquireNextImage()
	{
		NG_PROFILE("Dx12Swapchain::AcquireNextImage()");

		// Wait for this frame's previous last value
		{
			if (m_Fence->GetCompletedValue() < m_WaitFenceValuesAndEvents[m_CurrentFrame].first)
			{
				DX_VERIFY(m_Fence->SetEventOnCompletion(m_WaitFenceValuesAndEvents[m_CurrentFrame].first, m_WaitFenceValuesAndEvents[m_CurrentFrame].second));
				WaitForSingleObject(m_WaitFenceValuesAndEvents[m_CurrentFrame].second, INFINITE);
			}
		}

		m_AcquiredFrame = static_cast<uint8_t>(m_Swapchain->GetCurrentBackBufferIndex());
	}

	void Dx12Swapchain::Present()
	{
		NG_PROFILE("Dx12Swapchain::Present()");

		DX_VERIFY(m_Device.GetContext().GetD3D12CommandQueue(CommandQueue::Present)->Wait(m_Fence, m_SwapchainPresentableValues[m_CurrentFrame]));
		
		DX_VERIFY(m_Swapchain->Present(m_Specification.VSync, 0));

		if constexpr (Information::Validation)
		{
			NG_PROFILE("Dx12Swapchain::Present::Output");
			m_Device.GetContext().OutputMessages();
		}

		DX_VERIFY(m_Device.GetContext().GetD3D12CommandQueue(CommandQueue::Present)->Signal(m_Fence, m_CurrentFenceValue++));

		m_WaitFenceValuesAndEvents[m_CurrentFrame].first = m_CurrentFenceValue;
		m_CurrentFrame = (m_CurrentFrame + 1) % Information::FramesInFlight;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Internal getters
	////////////////////////////////////////////////////////////////////////////////////
	uint64_t Dx12Swapchain::GetPreviousCommandListWaitValue(const Dx12CommandList& commandList) const
	{
		NG_ASSERT(m_CommandListFenceValues.contains(&commandList), "[Dx12Swapchain] CommandList is not known in current Swapchain.");
		return m_CommandListFenceValues.at(&commandList);
	}

	uint64_t Dx12Swapchain::RetrieveCommandListWaitValue(const Dx12CommandList& commandList)
	{
		uint64_t value = ++m_CurrentFenceValue;
		m_CommandListFenceValues[&commandList] = value;
		return value;
	}

}