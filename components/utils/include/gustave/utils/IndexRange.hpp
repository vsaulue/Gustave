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
#include <limits>
#include <type_traits>
#include <span>
#include <vector>

#include <gustave/utils/cIndex.hpp>

namespace gustave::utils {
    template<cIndex Index_>
    class IndexRange {
    public:
        using Index = Index_;

        [[nodiscard]]
        constexpr IndexRange()
            : start_{ 0 }
            , size_{ 0 }
        {}

        [[nodiscard]]
        constexpr IndexRange(Index start, Index size)
            : start_{ start }
            , size_{ size }
        {
            assert(size_ >= 0);
            assert(start_ <= std::numeric_limits<Index>::max() - size_);
        }

        [[nodiscard]]
        constexpr Index start() const {
            return start_;
        }

        [[nodiscard]]
        constexpr Index end() const {
            return start_ + size_;
        }

        [[nodiscard]]
        constexpr Index size() const {
            return size_;
        }

        constexpr void setStart(Index value) {
            assert(start_ <= std::numeric_limits<Index>::max() - value);
            start_ = value;
        }

        constexpr void setSize(Index value) {
            assert(value >= 0);
            assert(start_ <= std::numeric_limits<Index>::max() - value);
            size_ = value;
        }

        template<typename T>
        [[nodiscard]]
        constexpr std::span<T const> subSpanOf(std::vector<T> const& vec) const {
            assert(size_ <= vec.size());
            assert(start_ <= vec.size() - size_);
            return { &vec[start_], size_ };
        }

        template<typename T>
        [[nodiscard]]
        constexpr std::span<T> subSpanOf(std::span<T> span) const {
            assert(size_ <= span.size());
            assert(start_ <= span.size() - size_);
            return { &span[start_], size_};
        }

        [[nodiscard]]
        constexpr bool operator==(IndexRange const&) const = default;
    private:
        Index start_;
        Index size_;
    };

    template<cIndex Index>
    IndexRange(Index, Index) -> IndexRange<Index>;
}
