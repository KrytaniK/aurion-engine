module;

#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>

module Aurion.Vulkan;

import Aurion.Services;

namespace Aurion::Vulkan
{
    RenderTarget::RenderTarget(const std::string_view& id)
        : Aurion::RenderTarget(id), m_driver(nullptr), m_buffer_index(0)
    {

    }

    RenderTarget::~RenderTarget()
    {
        m_driver->WaitIdle(); // Let all GPU work finish before destruction
    }

    void RenderTarget::Configure(const GraphicsResource::Config* properties)
    {
        switch (dynamic_cast<const ConfigBase*>(properties)->rtType)
        {
        case RenderTarget::Type::Swapchain:
            {
                m_impl = std::make_unique<RenderTargetImpl_Swapchain>(); break;
            }
        case RenderTarget::Type::Default:
            {
                m_impl = std::make_unique<RenderTargetImpl_Default>(); break;
            }
        }

        // Configure based on swapchain vs 'offline' image
        m_impl->Configure(m_driver, properties);
    }

    void RenderTarget::Attach(const IGraphicsDriver* driver)
    {
        m_driver = dynamic_cast<const Driver*>(driver);
    }

    u32 RenderTarget::SwapBuffers(const u32& frame_index)
    {
        m_impl->SwapBuffers(frame_index, m_buffer_index);
        return m_buffer_index;
    }

    u32 RenderTarget::GetWidth() const
    {
        return m_impl->GetWidth();
    }

    u32 RenderTarget::GetHeight() const
    {
        return m_impl->GetHeight();

    }

    u32 RenderTarget::GetDepth() const
    {
        return m_impl->GetDepth();
    }

    vk::MemoryRequirements RenderTarget::GetMemoryRequirements() const
    {
        return m_impl->GetMemoryRequirements();
    }

