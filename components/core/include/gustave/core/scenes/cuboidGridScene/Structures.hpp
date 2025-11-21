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

#include <type_traits>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/StructureReference.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    namespace structures::detail {
        template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
        class Enumerator {
        private:
            using SceneStructures = cuboidGridScene::detail::SceneData<libCfg_, UD_>::Structures;
            using DataIterator = utils::PropIterator<isMut_, SceneStructures>;

            template<typename T>
            using Prop = utils::Prop<isMut_, T>;
        public:
            using Value = StructureReference<libCfg_, UD_, isMut_>;

            [[nodiscard]]
            Enumerator()
                :  dataIterator_{}
                , value_{ utils::NO_INIT }
            {}

            [[nodiscard]]
            Enumerator(Enumerator const& other)
                // needed for MSVC, which doesn't auto-generate
                : dataIterator_{ other.dataIterator_ }
                , value_{ other.value_ }
            {}

            [[nodiscard]]
            explicit Enumerator(Prop<SceneStructures>& structures)
                : dataIterator_{ structures.begin() }
                , value_{ utils::NO_INIT }
            {
                updateValue();
            }

            Enumerator& operator=(Enumerator const& other) {
                // needed for MSVC, which doesn't auto-generate
                dataIterator_ = other.dataIterator_;
                value_ = other.value;
            }

            [[nodiscard]]
            bool isEnd() const {
                return dataIterator_ == std::default_sentinel;
            }

            void operator++() {
                ++dataIterator_;
                updateValue();
            }

            [[nodiscard]]
            Value& operator*() const {
                return value_;
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return dataIterator_ == other.dataIterator_;
            }
        private:
            void updateValue() {
                if (!isEnd()) {
                    value_ = Value{ *dataIterator_ };
                }
            }

            DataIterator dataIterator_;
            mutable Value value_;
        };
    }

    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UD_, bool isMut_>
    class Structures {
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        using SceneData = detail::SceneData<libCfg, UD_>;

        template<bool mut>
        using Enumerator = structures::detail::Enumerator<libCfg, UD_, mut>;
    public:
        template<bool mut>
        using StructureReference = cuboidGridScene::StructureReference<libCfg, UD_, mut>;
        using StructureIndex = StructureReference<false>::StructureIndex;

        using Iterator = utils::ForwardIterator<Enumerator<isMut_>>;
        using ConstIterator = utils::ForwardIterator<Enumerator<false>>;

        [[nodiscard]]
        explicit Structures(Prop<SceneData>& data)
            : data_{ &data }
        {}

        [[nodiscard]]
        Iterator begin()
            requires (isMut_)
        {
            return Iterator{ data_->structures };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ data_->structures };
        }

        [[nodiscard]]
        StructureReference<true> at(StructureIndex index)
            requires (isMut_)
        {
            return doAt(*this, index);
        }

        [[nodiscard]]
        StructureReference<false> at(StructureIndex index) const {
            return doAt(*this, index);
        }

        [[nodiscard]]
        StructureReference<true> find(StructureIndex index)
            requires (isMut_)
        {
            return StructureReference<true>{ *data_, index };
        }

        [[nodiscard]]
        StructureReference<false> find(StructureIndex index) const {
            return StructureReference<false>{ *data_, index };
        }

        [[nodiscard]]
        bool contains(StructureReference<false> const& structure) const {
            return data_->structures.contains(structure.index());
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }

        [[nodiscard]]
        std::size_t size() const {
            return data_->structures.size();
        }
    private:
        [[nodiscard]]
        static auto doAt(meta::cCvRefOf<Structures> auto&& self, StructureIndex index) {
            using Result = decltype(self.at(index));
            auto result = Result{ *self.data_, index};
            if (!result.isValid()) {
                throw result.invalidError();
            }
            return result;
        }

        PropPtr<SceneData> data_;
    };
}
