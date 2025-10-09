#pragma once

#include "Obsidian/Core/Information.hpp"

#include "Obsidian/Maths/Structs.hpp"

#include "Obsidian/Renderer/ResourceSpec.hpp"
#include "Obsidian/Renderer/SwapchainSpec.hpp"
#include "Obsidian/Renderer/Image.hpp"

#include "Obsidian/Platform/Dx12/Dx12.hpp"

#include <utility>

namespace Obsidian
{
	class Device;
	class Swapchain;
	class CommandListPool;
}

namespace Obsidian::Internal
{

	class Dx12Device;
	class Dx12Swapchain;
	class Dx12CommandList;

#if defined(OB_API_DX12)
	////////////////////////////////////////////////////////////////////////////////////
	// Dx12Swapchain
	////////////////////////////////////////////////////////////////////////////////////
	class Dx12Swapchain
	{
	public:
		// Constructor & Destructor
		Dx12Swapchain(const Device& device, const SwapchainSpecification& specs);
		~Dx12Swapchain();

		// Destruction methods
		void FreePool(CommandListPool& pool) const;

		// Methods
		void Resize(uint32_t width, uint32_t height); 
		void Resize(uint32_t width, uint32_t height, bool vsync, Format colourFormat, ColourSpace colourSpace);

		void AcquireNextImage();
		void Present();

		// Getters
		inline const SwapchainSpecification& GetSpecification() const { return m_Specification; }

		inline uint8_t GetCurrentFrame() const { return m_CurrentFrame; }
		inline uint8_t GetAcquiredImage() const { return m_AcquiredFrame; }

		inline Image& GetImage(uint8_t frame) { return *reinterpret_cast<Image*>(&m_Images[frame].Get()); }
		inline const Image& GetImage(uint8_t frame) const { return *reinterpret_cast<const Image*>(&m_Images[frame].Get()); }

		inline constexpr uint8_t GetImageCount() const { return static_cast<uint8_t>(m_Images.size()); }

		// Internal getters
		inline const Dx12Device& GetDx12Device() const { return m_Device; }

		inline void SetPresentableValue(uint64_t value) { m_SwapchainPresentableValues[m_CurrentFrame] = value; }
		
		uint64_t GetPreviousCommandListWaitValue(const Dx12CommandList& commandList) const;
		uint64_t RetrieveCommandListWaitValue(const Dx12CommandList& commandList);

		inline uint64_t GetCurrentFenceValue() const { return m_CurrentFenceValue; }

		inline DxPtr<IDXGISwapChain4> GetDXGISwapChain() const { return m_Swapchain; }
		inline DxPtr<ID3D12Fence> GetD3D12Fence() const { return m_Fence; }

		inline const std::array<std::pair<uint64_t, HANDLE>, Information::FramesInFlight>& GetValuesAndEvents() const { return m_WaitFenceValuesAndEvents; }

	private:
		const Dx12Device& m_Device;
		SwapchainSpecification m_Specification;

		DxPtr<IDXGISwapChain4> m_Swapchain = nullptr;

		// Note: DX12 gives the amount of images requested, so we can use FramesInFlight instead of MaxImages.
		std::array<Nano::Memory::DeferredConstruct<Image, true>, Information::FramesInFlight> m_Images = { };

		ID3D12Fence* m_Fence = nullptr;
		uint64_t m_CurrentFenceValue = 0;

		std::array<uint64_t, Information::FramesInFlight> m_SwapchainPresentableValues = { };

		std::array<std::pair<uint64_t, HANDLE>, Information::FramesInFlight> m_WaitFenceValuesAndEvents = { };
		std::unordered_map<const Dx12CommandList*, uint64_t> m_CommandListFenceValues = { };

		uint8_t m_CurrentFrame = 0;
		uint8_t m_AcquiredFrame = 0;

		friend class Dx12Device;
	};
#endif

}