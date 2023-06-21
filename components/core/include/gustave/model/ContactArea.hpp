/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <gustave/model/Material.hpp>

namespace Gustave::Model {
    template<Cfg::cLibConfig auto cfg>
    struct ContactArea {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        using NormalizedVector3 = Cfg::NormalizedVector3<cfg>;
        using NodeIndex = Cfg::NodeIndex<cfg>;
    public:
        [[nodiscard]]
        ContactArea(NodeIndex id1, NodeIndex id2, NormalizedVector3 const& normal, Real<u.area> area, Real<u.length> thickness, Material<cfg> const& maxConstraints)
            : localNodeId_(id1)
            , otherNodeId_(id2)
            , normal_(normal)
            , area_(area)
            , thickness_(thickness)
            , maxConstraints_(maxConstraints)
        {
            assert(id1 != id2);
            assert(area > 0.0f * u.area);
            assert(thickness > 0.0f * u.length);
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
        Real<u.area> area() const {
            return area_;
        }

        [[nodiscard]]
        Real<u.length> thickness() const {
            return thickness_;
        }

        [[nodiscard]]
        const Material<cfg>& maxConstraints() const {
            return maxConstraints_;
        }

        [[nodiscard]]
        Real<u.force> maxCompressionForce() const {
            return area_ * maxConstraints_.maxCompressionStress();
        }

        [[nodiscard]]
        Real<u.force> maxShearForce() const {
            return area_ * maxConstraints_.maxShearStress();
        }

        [[nodiscard]]
        Real<u.force> maxTensileForce() const {
            return area_ * maxConstraints_.maxTensileStress();
        }

        [[nodiscard]]
        Real<u.conductivity> compressionConductivity() const {
            return maxCompressionForce() / thickness_;
        }

        [[nodiscard]]
        Real<u.conductivity> shearConductivity() const {
            return maxShearForce() / thickness_;
        }

        [[nodiscard]]
        Real<u.conductivity> tensileConductivity() const {
            return maxTensileForce() / thickness_;
        }
    private:
        NodeIndex localNodeId_;
        NodeIndex otherNodeId_;
        NormalizedVector3 normal_; // normal at the surface of localNode.
        Real<u.area> area_;
        Real<u.length> thickness_;
        Material<cfg> const& maxConstraints_;
    };
}
