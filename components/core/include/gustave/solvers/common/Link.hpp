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
#include <gustave/model/MaxStress.hpp>

namespace Gustave::Solvers::Common {
    template<Cfg::cLibConfig auto cfg>
    struct Link {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        using MaxStress = Model::MaxStress<cfg>;
        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;
    public:
        [[nodiscard]]
        explicit Link(NodeIndex id1, NodeIndex id2, NormalizedVector3 const& normal, Real<u.area> area, Real<u.length> thickness, MaxStress const& maxStress)
            : localNodeId_{ id1 }
            , otherNodeId_{ id2 }
            , normal_{ normal }
            , compressionConductivity_{ maxStress.maxCompressionStress() * area / thickness }
            , shearConductivity_{ maxStress.maxShearStress() * area / thickness }
            , tensileConductivity_{ maxStress.maxTensileStress() * area / thickness }
        {
            assert(id1 != id2);
            assert(compressionConductivity_ > 0.f * u.conductivity);
            assert(shearConductivity_ > 0.f * u.conductivity);
            assert(tensileConductivity_ > 0.f * u.conductivity);
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
        Real<u.conductivity> compressionConductivity() const {
            return compressionConductivity_;
        }

        [[nodiscard]]
        Real<u.conductivity> shearConductivity() const {
            return shearConductivity_;
        }

        [[nodiscard]]
        Real<u.conductivity> tensileConductivity() const {
            return tensileConductivity_;
        }
    private:
        NodeIndex localNodeId_;
        NodeIndex otherNodeId_;
        NormalizedVector3 normal_; // normal at the surface of localNode.
        Real<u.conductivity> compressionConductivity_;
        Real<u.conductivity> shearConductivity_;
        Real<u.conductivity> tensileConductivity_;
    };
}
