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

#include <optional>
#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/worlds/sync/BlockReference.hpp>
#include <gustave/worlds/sync/detail/StructureData.hpp>

namespace Gustave::Worlds::Sync {
    template<Cfg::cLibConfig auto cfg>
    class BlockReference;

    template<Cfg::cLibConfig auto cfg>
    class StructureReference {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Vector3 = Cfg::Vector3<cfg, unit>;

        using StructureData = detail::StructureData<cfg>;
        using WorldData = detail::WorldData<cfg>;
    public:
        using BlockIndex = typename WorldData::Scene::BlockIndex;
        using BlockReference = Sync::BlockReference<cfg>;
        using State = typename StructureData::State;

        class Blocks {
        private:
            using SceneStructBlocks = typename WorldData::Scene::StructureReference::Blocks;
            using SceneIterator = typename SceneStructBlocks::Iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : blocks_{ nullptr }
                    , sceneIterator_{}
                    , value_{ Utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(Blocks const& blocks)
                    : blocks_{ &blocks }
                    , sceneIterator_{ blocks.sceneBlocks_.begin() }
                    , value_{ Utils::NO_INIT }
                {
                    updateValue();
                }

                [[nodiscard]]
                BlockReference const& operator*() const {
                    return value_;
                }

                void operator++() {
                    ++sceneIterator_;
                    updateValue();
                }

                [[nodiscard]]
                bool isEnd() const {
                    return sceneIterator_ == blocks_->sceneBlocks_.end();
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return sceneIterator_ == other.sceneIterator_;
                }
            private:
                void updateValue() {
                    if (!isEnd()) {
                        value_ = BlockReference{ blocks_->structure_->world(), sceneIterator_->index() };
                    }
                }

                Blocks const* blocks_;
                SceneIterator sceneIterator_;
                BlockReference value_;
            };
        public:
            using Iterator = Utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Blocks(StructureData const& structure)
                : structure_{ &structure }
                , sceneBlocks_{ structure.sceneStructure().blocks() }
            {}

            [[nodiscard]]
            BlockReference at(BlockIndex const& index) const {
                if (!sceneBlocks_.contains(index)) {
                    std::stringstream msg;
                    msg << "Structure does not contain the block at " << index << '.';
                    throw std::out_of_range(msg.str());
                }
                return BlockReference{ structure_->world(), index };
            }

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *this };
            }

            [[nodiscard]]
            bool contains(BlockIndex const& index) const {
                return sceneBlocks_.contains(index);
            }

            [[nodiscard]]
            constexpr Utils::EndIterator end() const {
                return {};
            }

            [[nodiscard]]
            std::optional<BlockReference> find(BlockIndex const& index) const {
                if (sceneBlocks_.contains(index)) {
                    return BlockReference{ structure_->world(), index };
                } else {
                    return {};
                }
            }

            [[nodiscard]]
            std::size_t size() const {
                return sceneBlocks_.size();
            }
        private:
            StructureData const* structure_;
            SceneStructBlocks sceneBlocks_;
        };

        [[nodiscard]]
        explicit StructureReference(std::shared_ptr<StructureData const> data)
            : data_{ std::move(data) }
        {
            assert(data_);
        }

        [[nodiscard]]
        explicit StructureReference(Utils::NoInit)
            : data_{ nullptr }
        {}

        [[nodiscard]]
        Blocks blocks() const {
            if (data_->state() == State::Invalid) {
                throw std::logic_error("This structure has been invalidated.");
            }
            return Blocks{ *data_ };
        }

        [[nodiscard]]
        std::optional<Vector3<u.force>> forceVector(BlockIndex const& to, BlockIndex const& from) const {
            if (data_->state() != State::Solved) {
                return {};
            }
            auto const toIndex = data_->sceneStructure().solverIndexOf(to);
            auto const fromIndex = data_->sceneStructure().solverIndexOf(from);
            if (toIndex && fromIndex) {
                return data_->solution().nodes().at(*toIndex).forceVectorFrom(*fromIndex);
            } else {
                return {};
            }
        }

        [[nodiscard]]
        bool isValid() const {
            return data_->state() != State::Invalid;
        }

        [[nodiscard]]
        State state() const {
            return data_->state();
        }

        [[nodiscard]]
        bool operator==(StructureReference const&) const = default;
    private:
        std::shared_ptr<StructureData const> data_;
    };
}
