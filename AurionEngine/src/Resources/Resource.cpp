module;

#include <functional>
#include <string>

module Aurion.Resources;

namespace Aurion
{
    Resource::Resource(const std::string_view& id)
        : m_loaded(false)
    {
        m_alias = std::string(id);
        m_id = std::hash<std::string_view>()(id);
    }

    const u64& Resource::GetId() const { return m_id; }
    std::string_view Resource::GetName() const { return m_alias; }
    bool Resource::IsLoaded() const { return m_loaded; }

    bool Resource::Load()
    {
        m_loaded = this->OnLoad();
        return m_loaded;
    }

    void Resource::Unload()
    {
        this->OnUnload();
        m_loaded = false;
    }
}
