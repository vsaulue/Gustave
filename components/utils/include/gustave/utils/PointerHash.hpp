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
#include <functional>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace Gustave::Utils {
    struct PointerHash {
    public:
        struct Hash {
        public:
            using is_transparent = void;

            [[nodiscard]]
            constexpr Hash() = default;

            [[nodiscard]]
            std::size_t operator()(auto const& value) const {
                return std::hash<void const*>{}(getKey(value));
            }
        };

        struct Equals {
            using is_transparent = void;

            [[nodiscard]]
            constexpr Equals() = default;

            [[nodiscard]]
            bool operator()(auto const& lhs, auto const& rhs) const {
                return getKey(lhs) == getKey(rhs);
            }
        };

        template<typename Key, typename Value>
        using Map = std::unordered_map<Key, Value, Hash, Equals>;

        template<typename Key>
        using Set = std::unordered_set<Key, Hash, Equals>;
    private:
        [[nodiscard]]
        static void const* getKey(void const* value) {
            return value;
        }

        template<typename T, typename Deleter>
        [[nodiscard]]
        static T* getKey(std::unique_ptr<T, Deleter> const& value) {
            return value.get();
        }

        template<typename T>
        [[nodiscard]]
        static T* getKey(std::shared_ptr<T> const& value) {
            return value.get();
        }
    };
}
