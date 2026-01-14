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

#include <algorithm>
#include <cstddef>
#include <sstream>
#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/InplaceVector.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::blockReference {
    template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
    class Structures {
    public:
        template<bool mut>
        using StructureReference = cuboidGridScene::StructureReference<libCfg_, UD_, mut>;

        using StructureIndex = StructureReference<false>::StructureIndex;
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        using DataNeighbours = cuboidGridScene::detail::DataNeighbours<libCfg_, UD_, false>;
        using SceneData = cuboidGridScene::detail::SceneData<libCfg_, UD_>;

        using BlockData = SceneData::BlockData;

        using Values = utils::InplaceVector<StructureIndex, 6>;

        template<bool mut>
        class Enumerator {
        public:
            using Value = cuboidGridScene::StructureReference<libCfg_, UD_, mut>;
        private:
            template<typename T>
            using Prop = utils::Prop<mut, T>;

            using IdsIterator = Values::ConstIterator;
        public:
            [[nodiscard]]
            Enumerator()
                : structs_{ nullptr }
                , indexIt_{}
            {}

            [[nodiscard]]
            explicit Enumerator(Prop<Structures>& structures)
                : structs_{ &structures }
                , indexIt_{ structures.structIds_.begin() }
            {}

            [[nodiscard]]
            bool isEnd() const {
                return indexIt_ == structs_->structIds_.end();
            }

            void operator++() {
                ++indexIt_;
            }

            [[nodiscard]]
            Value operator*() const {
                assert(not isEnd());
                return Value{ structs_->scene_->structures.atShared(*indexIt_) };
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return indexIt_ == other.indexIt_;
            }
        private:
            Prop<Structures>* structs_;
            IdsIterator indexIt_;
        };
    public:
        using ConstIterator = utils::ForwardIterator<Enumerator<false>>;
        using Iterator = utils::ForwardIterator<Enumerator<isMut_>>;

        [[nodiscard]]
        explicit Structures(Prop<SceneData>& scene, BlockData const& blockData)
            : scene_{ &scene }
            , block_{ &blockData }
        {
            if (blockData.isFoundation()) {
                for (auto const& neighbour : DataNeighbours{ scene, blockData.index() }) {
                    auto const& nBlockData = neighbour.otherBlock();
                    if (!nBlockData.isFoundation()) {
                        addValue(nBlockData.structureId());
                    }
                }
            } else {
                addValue(blockData.structureId());
            }
        }

        [[nodiscard]]
        StructureReference<true> operator[](std::size_t index)
            requires (isMut_)
        {
            return StructureReference<true>{ scene_->structures.atShared(structIds_[index]) };
        }

        [[nodiscard]]
        StructureReference<false> operator[](std::size_t index) const {
            return StructureReference<false>{ scene_->structures.atShared(structIds_[index]) };
        }

        [[nodiscard]]
        Iterator begin()
            requires (isMut_)
        {
            return Iterator{ *this };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ *this };
        }

        [[nodiscard]]
        std::default_sentinel_t end() const {
            return {};
        }

        [[nodiscard]]
        std::size_t size() const {
            return structIds_.size();
        }

        [[nodiscard]]
        StructureReference<true> unique()
            requires (isMut_)
        {
            return doUnique(*this);
        }

        [[nodiscard]]
        StructureReference<false> unique() const {
            return doUnique(*this);
        }
    private:
        [[nodiscard]]
        static auto doUnique(meta::cCvRefOf<Structures> auto&& self) {
            using Result = decltype(self.unique());
            if (self.structIds_.size() != 1) {
                throw self.noUniqueError();
            }
            return Result{ self.scene_->structures.atShared(self.structIds_[0]) };
        }

        [[nodiscard]]
        std::logic_error noUniqueError() const {
            std::stringstream msg;
            msg << "Block " << block_->index() << " does not have a unique structure (count = " << structIds_.size() << ").";
            return std::logic_error{ msg.str() };
        }

        void addValue(StructureIndex newId) {
            if (std::ranges::find(structIds_, newId) == structIds_.end()) {
                structIds_.pushBack(newId);
            }
        }

        PropPtr<SceneData> scene_;
        Values structIds_;
        BlockData const* block_;
    };
}
