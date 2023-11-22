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
#include <gustave/scenes/cuboidGrid/detail/SceneData.hpp>
#include <gustave/scenes/cuboidGrid/detail/StructureData.hpp>
#include <gustave/scenes/cuboidGrid/StructureReference.hpp>
#include <gustave/utils/NoInit.hpp>

namespace Gustave::Scenes::CuboidGrid {
    template<Cfg::cLibConfig auto cfg>
    class Structures {
    private:
        using SceneData = detail::SceneData<cfg>;
        using StructureData = detail::StructureData<cfg>;
    public:
        using StructureReference = CuboidGrid::StructureReference<cfg>;

        class EndIterator {
        public:
            [[nodiscard]]
            constexpr EndIterator() = default;
        };

        class Iterator {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = StructureReference;

            [[nodiscard]]
            Iterator()
                : data_{ nullptr }
                , dataIterator_{}
                , value_{ Utils::NO_INIT }
            {}

            [[nodiscard]]
            explicit Iterator(SceneData const& data)
                : data_{ &data }
                , dataIterator_{ data.structures.begin() }
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
            StructureReference const& operator*() const {
                return value_;
            }

            [[nodiscard]]
            StructureReference const* operator->() const {
                return &value_;
            }

            [[nodiscard]]
            bool operator==(Iterator const&) const = default;

            [[nodiscard]]
            bool operator==(EndIterator const&) const {
                return isEnd();
            }
        private:
            [[nodiscard]]
            bool isEnd() const {
                return dataIterator_ == data_->structures.end();
            }

            void updateValue() {
                if (!isEnd()) {
                    value_ = StructureReference{ *dataIterator_ };
                }
            }

            SceneData const* data_;
            typename SceneData::Structures::const_iterator dataIterator_;
            StructureReference value_;
        };

        [[nodiscard]]
        explicit Structures(SceneData const& data)
            : data_{ &data }
        {}

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *data_ };
        }

        [[nodiscard]]
        bool contains(StructureReference const& structure) const {
            return data_->structures.contains(&detail::structureDataOf(structure));
        }

        [[nodiscard]]
        EndIterator end() const {
            return {};
        }

        [[nodiscard]]
        std::size_t size() const {
            return data_->structures.size();
        }
    private:
        SceneData const* data_;
    };
}
