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
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg>
    class Structures {
    public:
        using StructureReference = cuboidGridScene::StructureReference<libCfg>;
        using StructureIndex = StructureReference::StructureIndex;
    private:
        using SceneData = detail::SceneData<libCfg>;

        class Enumerator {
        public:
            [[nodiscard]]
            Enumerator()
                : data_{ nullptr }
                , dataIterator_{}
                , value_{ utils::NO_INIT }
            {}

            [[nodiscard]]
            explicit Enumerator(SceneData const& data)
                : data_{ &data }
                , dataIterator_{ data.structures.begin() }
                , value_{ utils::NO_INIT }
            {
                updateValue();
            }

            [[nodiscard]]
            bool isEnd() const {
                return dataIterator_ == data_->structures.end();
            }

            void operator++() {
                ++dataIterator_;
                updateValue();
            }

            [[nodiscard]]
            StructureReference const& operator*() const {
                return value_;
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return dataIterator_ == other.dataIterator_;
            }
        private:
            void updateValue() {
                if (!isEnd()) {
                    value_ = StructureReference{ *dataIterator_ };
                }
            }

            SceneData const* data_;
            typename SceneData::Structures::const_iterator dataIterator_;
            StructureReference value_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit Structures(SceneData const& data)
            : data_{ &data }
        {}

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *data_ };
        }

        [[nodiscard]]
        StructureReference at(StructureIndex const& index) const {
            auto result = StructureReference{ *data_, index };
            if (!result.isValid()) {
                throw result.invalidError();
            }
            return result;
        }

        [[nodiscard]]
        StructureReference find(StructureIndex const& index) const {
            return StructureReference{ *data_, index };
        }

        [[nodiscard]]
        bool contains(StructureReference const& structure) const {
            return data_->structures.contains(structure.index());
        }

        [[nodiscard]]
        constexpr utils::EndIterator end() const {
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
