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

#include <cstddef>
#include <type_traits>
#include <utility>

#include <gustave/utils/cEnumerator.hpp>
#include <gustave/utils/EndIterator.hpp>

namespace Gustave::Utils {
    template<cEnumerator Enumerator>
    class ForwardIterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::remove_cvref_t<decltype(*std::declval<Enumerator&>())>;

        template<typename... Args>
        [[nodiscard]]
        explicit ForwardIterator(Args&&... args)
            : enumerator_{ std::forward<Args>(args)... }
        {}

        [[nodiscard]]
        value_type const& operator*() const {
            return *enumerator_;
        }

        [[nodiscard]]
        value_type const* operator->() const {
            return &*enumerator_;
        }

        ForwardIterator& operator++() {
            ++enumerator_;
            return *this;
        }

        [[nodiscard]]
        ForwardIterator operator++(int) {
            ForwardIterator result = *this;
            ++*this;
            return result;
        }

        [[nodiscard]]
        bool operator==(EndIterator const&) const {
            return enumerator_.isEnd();
        }

        [[nodiscard]]
        bool operator==(ForwardIterator const&) const = default;
    private:
        Enumerator enumerator_;
    };
}
