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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/meta/Meta.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    namespace blocks::detail {
        template<cfg::cLibConfig auto cfg_, common::cSceneUserData UD_, bool isMut_>
        class Enumerator {
        private:
            template<typename T>
            using Prop = utils::Prop<isMut_, T>;

            using SceneData = cuboidGridScene::detail::SceneData<cfg_, UD_>;

            using DataIterator = SceneData::Blocks::BlockMap::const_iterator;
        public:
            using Value = BlockReference<cfg_, UD_, isMut_>;

            [[nodiscard]]
            Enumerator()
                : sceneData_{ nullptr }
                , dataIterator_{}
            {}

            [[nodiscard]]
            explicit Enumerator(Prop<SceneData>& sceneData)
                : sceneData_{ &sceneData }
                , dataIterator_{ sceneData.blocks.begin() }
            {}

            [[nodiscard]]
            bool isEnd() const {
                return dataIterator_ == sceneData_->blocks.end();
            }

            void operator++() {
                ++dataIterator_;
            }

            [[nodiscard]]
            Value operator*() const {
                return Value{ *sceneData_, dataIterator_->first };
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return dataIterator_ == other.dataIterator_;
            }
        private:
            Prop<SceneData>* sceneData_;
            DataIterator dataIterator_;
        };
    }

    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UserData_, bool isMut_>
    class Blocks {
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        using SceneData = detail::SceneData<libCfg, UserData_>;

        template<bool mut>
        using Enumerator = blocks::detail::Enumerator<libCfg, UserData_, mut>;
    public:
        template<bool mut>
        using BlockReference = cuboidGridScene::BlockReference<libCfg, UserData_, mut>;

        using BlockIndex = BlockReference<false>::BlockIndex;

        using Iterator = utils::ForwardIterator<Enumerator<isMut_>>;
        using ConstIterator = utils::ForwardIterator<Enumerator<false>>;

        [[nodiscard]]
        explicit Blocks(Prop<SceneData>& sceneData)
            : sceneData_{ &sceneData }
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
            return Iterator{ *sceneData_ };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ *sceneData_ };
        }

        [[nodiscard]]
        constexpr utils::EndIterator end() const {
            return {};
        }

        [[nodiscard]]
        BlockReference<true> find(BlockIndex const& index)
            requires (isMut_)
        {
            return BlockReference<true>{ *sceneData_, index };
        }

        [[nodiscard]]
        BlockReference<false> find(BlockIndex const& index) const {
            return BlockReference<false>{ *sceneData_, index };
        }

        [[nodiscard]]
        std::size_t size() const {
            return sceneData_->blocks.size();
        }
    private:
        [[nodiscard]]
        static auto doAt(meta::cCvRefOf<Blocks> auto&& self, BlockIndex const& index) {
            using Result = decltype(self.at(index));
            auto result = Result{ *self.sceneData_, index };
            if (!result.isValid()) {
                std::stringstream msg;
                msg << "No block at index " << index << ".";
                throw std::out_of_range(msg.str());
            }
            return result;
        }

        PropPtr<SceneData> sceneData_;
    };
}
