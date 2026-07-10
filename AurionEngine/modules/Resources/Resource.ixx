module;

#include <string>

export module Aurion.Resources:Resource;

import Aurion.Types;

export namespace Aurion
{
    class Resource
    {
    public:
        explicit Resource(const std::string_view& id);
        virtual ~Resource() = default;

        const u64& GetId() const;
        virtual std::string_view GetName() const;

        bool IsLoaded() const;

        bool Load();
        void Unload();

    protected:
        virtual bool OnLoad() = 0;
        virtual bool OnUnload() = 0;

    private:
        std::string m_alias;
        u64 m_id;
        bool m_loaded;
    };
}