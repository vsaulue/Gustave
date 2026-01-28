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

#include <concepts>
#include <memory>
#include <unordered_map>

#include <gustave/meta/Meta.hpp>
#include <gustave/utils/cHashable.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::utils {
    template<typename T>
    concept cHashIndex = std::copy_constructible<T> && cHashable<T>;

    template<typename T>
    concept cIndexedItem = requires (T const& cv) {
        { meta::value(cv.index()) } -> cHashIndex;
    };

    template<cIndexedItem Value_>
    class SharedIndexedSet {
    public:
        using Index = decltype(meta::value(std::declval<Value_ const&>().index()));
        using Value = Value_;

        template<typename T>
        using SharedPtr = prop::SharedPtr<T>;
    private:
        using Map = std::unordered_map<Index, SharedPtr<Value>>;

        template<bool isMut_>
        class Enumerator {
        private:
            using MapIterator = utils::PropIterator<isMut_, Map>;
        public:
            using MapMember = utils::Prop<isMut_, Map>;
            using ItValue = std::conditional_t<isMut_, std::shared_ptr<Value>, SharedPtr<Value>>;
            using Reference = ItValue const&;

            [[nodiscard]]
            Enumerator()
                : map_{ nullptr }
                , mapIt_{}
            {}

            [[nodiscard]]
            explicit Enumerator(MapMember& structures)
                : map_{ &structures }
                , mapIt_{ structures.begin() }
            {}

            [[nodiscard]]
            bool isEnd() const {
                return mapIt_ == map_->end();
            }

            void operator++() {
                ++mapIt_;
            }

            [[nodiscard]]
            Reference operator*() const {
                if constexpr (isMut_) {
                    return mapIt_->second.unprop();
                }
                else {
                    return mapIt_->second;
                }
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return mapIt_ == other.mapIt_;
            }
        private:
            MapMember* map_;
            MapIterator mapIt_;
        };
    public:
        using Iterator = ForwardIterator<Enumerator<true>>;
        using ConstIterator = ForwardIterator<Enumerator<false>>;

        [[nodiscard]]
        SharedIndexedSet() = default;

        [[nodiscard]]
        SharedIndexedSet(SharedIndexedSet&&) = default;

        [[nodiscard]]
        SharedIndexedSet(SharedIndexedSet&) = default;

        SharedIndexedSet& operator=(SharedIndexedSet&&) = default;
        SharedIndexedSet& operator=(SharedIndexedSet&) = default;

        [[nodiscard]]
        Value& at(Index const& id) {
            return *map_.at(id);
        }

        [[nodiscard]]
        Value const& at(Index const& id) const {
            return *map_.at(id);
        }

        [[nodiscard]]
        SharedPtr<Value> atShared(Index const& id) {
            return map_.at(id);
        }

        [[nodiscard]]
        std::shared_ptr<Value const> atShared(Index const& id) const {
            return map_.at(id);
        }

        [[nodiscard]]
        Iterator begin() {
            return Iterator{ map_ };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ map_ };
        }

        [[nodiscard]]
        bool contains(Index const& id) const {
            return map_.contains(id);
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }

        template<typename... ValueCtorArgs>
        Value& emplace(ValueCtorArgs&&... valueCtorArgs)
            requires std::constructible_from<Value, ValueCtorArgs...>
        {
            return insert(std::make_shared<Value>(std::forward<ValueCtorArgs>(valueCtorArgs)...));
        }


        bool erase(Index const& id) {
            auto const numDeleted = map_.erase(id);
            return numDeleted > 0;
        }

        SharedPtr<Value> extract(Index const& id) {
            if (auto node = map_.extract(id)) {
                return std::move(node.mapped());
            } else {
                return nullptr;
            }
        }

        [[nodiscard]]
        Value* find(Index const& id) {
            return doFind(*this, id);
        }

        [[nodiscard]]
        Value const* find(Index const& id) const {
            return doFind(*this, id);
        }

        [[nodiscard]]
        SharedPtr<Value> findShared(Index const& id) {
            return doFindShared(*this, id);
        }

        [[nodiscard]]
        std::shared_ptr<Value const> findShared(Index const& id) const {
            return doFindShared(*this, id);
        }

        Value& insert(SharedPtr<Value> newValue) {
            assert(newValue);
            [[maybe_unused]]
            auto const res = map_.insert({ newValue->index(), std::move(newValue) });
            assert(res.second);
            return *res.first->second;
        }

        [[nodiscard]]
        std::size_t size() const {
            return map_.size();
        }
    private:
        [[nodiscard]]
        static auto doFind(meta::cCvRefOf<SharedIndexedSet> auto&& self, Index const& id) -> decltype(self.find(id)) {
            auto const it = self.map_.find(id);
            if (it != self.map_.end()) {
                return it->second.get();
            } else {
                return nullptr;
            }
        }

        [[nodiscard]]
        static auto doFindShared(meta::cCvRefOf<SharedIndexedSet> auto&& self, Index const& id) -> decltype(self.findShared(id)) {
            auto const it = self.map_.find(id);
            if (it != self.map_.end()) {
                return it->second;
            } else {
                return nullptr;
            }
        }

        Map map_;
    };
}
