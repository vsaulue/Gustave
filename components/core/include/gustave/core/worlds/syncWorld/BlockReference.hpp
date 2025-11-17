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
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/worlds/syncWorld/detail/StructureData.hpp>
#include <gustave/core/worlds/syncWorld/ContactReference.hpp>
#include <gustave/core/worlds/syncWorld/StructureReference.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::worlds::syncWorld {
    template<cfg::cLibConfig auto libCfg>
    class ContactReference;

    template<cfg::cLibConfig auto libCfg>
    class StructureReference;

    template<cfg::cLibConfig auto libCfg>
    class BlockReference {
    private:
        static constexpr auto u = cfg::units(libCfg);

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        using StructureData = detail::StructureData<libCfg>;
        using WorldData = detail::WorldData<libCfg>;
        using Scene = typename WorldData::Scene;

        using SceneBlock = Scene::template BlockReference<false>;
    public:
        using BlockIndex = typename WorldData::Scene::BlockIndex;
        using ContactReference = syncWorld::ContactReference<libCfg>;
        using PressureStress = model::PressureStress<libCfg>;
        using StressRatio = model::StressRatio<libCfg>;
        using StructureReference = syncWorld::StructureReference<libCfg>;

        class Contacts {
        private:
            using SceneContacts = SceneBlock::template Contacts<false>;

            class Enumerator {
            private:
                using SceneIterator = typename SceneContacts::Iterator;
            public:
                [[nodiscard]]
                Enumerator()
                    : contacts_{ nullptr }
                    , sceneIt_{}
                {}

                [[nodiscard]]
                explicit Enumerator(Contacts const& contacts)
                    : contacts_{ &contacts }
                    , sceneIt_{ contacts.sceneContacts_.begin() }
                {}

                [[nodiscard]]
                bool isEnd() const {
                    return sceneIt_ == contacts_->sceneContacts_.end();
                }

                [[nodiscard]]
                ContactReference operator*() const {
                    return ContactReference{ *contacts_->world_, (*sceneIt_).index() };
                }

                void operator++() {
                    ++sceneIt_;
                }

                [[nodiscard]]
                bool operator==(Enumerator const& other) const {
                    return sceneIt_ == other.sceneIt_;
                }
            private:
                Contacts const* contacts_;
                SceneIterator sceneIt_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Contacts(BlockReference const& block)
                : sceneContacts_{ block.sceneBlock().contacts() }
                , world_{ block.world_ }
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
            SceneContacts sceneContacts_;
            WorldData const* world_;
        };

        class Neighbour {
        public:
            [[nodiscard]]
            explicit Neighbour(BlockReference const& block)
                : block_{ block }
            {}

            [[nodiscard]]
            explicit Neighbour(utils::NoInit NO_INIT)
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

        class Structures {
        private:
            using SceneStructures = typename SceneBlock::template Structures<false>;
            using SceneIterator = typename SceneStructures::ConstIterator;

            class Enumerator {
            public:
                [[nodiscard]]
                Enumerator()
                    : structures_{ nullptr }
                    , sceneIterator_{}
                    , value_{ utils::NO_INIT }
                {}

                [[nodiscard]]
                explicit Enumerator(Structures const& structures)
                    : structures_{ &structures }
                    , sceneIterator_{ structures.sceneStructures_.begin() }
                    , value_{ utils::NO_INIT }
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
                        value_ = StructureReference{ *structures_->world_, sceneIterator_->index() };
                    }
                }

                Structures const* structures_;
                SceneIterator sceneIterator_;
                StructureReference value_;
            };
        public:
            using Iterator = utils::ForwardIterator<Enumerator>;

            [[nodiscard]]
            explicit Structures(BlockReference const& block)
                : world_{ block.world_ }
                , sceneStructures_{ block.sceneBlock().structures() }
            {}

            [[nodiscard]]
            StructureReference operator[](std::size_t index) const {
                return StructureReference{ *world_, sceneStructures_[index].index() };
            }

            [[nodiscard]]
            Iterator begin() const {
                return Iterator{ *this };
            }

            [[nodiscard]]
            constexpr utils::EndIterator end() const {
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
        explicit BlockReference(utils::NoInit NO_INIT)
            : world_{ nullptr }
            , index_{ NO_INIT }
        {}

        [[nodiscard]]
        Vector3<u.length> const& blockSize() const
            requires requires (SceneBlock const& sb) { sb.blockSize(); }
        {
            return sceneBlock().blockSize();
        }

        [[nodiscard]]
        Contacts contacts() const {
            return Contacts{ *this };
        }

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
        PressureStress const& maxPressureStress() const {
            return sceneBlock().maxPressureStress();
        }

        [[nodiscard]]
        decltype(auto) position() const {
            return sceneBlock().position();
        }

        [[nodiscard]]
        StressRatio stressRatio() const {
            PressureStress const stressMax = maxPressureStress();
            StressRatio result{ 0.f * u.one, 0.f * u.one, 0.f * u.one };
            for (auto const& contact : contacts()) {
                result.mergeMax(contact.pressureStress() / stressMax);
            }
            return result;
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
