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

#include <sstream>
#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/model/MaxStress.hpp>
#include <gustave/scenes/cuboidGridScene/BlockIndex.hpp>

namespace gustave::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg>
    class BlockConstructionInfo {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        using MaxStress = model::MaxStress<libCfg>;
    public:
        [[nodiscard]]
        BlockConstructionInfo(BlockIndex const& index, MaxStress const& maxStress, Real<u.mass> mass, bool isFoundation)
            : index_{ index }
            , maxStress_{ maxStress }
            , mass_{ mass }
            , isFoundation_{ isFoundation }
        {
            checkMass(mass);
        }

        [[nodiscard]]
        BlockIndex& index() {
            return index_;
        }

        [[nodiscard]]
        BlockIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        MaxStress& maxStress() {
            return maxStress_;
        }

        [[nodiscard]]
        MaxStress const& maxStress() const {
            return maxStress_;
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return mass_;
        }

        void setMass(Real<u.mass> value) {
            checkMass(value);
            mass_ = value;
        }

        [[nodiscard]]
        bool& isFoundation() {
            return isFoundation_;
        }

        [[nodiscard]]
        bool isFoundation() const {
            return isFoundation_;
        }

        [[nodiscard]]
        bool operator==(BlockConstructionInfo const&) const = default;
    private:
        BlockIndex index_;
        MaxStress maxStress_;
        Real<u.mass> mass_;
        bool isFoundation_;

        void checkMass(Real<u.mass> value) {
            if (value <= 0.f * u.mass) {
                std::stringstream msg;
                msg << "Expected a strictly positive mass (passed: " << value << ").";
                throw std::invalid_argument(msg.str());
            }
        }
    };
}
