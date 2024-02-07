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

#include <cassert>
#include <stdexcept>
#include <vector>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/solvers/force1/detail/ContactInfo.hpp>
#include <gustave/utils/canNarrow.hpp>

namespace Gustave::Solvers::Force1::detail {
    template<Cfg::cLibConfig auto cfg>
    class NodeInfo {
    private:
        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        static constexpr auto u = Cfg::units(cfg);
    public:
        using Contacts = std::vector<ContactInfo<cfg>>;
        using LinkIndex = Cfg::LinkIndex<cfg>;
        using LocalContactIndex = LinkIndex;
        using NodeIndex = Cfg::NodeIndex<cfg>;

        [[nodiscard]]
        explicit NodeInfo(Real<u.force> weight)
            : weight{ weight }
        {
            assert(weight > 0.f * u.force);
        }

        LocalContactIndex addContact(NodeIndex otherIndex, LinkIndex linkIndex, Real<u.resistance> rPlus, Real<u.resistance> rMinus) {
            auto const result = contacts.size();
            assert(Utils::canNarrow<LocalContactIndex>(result));
            contacts.emplace_back(otherIndex, linkIndex, rPlus, rMinus);
            return result;
        }

        Contacts contacts;
        Real<u.force> weight;
    };
}