    void RenderTarget::BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory, const u32& offset,
        const u32& index) const
    {
        m_impl->BindDeviceMemory(memory, offset, index);
    }

    const vk::Image& RenderTarget::GetImage() const
    {
        return m_impl->GetImage(m_buffer_index);
    }

    const vk::raii::ImageView& RenderTarget::GetView() const
    {
        return m_impl->GetView(m_buffer_index);
    }

    vk::Format RenderTarget::GetFormat() const
    {
        return m_impl->GetFormat();
    }

    const vk::raii::Semaphore* RenderTarget::GetWaitSemaphore(const u32& index) const
    {
        return m_impl->GetWaitSemaphore(index);
    }

    const vk::raii::Semaphore* RenderTarget::GetSignalSemaphore(const u32& index) const
    {
        return m_impl->GetSignalSemaphore(index);
    }

    bool RenderTarget::CanPresent() const
    {
        return m_impl->CanPresent();
    }

    void RenderTarget::Present(const vk::raii::Queue& queue) const
    {
        m_impl->Present(queue, m_buffer_index);
    }

    bool RenderTarget::OnLoad()
    {
        return true; // No-Op: Render targets are blank on creation
    }

    bool RenderTarget::OnUnload()
    {
        return true; // No-Op: vulkan_raii cleans up on its own
    }

    // --------------------------
    // IMPLEMENTATIONS: Default
    // --------------------------

    RenderTargetImpl_Default::RenderTargetImpl_Default()
        : m_driver(nullptr)
    {

    }

    RenderTargetImpl_Default::~RenderTargetImpl_Default()
    {
        m_driver->WaitIdle(); // Let all GPU work finish before destruction
    }

    void RenderTargetImpl_Default::Configure(const Driver* driver, const GraphicsResource::Config* properties)
    {
        m_driver = driver;
        m_config = *dynamic_cast<const RenderTarget::Config*>(properties);

        const vk::SemaphoreCreateInfo sem_info{};
        vk::FenceCreateInfo fence_info{};
        fence_info.flags = vk::FenceCreateFlagBits::eSignaled;

        // Configure target textures and create sync objects
        for (u32 i = 0; i < m_config.frames_in_flight; ++i)
        {
            // Texture Config
            m_config.texture.name = m_config.name + "_" + std::to_string(i);
            m_targets.emplace_back(m_config.texture.name);
            m_targets.back().Attach(m_driver);
            m_targets.back().Configure(&m_config.texture);
        }
    }

    u32 RenderTargetImpl_Default::GetWidth() const
    {
        return m_config.texture.width;
    }

    u32 RenderTargetImpl_Default::GetHeight() const
    {
        return m_config.texture.height;
    }

    u32 RenderTargetImpl_Default::GetDepth() const
    {
        return m_config.texture.depth;
    }

    vk::MemoryRequirements RenderTargetImpl_Default::GetMemoryRequirements() const
    {
        return m_targets[0].GetMemoryRequirements();
    }

    void RenderTargetImpl_Default::BindDeviceMemory(
        std::shared_ptr<vk::raii::DeviceMemory>& memory,
        const u32& index,
        const u32& offset
    ) const
    {
        m_targets[index].BindDeviceMemory(memory, offset);
    }

    const vk::Image& RenderTargetImpl_Default::GetImage(const u32& index) const
    {
        return *m_targets[index].GetImage();
    }

    const vk::raii::ImageView& RenderTargetImpl_Default::GetView(const u32& index) const
    {
        return m_targets[index].GetView();
    }

    vk::Format RenderTargetImpl_Default::GetFormat() const
    {
        return m_config.texture.image.format;
    }

    const vk::raii::Semaphore* RenderTargetImpl_Default::GetWaitSemaphore(const u32& index) const
    {
        return nullptr;
    }

    const vk::raii::Semaphore* RenderTargetImpl_Default::GetSignalSemaphore(const u32& index) const
    {
        return nullptr;
    }

    void RenderTargetImpl_Default::SwapBuffers(const u32& frame_index, u32& image_index)
    {
        // Iterate linearly over buffer/frame indices
        image_index = (image_index + 1) % m_config.frames_in_flight;
    }

    bool RenderTargetImpl_Default::CanPresent() const
    {
        return false;
    }

    void RenderTargetImpl_Default::Present(const vk::raii::Queue& queue, const u32& buffer_index)
    {
        // No-op for 'offline' targets
    }

    // --------------------------
    // IMPLEMENTATIONS: Swapchain
    // --------------------------

    RenderTargetImpl_Swapchain::RenderTargetImpl_Swapchain()
        : m_win_props(nullptr), m_driver(nullptr), m_surface(nullptr), m_swapchain(nullptr), m_present_queue(nullptr)
    {

    }

    RenderTargetImpl_Swapchain::~RenderTargetImpl_Swapchain()
    {
        m_driver->WaitIdle(); // Let all GPU work finish before destruction
    }

    void RenderTargetImpl_Swapchain::Configure(const Driver* driver, const GraphicsResource::Config* properties)
    {
        m_driver = driver;
        m_config = *dynamic_cast<const RenderTarget::SwapchainConfig*>(properties);
        m_win_props = &m_config.window->GetProperties();

        // Create the presentation surface
        m_surface = m_driver->CreateWindowSurface(m_config.window);

        // Ensure presentation is supported on said surface
        m_driver->ValidatePresentSupport(m_surface);

        // Create the swapchain
        // TODO: REWORK
        m_swapchain = m_driver->CreateSwapchain(m_surface, m_config, &m_swapchain);

        // Extract the images from the swapchain
        m_images = m_swapchain.getImages();
        m_views.reserve(m_images.size());

        m_config.view_config.format = m_config.format;

        const vk::SemaphoreCreateInfo sem_info{};

        // Create image views and sync objects
        m_wait_semaphores.reserve(m_images.size());
        m_signal_semaphores.reserve(m_images.size());
        for (u64 i = 0; i < m_images.size(); ++i)
        {
            // Image view
            m_views.emplace(m_views.begin() + i, m_driver->AllocateImageView(m_images[i], m_config.view_config));

            m_wait_semaphores.push_back(m_driver->CreateSemaphore(sem_info));
            m_signal_semaphores.push_back(m_driver->CreateSemaphore(sem_info));
        }
    }

    u32 RenderTargetImpl_Swapchain::GetWidth() const
    {
        return m_win_props->width;
    }

    u32 RenderTargetImpl_Swapchain::GetHeight() const
    {
        return m_win_props->height;
    }

    u32 RenderTargetImpl_Swapchain::GetDepth() const
    {
        return 1;
    }

    vk::MemoryRequirements RenderTargetImpl_Swapchain::GetMemoryRequirements() const
    {
        return {};
    }

    void RenderTargetImpl_Swapchain::BindDeviceMemory(std::shared_ptr<vk::raii::DeviceMemory>& memory,
        const u32& offset, const u32& index) const
    {
        // No op for swapchains
    }

    const vk::Image& RenderTargetImpl_Swapchain::GetImage(const u32& index) const
    {
        return m_images[index];
    }

    const vk::raii::ImageView& RenderTargetImpl_Swapchain::GetView(const u32& index) const
    {
        return m_views[index];
    }

    vk::Format RenderTargetImpl_Swapchain::GetFormat() const
    {
        return m_config.format;
    }

    const vk::raii::Semaphore* RenderTargetImpl_Swapchain::GetWaitSemaphore(const u32& index) const
    {
        return &m_wait_semaphores[index % m_wait_semaphores.size()];
    }

    const vk::raii::Semaphore* RenderTargetImpl_Swapchain::GetSignalSemaphore(const u32& index) const
    {
        return &m_signal_semaphores[index];
    }

    void RenderTargetImpl_Swapchain::SwapBuffers(const u32& frame_index, u32& image_index)
    {
        const u32 wait_sem_idx = frame_index % m_wait_semaphores.size();

        // Let swapchain control which image index to resolve to
        auto [result, img_idx] = m_swapchain.acquireNextImage(UINT64_MAX, *m_wait_semaphores[wait_sem_idx], nullptr);
        image_index = img_idx;

        // If the swapchain is out of date, flag for recreation and invalidate the internal buffer index
        if (result == vk::Result::eErrorOutOfDateKHR)
        {
            RecreateSwapchain();
            image_index = UINT32_MAX;
        } else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        {
            assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
            throw std::runtime_error("[Vulkan Render Target (Swapchain)] Failed to acquire swap chain image!");
        }
    }

    bool RenderTargetImpl_Swapchain::CanPresent() const
    {
        return m_swapchain != nullptr;
    }

    void RenderTargetImpl_Swapchain::Present(const vk::raii::Queue& queue, const u32& buffer_index)
    {
        vk::PresentInfoKHR present_info;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &*m_signal_semaphores[buffer_index];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &*m_swapchain;
        present_info.pImageIndices = &buffer_index;

        auto result = queue.presentKHR(present_info);
    }

    void RenderTargetImpl_Swapchain::RecreateSwapchain()
    {
        // Clean up stale images/views
        m_views.clear();
        m_images.clear();

        // Recreate swapchain
        m_swapchain = m_driver->CreateSwapchain(m_surface, m_config, &m_swapchain);

        // Extract the images from the swapchain
        m_images = m_swapchain.getImages();
        m_views.reserve(m_images.size());

        m_config.view_config.format = m_config.format;

        const vk::SemaphoreCreateInfo sem_info{};

        // Create image views and sync objects
        for (u64 i = 0; i < m_images.size(); ++i)
        {
            // Image view
            m_views.emplace(m_views.begin() + i, m_driver->AllocateImageView(m_images[i], m_config.view_config));
        }
    }
}
