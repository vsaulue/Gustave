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

#include <array>
#include <cstddef>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::structureReference {
    namespace blocks::detail {
        template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
        class Enumerator {
        private:
            template<typename T>
            using Prop = utils::Prop<isMut_, T>;

            using StructureData = cuboidGridScene::detail::StructureData<libCfg_, UD_>;

            using DataIterator = typename StructureData::SolverIndices::const_iterator;
        public:
            using Value = cuboidGridScene::BlockReference<libCfg_, UD_, isMut_>;

            [[nodiscard]]
            Enumerator()
                : structureData_{ nullptr }
                , dataIterator_{}
            {}

            [[nodiscard]]
            explicit Enumerator(Prop<StructureData>& structureData)
                : structureData_{ &structureData }
                , dataIterator_{ structureData.solverIndices().begin() }
            {}

            [[nodiscard]]
            bool isEnd() const {
                return dataIterator_ == structureData_->solverIndices().end();
            }

            void operator++() {
                ++dataIterator_;
            }

            [[nodiscard]]
            Value operator*() const {
                return Value{ structureData_->sceneData(), dataIterator_->first };
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return dataIterator_ == other.dataIterator_;
            }
        private:
            Prop<StructureData>* structureData_;
            DataIterator dataIterator_;
        };
    }

    template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
    class Blocks {
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        template<bool mut>
        using Enumerator = blocks::detail::Enumerator<libCfg_, UD_, mut>;

        using StructureData = cuboidGridScene::detail::StructureData<libCfg_, UD_>;
    public:
        using Iterator = utils::ForwardIterator<Enumerator<isMut_>>;
        using ConstIterator = utils::ForwardIterator<Enumerator<false>>;

        template<bool mut>
        using BlockReference = cuboidGridScene::BlockReference<libCfg_, UD_, mut>;

        [[nodiscard]]
        explicit Blocks(Prop<StructureData>& data)
            : data_{ &data }
        {}

        [[nodiscard]]
        BlockReference<true> at(BlockIndex const& index)
            requires (isMut_)
        {
            return doAt(*this, index);
        }

        [[nodiscard]]
        BlockReference<false> at(BlockIndex const& index) const {
            return doAt(*this, index);
        }

        [[nodiscard]]
        Iterator begin()
            requires (isMut_)
        {
            return Iterator{ *data_ };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ *data_ };
        }

        [[nodiscard]]
        bool contains(BlockIndex const& index) const {
            return data_->solverIndices().contains(index);
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }

        [[nodiscard]]
        std::optional<BlockReference<true>> find(BlockIndex const& index)
            requires (isMut_)
        {
            return doFind(*this, index);
        }

        [[nodiscard]]
        std::optional<BlockReference<false>> find(BlockIndex const& index) const {
            return doFind(*this, index);
        }

        [[nodiscard]]
        std::size_t size() const {
            return data_->solverIndices().size();
        }
    private:
        [[nodiscard]]
        static auto doAt(meta::cCvRefOf<Blocks> auto&& self, BlockIndex const& blockId) {
            using Result = decltype(self.at(blockId));
            auto it = self.data_->solverIndices().find(blockId);
            if (it == self.data_->solverIndices().end()) {
                std::stringstream msg;
                msg << "Structure (id=" << self.data_->index() << ") does not contain the block at " << blockId << '.';
                throw std::out_of_range(msg.str());
            }
            return Result{ self.data_->sceneData(), blockId };
        }

        [[nodiscard]]
        static auto doFind(meta::cCvRefOf<Blocks> auto&& self, BlockIndex const& blockId) -> decltype(self.find(blockId)) {
            using Result = decltype(self.find(blockId));
            using Value = Result::value_type;
            auto it = self.data_->solverIndices().find(blockId);
            if (it == self.data_->solverIndices().end()) {
                return {};
            } else {
                return Value{ self.data_->sceneData(), blockId };
            }
        }

        PropPtr<StructureData> data_;
    };
}
