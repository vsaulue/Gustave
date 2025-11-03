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
#include <optional>
#include <unordered_map>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/utils/prop/SharedPtr.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/EndIterator.hpp>

namespace gustave::core::scenes::cuboidGridScene::detail {
    template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UserData_>
    class SceneStructures {
    public:
        using StructureData = detail::StructureData<libCfg_, UserData_>;
        using StructureIndex = cfg::StructureIndex<libCfg_>;

        template<typename T>
        using SharedPtr = utils::prop::SharedPtr<T>;
    private:
        using StructuresMap = std::unordered_map<StructureIndex, SharedPtr<StructureData>>;

        template<bool isMut_>
        class Enumerator {
        private:
            using PairIterator = std::conditional_t<isMut_, typename StructuresMap::iterator, typename StructuresMap::const_iterator>;
        public:
            using StructuresMember = meta::MutableIf<isMut_, StructuresMap>;
            using Value = std::conditional_t<isMut_, std::shared_ptr<StructureData>, SharedPtr<StructureData>>;
            using Reference =  Value const&;

            [[nodiscard]]
            Enumerator()
                : structures_{ nullptr }
                , pairIt_{}
            {}

            [[nodiscard]]
            explicit Enumerator(StructuresMember& structures)
                : structures_{ &structures }
                , pairIt_{ structures.begin() }
            {}

            [[nodiscard]]
            bool isEnd() const {
                return pairIt_ == structures_->end();
            }

            void operator++() {
                ++pairIt_;
            }

            [[nodiscard]]
            Reference operator*() const {
                if constexpr (isMut_) {
                    return pairIt_->second.unprop();
                } else {
                    return pairIt_->second;
                }
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return pairIt_ == other.pairIt_;
            }
        private:
            StructuresMember* structures_;
            PairIterator pairIt_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator<true>>;
        using ConstIterator = utils::ForwardIterator<Enumerator<false>>;

        [[nodiscard]]
        SceneStructures() = default;

        [[nodiscard]]
        SceneStructures(SceneStructures&&) = default;

        [[nodiscard]]
        SceneStructures(SceneStructures&) = default;

        SceneStructures& operator=(SceneStructures&&) = default;
        SceneStructures& operator=(SceneStructures&) = default;

        [[nodiscard]]
        StructureData& at(StructureIndex id) {
            return *structures_.at(id);;
        }

        [[nodiscard]]
        StructureData const& at(StructureIndex id) const {
            return *structures_.at(id);
        }

        [[nodiscard]]
        SharedPtr<StructureData> atShared(StructureIndex id) {
            return structures_.at(id);
        }

        [[nodiscard]]
        std::shared_ptr<StructureData const> atShared(StructureIndex id) const {
            return structures_.at(id);
        }

        [[nodiscard]]
        Iterator begin() {
            return Iterator{ structures_ };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ structures_ };
        }

        [[nodiscard]]
        bool contains(StructureIndex id) const {
            return structures_.contains(id);
        }

        [[nodiscard]]
        constexpr utils::EndIterator end() const {
            return {};
        }

        bool erase(StructureIndex id) {
            std::size_t const numDeleted = structures_.erase(id);
            return numDeleted > 0;
        }

        [[nodiscard]]
        SharedPtr<StructureData> findShared(StructureIndex id) {
            return doFindShared(*this, id);
        }

        [[nodiscard]]
        std::shared_ptr<StructureData const> findShared(StructureIndex id) const {
            return doFindShared(*this, id);
        }

        void insert(SharedPtr<StructureData> newValue) {
            assert(newValue);
            [[maybe_unused]]
            auto const res = structures_.insert({ newValue->index(), std::move(newValue) });
            assert(res.second);
        }

        [[nodiscard]]
        std::size_t size() const {
            return structures_.size();
        }
    private:
        [[nodiscard]]
        static auto doFindShared(meta::cCvRefOf<SceneStructures> auto&& self, StructureIndex id) -> decltype(self.findShared(id)) {
            auto const it = self.structures_.find(id);
            if (it != self.structures_.end()) {
                return it->second;
            } else {
                return nullptr;
            }
        }

        StructuresMap structures_;
    };
}
