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

#include <unordered_set>

#include <gustave/meta/Meta.hpp>
#include <gustave/utils/cHashableMemberOf.hpp>
#include <gustave/utils/getMember.hpp>
#include <gustave/utils/Hasher.hpp>

namespace gustave::utils {
    template<typename T, typename Object>
    concept cEqualsMemberOf = requires (Object const& object, T const& getter) {
        { getMember(object, getter) } -> std::equality_comparable;
    };

    template<typename T, typename Object>
    concept cHashEqualsMemberOf = cHashableMemberOf<T, Object>&& cEqualsMemberOf<T, Object>;

    template<typename T, auto... members>
        // constraint moved into requires clause because of MSVC bug:
        // https://developercommunity.visualstudio.com/t/Template-parameters:-constraints-dependi/10312655#T-ND10364982
        requires (cHashEqualsMemberOf<decltype(members),T> && ...)
    struct HashEquals {
        using Hash = Hasher<T, members...>;

        struct Equals {
            bool operator()(T const& lhs, T const& rhs) const {
                return ((getMember(lhs, members) == getMember(rhs, members)) && ...);
            }
        };

        using Set = std::unordered_set<T, Hash, Equals>;
    };
}
