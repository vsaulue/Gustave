/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022,2023 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <gustave/scenes/cuboidGrid/BlockReference.hpp>
#include <gustave/scenes/cuboidGrid/detail/SceneBlocks.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class Blocks {
    private:
        using BlockReference = CuboidGrid::BlockReference<cfg>;
        using SceneBlocks = detail::SceneBlocks<cfg>;

        using DataIterator = typename SceneBlocks::BlockMap::const_iterator;
    public:
        class EndIterator {
        public:
            [[nodiscard]]
            constexpr EndIterator() = default;
        };

        class Iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = BlockReference;

            [[nodiscard]]
            Iterator()
                : blocksData_{ nullptr }
                , dataIterator_{}
                , value_{ Utils::NO_INIT }
            {}

            [[nodiscard]]
            explicit Iterator(SceneBlocks const& sceneBlocks)
                : blocksData_{ &sceneBlocks }
                , dataIterator_{ sceneBlocks.begin() }
                , value_{ Utils::NO_INIT }
            {
                updateValue();
            }

            Iterator& operator++() {
                ++dataIterator_;
                updateValue();
                return *this;
            }

            Iterator operator++(int) {
                Iterator result = *this;
                ++*this;
                return result;
            }

            [[nodiscard]]
            BlockReference const& operator*() const {
                return value_;
            }

            [[nodiscard]]
            BlockReference const& operator->() const {
                return &value_;
            }

            [[nodiscard]]
            bool operator==(EndIterator const&) const {
                return isEnd();
            }

            [[nodiscard]]
            bool operator==(Iterator const&) const = default;
        private:
            [[nodiscard]]
            bool isEnd() const {
                return dataIterator_ == blocksData_->end();
            }

            void updateValue() {
                if (!isEnd()) {
                    value_ = BlockReference{ *blocksData_, dataIterator_->first };
                }
            }

            SceneBlocks const* blocksData_;
            DataIterator dataIterator_;
            BlockReference value_;
        };

        [[nodiscard]]
        explicit Blocks(SceneBlocks const& sceneBlocks)
            : blocksData_{ &sceneBlocks }
        {}

        [[nodiscard]]
        BlockReference at(BlockPosition const& position) const {
            BlockReference result{ *blocksData_, position };
            if (!result.isValid()) {
                std::stringstream msg;
                msg << "No block at position " << position << ".";
                throw std::out_of_range(msg.str());
            }
            return result;
        }

        [[nodiscard]]
        BlockReference find(BlockPosition const& position) const {
            return BlockReference{ *blocksData_, position };
        }

        [[nodiscard]]
        std::size_t size() const {
            return blocksData_->size();
        }

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *blocksData_ };
        }

        [[nodiscard]]
        EndIterator end() const {
            return {};
        }
    private:
        SceneBlocks const* blocksData_;
    };
}
