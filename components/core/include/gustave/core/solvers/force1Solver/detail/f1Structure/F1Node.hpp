/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2025 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/solvers/force1Solver/detail/f1Structure/F1Contact.hpp>
#include <gustave/utils/canNarrow.hpp>
#include <gustave/utils/IndexRange.hpp>

namespace gustave::core::solvers::force1Solver::detail::f1Structure {
    template<cfg::cLibConfig auto libCfg>
    class F1Node {
    private:
        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        static constexpr auto u = cfg::units(libCfg);
    public:
        using ContactIndex = cfg::LinkIndex<libCfg>;
        using F1Contact = f1Structure::F1Contact<libCfg>;
        using LinkIndex = cfg::LinkIndex<libCfg>;
        using LocalContactIndex = LinkIndex;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        using ContactIds = utils::IndexRange<ContactIndex>;

        [[nodiscard]]
        explicit F1Node(Real<u.force> weight, bool isFoundation)
            : weight{ weight }
            , isFoundation{ isFoundation }
        {
            assert(weight > 0.f * u.force);
        }

        [[nodiscard]]
        bool operator==(F1Node const&) const = default;

        ContactIds contactIds;
        Real<u.force> weight;
        bool isFoundation;
    };
}
