module;

#include <string>

export module Aurion.Assets:Interface;

import Aurion.Types;

export namespace Aurion
{
    // Resource Interface for any application-level resource
    struct IAsset
    {
        virtual ~IAsset() = default;

        [[nodiscard]] virtual const u64& GetID() const = 0;
        [[nodiscard]] virtual std::string_view GetAlias() const = 0;

        [[nodiscard]] virtual bool IsLoaded() const = 0;
    };
}