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

#include <optional>
#include <stdexcept>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/worlds/syncWorld/detail/StructureData.hpp>
#include <gustave/core/worlds/syncWorld/BlockReference.hpp>
#include <gustave/core/worlds/syncWorld/ContactReference.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::worlds::syncWorld {
    template<cfg::cLibConfig auto libCfg>
    class BlockReference;

    template<cfg::cLibConfig auto libCfg>
    class ContactReference;

    template<cfg::cLibConfig auto libCfg>
    class StructureReference {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using StructureData = detail::StructureData<libCfg>;
        using WorldData = detail::WorldData<libCfg>;

        using SceneContactReference = typename WorldData::Scene::ContactReference;
        using SceneStructureReference = WorldData::Scene::StructureReference;
    public:
        using BlockIndex = typename WorldData::Scene::BlockIndex;
        using BlockReference = syncWorld::BlockReference<libCfg>;
        using ContactIndex = typename WorldData::Scene::ContactIndex;
        using ContactReference = syncWorld::ContactReference<libCfg>;
        using State = typename StructureData::State;
        using StructureIndex = WorldData::Scene::StructureIndex;

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
                    , value_{ utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(Blocks const& blocks)
                    : blocks_{ &blocks }
                    , sceneIterator_{ blocks.sceneBlocks_.begin() }
                    , value_{ utils::NO_INIT }
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
            using Iterator = utils::ForwardIterator<Enumerator>;

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
            constexpr utils::EndIterator end() const {
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

        class Contacts {
        public:
            [[nodiscard]]
            explicit Contacts(StructureData const& structure)
                : structure_{ &structure }
            {}

            [[nodiscard]]
            ContactReference at(ContactIndex const& index) const {
                SceneContactReference sceneContact = structure_->sceneStructure().contacts().at(index);
                return ContactReference{ structure_->world(), sceneContact.index() };
            }
        private:
            StructureData const* structure_;
        };

        class Links {
        private:
            using SceneStructLinks = typename WorldData::Scene::StructureReference::Links;
            using SceneIterator = typename SceneStructLinks::Iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : links_{ nullptr }
                    , sceneIterator_{}
                    , value_{ utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(Links const& links)
                    : links_{ &links }
                    , sceneIterator_{ links.sceneLinks_.begin() }
                    , value_{ utils::NO_INIT }
                {
                    updateValue();
                }

                [[nodiscard]]
                bool isEnd() const {
                    return sceneIterator_ == links_->sceneLinks_.end();
                }

                [[nodiscard]]
                ContactReference const& operator*() const {
                    return value_;
                }

                void operator++() {
                    ++sceneIterator_;
                    updateValue();
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return sceneIterator_ == other.sceneIterator_;
                }
            private:
                void updateValue() {
                    if (!isEnd()) {
                        value_ = ContactReference{ links_->structure_->world(), sceneIterator_->index() };
                    }
                }

                Links const* links_;
                SceneIterator sceneIterator_;
                ContactReference value_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Links(StructureData const& structure)
                : structure_{ &structure }
                , sceneLinks_{ structure.sceneStructure().links() }
            {}

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *this };
            }

            [[nodiscard]]
            utils::EndIterator end() const {
                return {};
            }
        private:
            StructureData const* structure_;
            SceneStructLinks sceneLinks_;
        };

        [[nodiscard]]
        explicit StructureReference(std::shared_ptr<StructureData const> data)
            : data_{ std::move(data) }
        {}

        [[nodiscard]]
        explicit StructureReference(utils::NoInit)
            : data_{ nullptr }
        {}

        [[nodiscard]]
        Blocks blocks() const {
            if (!isValid()) {
                throw invalidError();
            }
            return Blocks{ *data_ };
        }

        [[nodiscard]]
        Contacts contacts() const {
            if (!isValid()) {
                throw invalidError();
            }
            return Contacts{ *data_ };
        }

        [[nodiscard]]
        std::optional<Vector3<u.force>> forceVector(BlockIndex const& to, BlockIndex const& from) const {
            if (!isSolved()) {
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
        StructureIndex index() const {
            if (data_ == nullptr) {
                throw invalidError();
            }
            return data_->sceneStructure().index();
        }

        [[nodiscard]]
        std::out_of_range invalidError() const {
            if (data_ == nullptr) {
                return SceneStructureReference{ utils::NO_INIT }.invalidError();
            } else {
                return data_->sceneStructure().invalidError();
            }
        }

        [[nodiscard]]
        bool isSolved() const {
            return data_ != nullptr && data_->state() == State::Solved;
        }

        [[nodiscard]]
        bool isValid() const {
            return data_!= nullptr && data_->state() != State::Invalid;
        }

        [[nodiscard]]
        Links links() const {
            if (!isValid()) {
                throw invalidError();
            }
            return Links{ *data_ };
        }

        [[nodiscard]]
        State state() const {
            if (data_ == nullptr) {
                return State::Invalid;
            }
            return data_->state();
        }

        [[nodiscard]]
        bool operator==(StructureReference const&) const = default;
    private:
        std::shared_ptr<StructureData const> data_;
    };
}
