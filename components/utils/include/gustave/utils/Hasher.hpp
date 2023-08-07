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

#include <cstddef>
#include <functional>

#include <gustave/utils/cHashable.hpp>
#include <gustave/utils/cHashableMemberOf.hpp>
#include <gustave/utils/getMember.hpp>

namespace Gustave::Utils {
    namespace detail {
        [[nodiscard]]
        inline std::size_t composeHash(std::size_t const h1, std::size_t const h2) {
            static_assert(sizeof(std::size_t) == 8, "This implementation only works for 64bits hashing.");
            return h1 ^ (h2 + 0x517cc1b727220a95 + (h1 << 6) + (h1 >> 2));
        }

        template<cHashable Arg>
        [[nodiscard]]
        std::size_t doHash(Arg const& arg) {
            return std::hash<Arg>{}(arg);
        }
    }

    template<typename T, auto... members>
        // constraint moved into requires clause because of MSVC bug:
        // https://developercommunity.visualstudio.com/t/Template-parameters:-constraints-dependi/10312655#T-ND10364982
        requires (cHashableMemberOf<decltype(members),T> && ...)
    struct Hasher {
        [[nodiscard]]
        constexpr Hasher() = default;

        [[nodiscard]]
        std::size_t operator()(T const& obj) const {
            std::size_t result = 0;
            ((result = detail::composeHash(result, detail::doHash(getMember(obj, members)))), ...);
            return result;
        }
    };
}
