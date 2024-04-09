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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/meta/Meta.hpp>
#include <gustave/model/Stress.hpp>

namespace gustave::solvers::structure {
    template<cfg::cLibConfig auto libCfg>
    struct Link {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;
    public:
        using LinkIndex = cfg::LinkIndex<libCfg>;
        using Conductivity = model::ConductivityStress<libCfg>;
        using PressureStress = model::PressureStress<libCfg>;
        using NodeIndex = cfg::NodeIndex<libCfg>;

        [[nodiscard]]
        explicit Link(NodeIndex id1, NodeIndex id2, NormalizedVector3 const& normal, Real<u.area> area, Real<u.length> thickness, PressureStress const& maxStress)
            : localNodeId_{ id1 }
            , otherNodeId_{ id2 }
            , normal_{ normal }
            , conductivity_{ (area / thickness) * maxStress }
        {
            assert(id1 != id2);
            assert(conductivity_.compression() > 0.f * u.conductivity);
            assert(conductivity_.shear() > 0.f * u.conductivity);
            assert(conductivity_.tensile() > 0.f * u.conductivity);
        }

        [[nodiscard]]
        NodeIndex localNodeId() const {
            return localNodeId_;
        }

        [[nodiscard]]
        NodeIndex otherNodeId() const {
            return otherNodeId_;
        }

        [[nodiscard]]
        const NormalizedVector3& normal() const {
            return normal_;
        }

        [[nodiscard]]
        Conductivity const& conductivity() const {
            return conductivity_;
        }
    private:
        NodeIndex localNodeId_;
        NodeIndex otherNodeId_;
        NormalizedVector3 normal_; // normal at the surface of localNode.
        Conductivity conductivity_;
    };
}
