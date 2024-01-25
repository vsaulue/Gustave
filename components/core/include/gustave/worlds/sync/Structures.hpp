/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/worlds/sync/detail/WorldData.hpp>
#include <gustave/worlds/sync/StructureReference.hpp>

namespace Gustave::Worlds::Sync {
    template<Cfg::cLibConfig auto cfg>
    class Structures {
    public:
        using StructureReference = Sync::StructureReference<cfg>;
    private:
        using WorldData = detail::WorldData<cfg>;

        class Enumerator {
        private:
            using DataIterator = typename WorldData::Structures::const_iterator;
        public:
            [[nodiscard]]
            Enumerator()
                : world_{ nullptr }
                , dataIterator_{}
                , value_{ Utils::NO_INIT }
            {}

            [[nodiscard]]
            explicit Enumerator(WorldData const& world)
                : world_{ &world }
                , dataIterator_{ world.structures.begin() }
                , value_{ Utils::NO_INIT }
            {
                updateValue();
            }

            [[nodiscard]]
            bool isEnd() const {
                return dataIterator_ == world_->structures.end();
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
                    value_ = StructureReference{ dataIterator_->second };
                }
            }
            WorldData const* world_;
            DataIterator dataIterator_;
            StructureReference value_;
        };
    public:
        using Iterator = Utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit Structures(WorldData const& world)
            : world_{ &world }
        {}

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *world_ };
        }

        [[nodiscard]]
        constexpr Utils::EndIterator end() const {
            return {};
        }

        [[nodiscard]]
        std::size_t size() const {
            return world_->structures.size();
        }
    private:
        WorldData const* world_;
    };
}
