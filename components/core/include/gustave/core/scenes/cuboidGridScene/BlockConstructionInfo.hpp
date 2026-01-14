/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg>
    class BlockConstructionInfo {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;
    public:
        using BlockIndex = cuboidGridScene::BlockIndex;
        using PressureStress = model::PressureStress<libCfg>;

        [[nodiscard]]
        BlockConstructionInfo(BlockIndex const& index, PressureStress const& maxPressureStress, Real<u.mass> mass, bool isFoundation)
            : index_{ index }
            , maxPressureStress_{ maxPressureStress }
            , mass_{ mass }
            , isFoundation_{ isFoundation }
        {
            checkMass(mass);
            if (maxPressureStress_.compression() <= 0.f * u.pressure) {
                throw invalidMaxStressError("compression", maxPressureStress_.compression());
            }
            if (maxPressureStress_.shear() <= 0.f * u.pressure) {
                throw invalidMaxStressError("shear", maxPressureStress_.shear());
            }
            if (maxPressureStress_.tensile() <= 0.f * u.pressure) {
                throw invalidMaxStressError("tensile", maxPressureStress_.tensile());
            }
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
        PressureStress const& maxPressureStress() const {
            return maxPressureStress_;
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
        PressureStress maxPressureStress_;
        Real<u.mass> mass_;
        bool isFoundation_;

        [[nodiscard]]
        static std::invalid_argument invalidMaxStressError(std::string name, Real<u.pressure> value) {
            std::stringstream result;
            result << "maxPressureStress." << name << " must be strictly positive (passed: " << value << ").";
            return std::invalid_argument(result.str());
        }

        void checkMass(Real<u.mass> value) {
            if (value <= 0.f * u.mass) {
                std::stringstream msg;
                msg << "Expected a strictly positive mass (passed: " << value << ").";
                throw std::invalid_argument(msg.str());
            }
        }
    };
}
