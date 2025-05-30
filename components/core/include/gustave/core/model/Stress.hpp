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
#include <concepts>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/cUnitOf.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/math3d/cRealConstArg.hpp>
#include <gustave/meta/Meta.hpp>

namespace gustave::core::model {
    template<cfg::cLibConfig auto libCfg_, auto unit_>
        // Concept moved to a require clause due to a MSVC bug.
        // See https://developercommunity.visualstudio.com/t/Regression-Reject-valid-Wrong-concep/10736602
        requires cfg::cUnitOf<decltype(unit_), libCfg_>
    struct Stress;

    template<typename T>
    concept cStress = std::same_as<T, Stress<T::libCfg(), T::unit()>>;


    template<cfg::cLibConfig auto libCfg_, auto unit_>
        requires cfg::cUnitOf<decltype(unit_), libCfg_>
    struct Stress {
    private:
        static constexpr auto rt = libCfg_.realTraits;

        template<cfg::cUnitOf<libCfg_> auto rUnit>
        using Real = cfg::Real<libCfg_, rUnit>;
    public:
        using Coord = Real<unit_>;
        using Unit = decltype(unit_);

        [[nodiscard]]
        Stress(Coord compression, Coord shear, Coord tensile)
            : compression_{ compression }
            , shear_{ shear }
            , tensile_{ tensile }
        {
            assert(compression >= 0.f * unit_);
            assert(shear >= 0.f * unit_);
            assert(tensile >= 0.f * unit_);
        }

        template<cfg::cUnitOf<libCfg_> auto otherUnit>
        [[nodiscard]]
        Stress(Stress<libCfg_, otherUnit> const& other)
            : compression_{ other.compression() }
            , shear_{ other.shear() }
            , tensile_{ other.tensile() }
        {}

        [[nodiscard]]
        static constexpr auto libCfg() {
            return libCfg_;
        }

        [[nodiscard]]
        Coord maxCoord() const {
            return rt.max(rt.max(compression_, shear_), tensile_);
        }

        [[nodiscard]]
        static Stress minStress(Stress const& m1, Stress const& m2) {
            return {
                rt.min(m1.compression_, m2.compression_),
                rt.min(m1.shear_, m2.shear_),
                rt.min(m1.tensile_, m2.tensile_)
            };
        }

        template<cfg::cUnitOf<libCfg_> auto otherUnit_>
        void mergeMax(Stress<libCfg_, otherUnit_> const& other) {
            compression_ = rt.max(compression_, other.compression());
            shear_ = rt.max(shear_, other.shear());
            tensile_ = rt.max(tensile_, other.tensile());
        }

        [[nodiscard]]
        Coord compression() const {
            return compression_;
        }

        [[nodiscard]]
        Coord shear() const {
            return shear_;
        }

        [[nodiscard]]
        Coord tensile() const {
            return tensile_;
        }

        [[nodiscard]]
        static constexpr Unit unit() {
            return unit_;
        }

        [[nodiscard]]
        auto operator*(cfg::cReal auto rhs) const {
            constexpr auto rhsUnit = decltype(rhs)::unit();
            return Stress<libCfg_, unit_* rhsUnit>{ compression_ * rhs, shear_ * rhs, tensile_ * rhs };
        }

        [[nodiscard]]
        friend auto operator*(cfg::cReal auto lhs, Stress const& rhs) {
            constexpr auto lhsUnit = decltype(lhs)::unit();
            return Stress<libCfg_, lhsUnit* unit_>{ lhs* rhs.compression_, lhs* rhs.shear_, lhs* rhs.tensile_ };
        }

        template<cfg::cUnitOf<libCfg_> auto rhsUnit_>
        [[nodiscard]]
        auto operator/(Stress<libCfg_, rhsUnit_> const& rhs) const -> Stress<libCfg_, unit_ / rhsUnit_> {
            return { compression_ / rhs.compression(), shear_ / rhs.shear(), tensile_ / rhs.tensile() };
        }

        [[nodiscard]]
        auto operator/(cfg::cReal auto rhs) const {
            return Stress<libCfg_, unit_ / decltype(rhs)::unit()>{ compression_ / rhs, shear_ / rhs, tensile_ / rhs };
        }

        template<cfg::cUnitOf<libCfg_> auto otherUnit>
        [[nodiscard]]
        bool operator==(Stress<libCfg_, otherUnit> const& other) const {
            return compression_ == other.compression()
                && shear_ == other.shear()
                && tensile_ == other.tensile();
        }

        friend std::ostream& operator<<(std::ostream& stream, Stress const& stress) {
            stream << "{ \"compression\": " << stress.compression_.value();
            stream << ", \"shear\": " << stress.shear_.value();
            stream << ", \"tensile\": " << stress.tensile_.value();
            if (stress.unit().isTrivialOne()) {
                stream << " }";
            } else {
                stream << ", \"unit\": \"" << stress.unit() << "\" }";
            }
            return stream;
        }
    private:
        Coord compression_;
        Coord shear_;
        Coord tensile_;
    };

    template<cfg::cLibConfig auto libCfg>
    using ConductivityStress = Stress<libCfg, cfg::units(libCfg).conductivity>;

    template<cfg::cLibConfig auto libCfg>
    using ForceStress = Stress<libCfg, cfg::units(libCfg).force>;

    template<cfg::cLibConfig auto libCfg>
    using PressureStress = Stress<libCfg, cfg::units(libCfg).pressure>;

    template<cfg::cLibConfig auto libCfg>
    using StressRatio = Stress<libCfg, cfg::units(libCfg).one>;
}
