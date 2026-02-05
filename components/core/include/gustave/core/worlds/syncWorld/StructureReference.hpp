/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2026 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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
#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/forwardDecls.hpp>
#include <gustave/core/worlds/syncWorld/BlockReference.hpp>
#include <gustave/core/worlds/syncWorld/ContactReference.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::worlds::syncWorld {
    template<cfg::cLibConfig auto libCfg>
    class StructureReference {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using WorldData = detail::WorldData<libCfg>;

        using SceneStructureReference = WorldData::Scene::template StructureReference<false>;
    public:
        using BlockIndex = WorldData::Scene::BlockIndex;
        using BlockReference = syncWorld::BlockReference<libCfg>;
        using ContactIndex = WorldData::Scene::ContactIndex;
        using ContactReference = syncWorld::ContactReference<libCfg>;
        using State = WorldData::StructureState;
        using StructureIndex = WorldData::Scene::StructureIndex;

        class Blocks {
        private:
            using SceneStructBlocks = SceneStructureReference::template Blocks<false>;
            using SceneIterator = SceneStructBlocks::Iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : blocks_{ nullptr }
                    , sceneIterator_{}
                {}

                [[nodiscard]]
                explicit Enumerator(Blocks const& blocks)
                    : blocks_{ &blocks }
                    , sceneIterator_{ blocks.sceneBlocks_.begin() }
                {}

                [[nodiscard]]
                BlockReference operator*() const {
                    return BlockReference{ blocks_->structure_->world(), (*sceneIterator_).index() };
                }

                void operator++() {
                    ++sceneIterator_;
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
                Blocks const* blocks_;
                SceneIterator sceneIterator_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Blocks(StructureReference const& structure)
                : structure_{ &structure }
                , sceneBlocks_{ structure.sceneStructRef_.blocks() }
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
            constexpr std::default_sentinel_t end() const {
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
            StructureReference const* structure_;
            SceneStructBlocks sceneBlocks_;
        };

        class Contacts {
        public:
            [[nodiscard]]
            explicit Contacts(StructureReference const& structure)
                : structure_{ &structure }
            {}

            [[nodiscard]]
            ContactReference at(ContactIndex const& index) const {
                auto const sceneContact = structure_->sceneStructRef_.contacts().at(index);
                return ContactReference{ structure_->world(), sceneContact.index() };
            }
        private:
            StructureReference const* structure_;
        };

        class Links {
        private:
            using SceneStructLinks = SceneStructureReference::template Links<false>;
            using SceneIterator = SceneStructLinks::Iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : links_{ nullptr }
                    , sceneIterator_{}
                {}

                [[nodiscard]]
                explicit Enumerator(Links const& links)
                    : links_{ &links }
                    , sceneIterator_{ links.sceneLinks_.begin() }
                {}

                [[nodiscard]]
                bool isEnd() const {
                    return sceneIterator_ == links_->sceneLinks_.end();
                }

                [[nodiscard]]
                ContactReference operator*() const {
                    return ContactReference{ links_->structure_->world(), (*sceneIterator_).index() };
                }

                void operator++() {
                    ++sceneIterator_;
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return sceneIterator_ == other.sceneIterator_;
                }
            private:
                Links const* links_;
                SceneIterator sceneIterator_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Links(StructureReference const& structure)
                : structure_{ &structure }
                , sceneLinks_{ structure.sceneStructRef_.links() }
            {}

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *this };
            }

            [[nodiscard]]
            constexpr std::default_sentinel_t end() const {
                return {};
            }
        private:
            StructureReference const* structure_;
            SceneStructLinks sceneLinks_;
        };

        [[nodiscard]]
        explicit StructureReference(SceneStructureReference sceneStructRef)
            : sceneStructRef_{ std::move(sceneStructRef) }
        {}

        [[nodiscard]]
        explicit StructureReference(WorldData const& world, StructureIndex index)
            : sceneStructRef_{ world.scene.structures().find(index) }
        {}

        [[nodiscard]]
        explicit StructureReference(utils::NoInit)
            : sceneStructRef_{ nullptr }
        {}

        [[nodiscard]]
        Blocks blocks() const {
            if (!isValid()) {
                throw invalidError();
            }
            return Blocks{ *this };
        }

        [[nodiscard]]
        Contacts contacts() const {
            if (!isValid()) {
                throw invalidError();
            }
            return Contacts{ *this };
        }

        [[nodiscard]]
        std::optional<Vector3<u.force>> forceVector(BlockIndex const& to, BlockIndex const& from) const {
            if (!isSolved()) {
                return {};
            }
            auto const toIndex = sceneStructRef_.solverIndexOf(to);
            auto const fromIndex = sceneStructRef_.solverIndexOf(from);
            if (toIndex && fromIndex) {
                auto const& solution = sceneStructRef_.userData().solution();
                return solution.nodes().at(*toIndex).forceVectorFrom(*fromIndex);
            } else {
                return {};
            }
        }

        [[nodiscard]]
        StructureIndex index() const {
            return sceneStructRef_.index();
        }

        [[nodiscard]]
        std::out_of_range invalidError() const {
            return sceneStructRef_.invalidError();
        }

        [[nodiscard]]
        bool isSolved() const {
            return state() == State::Solved;
        }

        [[nodiscard]]
        bool isValid() const {
            return state() != State::Invalid;
        }

        [[nodiscard]]
        Links links() const {
            if (!isValid()) {
                throw invalidError();
            }
            return Links{ *this };
        }

        [[nodiscard]]
        State state() const {
            if (not sceneStructRef_.isValid()) {
                return State::Invalid;
            }
            return sceneStructRef_.userData().state();
        }

        [[nodiscard]]
        bool operator==(StructureReference const&) const = default;
    private:
        [[nodiscard]]
        WorldData const& world() const {
            return sceneStructRef_.userData().world();
        }

        SceneStructureReference sceneStructRef_;
    };
}
