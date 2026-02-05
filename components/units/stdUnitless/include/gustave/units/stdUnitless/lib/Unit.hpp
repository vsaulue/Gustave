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

#include <ostream>

#include <gustave/cfg/cRealRep.hpp>

namespace gustave::units::stdUnitless::lib {
    template<cfg::cRealRep Rep_>
    class Real;

    struct Unit {
    public:
        [[nodiscard]]
        constexpr Unit() = default;

        [[nodiscard]]
        static constexpr bool isAssignableFrom(Unit) {
            return true;
        }

        [[nodiscard]]
        static constexpr bool isOne() {
            return true;
        }

        [[nodiscard]]
        static constexpr bool isTrivialOne() {
            return true;
        }

        [[nodiscard]]
        constexpr Unit operator*(Unit) const {
            return {};
        }

        [[nodiscard]]
        constexpr Unit operator/(Unit) const {
            return {};
        }

        [[nodiscard]]
        constexpr bool operator==(Unit) const {
            return true;
        }

        friend std::ostream& operator<<(std::ostream& stream, Unit) {
            return stream;
        }

        [[nodiscard]]
        friend constexpr auto operator*(cfg::cRealRep auto value, Unit) {
            return Real{ value };
        }

        [[nodiscard]]
        friend constexpr auto operator/(cfg::cRealRep auto value, Unit) {
            return Real{ value };
        }
    };
}
