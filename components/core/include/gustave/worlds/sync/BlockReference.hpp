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
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/worlds/sync/detail/StructureData.hpp>
#include <gustave/worlds/sync/StructureReference.hpp>

namespace Gustave::Worlds::Sync {
    template<Cfg::cLibConfig auto cfg>
    class StructureReference;

    template<Cfg::cLibConfig auto cfg>
    class BlockReference {
    private:
        static constexpr auto u = Cfg::units(cfg);

        template<Cfg::cUnitOf<cfg> auto unit>
        using Real = Cfg::Real<cfg, unit>;

        using StructureData = detail::StructureData<cfg>;
        using WorldData = detail::WorldData<cfg>;
        using Scene = typename WorldData::Scene;

        using SceneBlock = typename Scene::BlockReference;
    public:
        using BlockIndex = typename WorldData::Scene::BlockIndex;
        using MaxStress = Model::MaxStress<cfg>;
        using StructureReference = Sync::StructureReference<cfg>;

        class Neighbour {
        public:
            [[nodiscard]]
            explicit Neighbour(BlockReference const& block)
                : block_{ block }
            {}

            [[nodiscard]]
            explicit Neighbour(Utils::NoInit NO_INIT)
                : block_{ NO_INIT }
            {}

            [[nodiscard]]
            BlockReference const& block() const {
                return block_;
            }

            [[nodiscard]]
            bool operator==(Neighbour const&) const = default;
        private:
            BlockReference block_;
        };

        class Neighbours {
        private:
            using SceneNeighbours = typename SceneBlock::Neighbours;
            using SceneIterator = typename SceneNeighbours::Iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : neighbours_{ nullptr }
                    , sceneIterator_{}
                    , value_{ Utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(Neighbours const& neighbours)
                    : neighbours_{ &neighbours }
                    , sceneIterator_{ neighbours.sceneNeighbours_.begin() }
                    , value_{ Utils::NO_INIT }
                {
                    updateValue();
                }

                void operator++() {
                    ++sceneIterator_;
                    updateValue();
                }

                [[nodiscard]]
                Neighbour const& operator*() const {
                    return value_;
                }

                [[nodiscard]]
                bool isEnd() const {
                    return sceneIterator_ == neighbours_->sceneNeighbours_.end();
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return sceneIterator_ == other.sceneIterator_;
                }
            private:
                void updateValue() {
                    if (!isEnd()) {
                        value_ = Neighbour{ BlockReference{ *neighbours_->world_, (*sceneIterator_).block().index() }};
                    }
                }

                Neighbours const* neighbours_;
                SceneIterator sceneIterator_;
                Neighbour value_;
            };
        public:
            using Iterator = Utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Neighbours(BlockReference const& block)
                : world_{ block.world_ }
                , sceneNeighbours_{ block.sceneBlock().neighbours() }
            {}

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *this };
            }

            [[nodiscard]]
            constexpr Utils::EndIterator end() const {
                return {};
            }
        private:
            WorldData const* world_;
            SceneNeighbours sceneNeighbours_;
        };

        class Structures {
        private:
            using SceneStructures = typename SceneBlock::Structures;
            using SceneIterator = typename SceneStructures::Iterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : structures_{ nullptr }
                    , sceneIterator_{}
                    , value_{ Utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(Structures const& structures)
                    : structures_{ &structures }
                    , sceneIterator_{ structures.sceneStructures_.begin() }
                    , value_{ Utils::NO_INIT }
                {
                    updateValue();
                }

                void operator++() {
                    ++sceneIterator_;
                    updateValue();
                }

                [[nodiscard]]
                StructureReference const& operator*() const {
                    return value_;
                }

                [[nodiscard]]
                bool isEnd() const {
                    return sceneIterator_ == structures_->sceneStructures_.end();
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return sceneIterator_ == other.sceneIterator_;
                }
            private:
                void updateValue() {
                    if (!isEnd()) {
                        auto const& structureData = structures_->world_->structures.at(*sceneIterator_);
                        value_ = StructureReference{ structureData };
                    }
                }

                Structures const* structures_;
                SceneIterator sceneIterator_;
                StructureReference value_;
            };
        public:
            using Iterator = Utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Structures(BlockReference const& block)
                : world_{ block.world_ }
                , sceneStructures_{ block.sceneBlock().structures() }
            {}

            [[nodiscard]]
            StructureReference operator[](std::size_t index) const {
                return StructureReference{ world_->structures.at(sceneStructures_[index]) };
            }

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *this };
            }

            [[nodiscard]]
            constexpr Utils::EndIterator end() const {
                return {};
            }

            [[nodiscard]]
            std::size_t size() const {
                return sceneStructures_.size();
            }
        private:
            WorldData const* world_;
            SceneStructures sceneStructures_;
        };

        [[nodiscard]]
        explicit BlockReference(WorldData const& world, BlockIndex const& index)
            : world_{ &world }
            , index_{ index }
        {}

        [[nodiscard]]
        explicit BlockReference(Utils::NoInit NO_INIT)
            : world_{ nullptr}
            , index_{ NO_INIT }
        {}

        [[nodiscard]]
        BlockIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        bool isFoundation() const {
            return sceneBlock().isFoundation();
        }

        [[nodiscard]]
        bool isValid() const {
            return world_->scene.blocks().find(index_).isValid();
        }

        [[nodiscard]]
        Real<u.mass> mass() const {
            return sceneBlock().mass();
        }

        [[nodiscard]]
        MaxStress const& maxStress() const {
            return sceneBlock().maxStress();
        }

        [[nodiscard]]
        Neighbours neighbours() const {
            return Neighbours{ *this };
        }

        [[nodiscard]]
        decltype(auto) position() const {
            return sceneBlock().position();
        }

        [[nodiscard]]
        Structures structures() const {
            return Structures{ *this };
        }

        [[nodiscard]]
        bool operator==(BlockReference const&) const = default;
    private:
        [[nodiscard]]
        SceneBlock sceneBlock() const {
            return world_->scene.blocks().at(index_);
        }

        WorldData const* world_;
        BlockIndex index_;
    };
}
