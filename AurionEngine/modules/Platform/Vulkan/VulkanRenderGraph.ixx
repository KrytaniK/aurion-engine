module;

#include <vulkan/vulkan_raii.hpp>
#include <memory>
#include <vector>
#include <span>
#include <cstdint>
#include <string>
#include <typeindex>
#include <typeinfo>

export module Aurion.Vulkan:RenderGraph;

import Aurion.Graphics;
import Aurion.Resources;
import Aurion.Types;

import :Buffer;
import :RenderTarget;

export namespace Aurion::Vulkan
{
    class Driver;

    class RenderGraph : public Aurion::RenderGraph
    {
        template<typename R>
        struct ResourceConfig
        {
            R::Config config;
            u64 index = UINT64_MAX;
        };

    public:
        explicit RenderGraph(const std::shared_ptr<IGraphicsDriver>& renderer);
        ~RenderGraph() override = default;

        void RegisterBuffer(const ResourceHandle<Aurion::Buffer>& buffer) override;
        void RegisterRenderTarget(const ResourceHandle<Aurion::RenderTarget>& render_target) override;

        const ResourceHandle<Aurion::Buffer>& CreateBuffer(const Aurion::Buffer::Config* desc) override;
        const ResourceHandle<Aurion::RenderTarget>& CreateRenderTarget(const Aurion::RenderTarget::Config* desc) override;

        void AddPass(const RenderPass::Config* desc) override;

        // Builds the execution sequence for supplied render passes based on resource requirements.
        void Compile() override;

        // Specifies which render target this graph should export
        void Export(const std::string& render_target, const u32& version) override;

        const ResourceHandle<Aurion::RenderTarget>& GetExportTarget() const override;

    private:
        // Builds a dependency graph (as a DAG) from imported/transient resources and pass descriptions
        [[nodiscard]] std::vector<std::vector<u64>> BuildDependencyGraph() const;

        // Filters the dependency graph based on the exported render target
        [[nodiscard]] std::vector<u8> CullPasses();

        // Sorts passes by index based on the provided dependency graph
        [[nodiscard]] std::vector<u64> TopologicallySortPasses(std::span<u8> mask) const;

        void AliasResources(std::span<u64> execution_order);

    private:
        std::shared_ptr<Driver> m_driver;
        std::vector<std::vector<u64>> m_dependency_graph;
        Aurion::RenderPass::ResourceRef m_export_target_ref;
        ResourceHandle<Aurion::RenderTarget> m_export_target; //
        std::vector<ResourceHandle<Aurion::Buffer>> m_buffers;
        std::vector<ResourceHandle<Aurion::RenderTarget>> m_render_targets;
        std::vector<ResourceConfig<Vulkan::Buffer>> m_buffer_configs; //
        std::vector<ResourceConfig<Vulkan::RenderTarget>> m_render_target_configs; //
        std::vector<const Aurion::RenderPass::Config*> m_pass_descriptions; //
    };
}