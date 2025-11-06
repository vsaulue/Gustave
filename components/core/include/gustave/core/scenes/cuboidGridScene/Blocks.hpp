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
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UserData_>
    class Blocks {
    public:
        using BlockReference = cuboidGridScene::BlockReference<libCfg, UserData_, false>;
        using BlockIndex = typename BlockReference::BlockIndex;
    private:
        using SceneData = detail::SceneData<libCfg, UserData_>;

        using DataIterator = typename SceneData::Blocks::BlockMap::const_iterator;

        class Enumerator {
        public:
            [[nodiscard]]
            Enumerator()
                : sceneData_{ nullptr }
                , dataIterator_{}
                , value_{ utils::NO_INIT }
            {}

            [[nodiscard]]
            explicit Enumerator(SceneData const& sceneData)
                : sceneData_{ &sceneData }
                , dataIterator_{ sceneData.blocks.begin() }
                , value_{ utils::NO_INIT }
            {
                updateValue();
            }

            [[nodiscard]]
            bool isEnd() const {
                return dataIterator_ == sceneData_->blocks.end();
            }

            void operator++() {
                ++dataIterator_;
                updateValue();
            }

            [[nodiscard]]
            BlockReference const& operator*() const {
                return value_;
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return dataIterator_ == other.dataIterator_;
            }
        private:
            void updateValue() {
                if (!isEnd()) {
                    value_ = BlockReference{ *sceneData_, dataIterator_->first };
                }
            }

            SceneData const* sceneData_;
            DataIterator dataIterator_;
            BlockReference value_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit Blocks(SceneData const& sceneData)
            : sceneData_{ &sceneData }
        {}

        [[nodiscard]]
        BlockReference at(BlockIndex const& index) const {
            BlockReference result{ *sceneData_, index };
            if (!result.isValid()) {
                std::stringstream msg;
                msg << "No block at index " << index << ".";
                throw std::out_of_range(msg.str());
            }
            return result;
        }

        [[nodiscard]]
        BlockReference find(BlockIndex const& index) const {
            return BlockReference{ *sceneData_, index };
        }

        [[nodiscard]]
        std::size_t size() const {
            return sceneData_->blocks.size();
        }

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *sceneData_ };
        }

        [[nodiscard]]
        constexpr utils::EndIterator end() const {
            return {};
        }
    private:
        SceneData const* sceneData_;
    };
}
