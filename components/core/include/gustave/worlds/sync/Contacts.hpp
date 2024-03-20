/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <gustave/worlds/sync/detail/WorldData.hpp>
#include <gustave/worlds/sync/ContactReference.hpp>

namespace Gustave::Worlds::Sync {
    template<Cfg::cLibConfig auto cfg>
    class Contacts {
    private:
        using WorldData = detail::WorldData<cfg>;
    public:
        using ContactReference = Sync::ContactReference<cfg>;

        using ContactIndex = typename ContactReference::ContactIndex;

        [[nodiscard]]
        explicit Contacts(WorldData const& world)
            : world_{ &world }
        {}

        [[nodiscard]]
        ContactReference at(ContactIndex const& index) const {
            ContactReference result{ *world_, index };
            if (!result.isValid()) {
                throw std::out_of_range(result.invalidMessage());
            }
            return result;
        }

        [[nodiscard]]
        ContactReference find(ContactIndex const& index) const {
            return ContactReference{ *world_, index };
        }
    private:
        WorldData const* world_;
    };
}
