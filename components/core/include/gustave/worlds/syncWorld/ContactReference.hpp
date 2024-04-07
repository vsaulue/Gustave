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

#include <sstream>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/worlds/syncWorld/BlockReference.hpp>
#include <gustave/worlds/syncWorld/StructureReference.hpp>

namespace gustave::worlds::syncWorld {
    template<cfg::cLibConfig auto libCfg>
    class StructureReference;

    template<cfg::cLibConfig auto libCfg>
    class BlockReference;

    template<cfg::cLibConfig auto libCfg>
    class ContactReference {
    private:
        using WorldData = detail::WorldData<libCfg>;
        using SceneContact = typename WorldData::Scene::ContactReference;
        using SceneStructure = typename WorldData::Scene::StructureReference;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        static constexpr auto u = cfg::units(libCfg);
    public:
        using BlockReference = syncWorld::BlockReference<libCfg>;
        using ContactIndex = typename WorldData::Scene::ContactIndex;
        using MaxStress = typename SceneContact::MaxStress;
        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;
        using StructureReference = syncWorld::StructureReference<libCfg>;

        [[nodiscard]]
        explicit ContactReference(WorldData const& world, ContactIndex const& index)
            : world_{ &world }
            , index_{ index }
        {}

        [[nodiscard]]
        explicit ContactReference(utils::NoInit NO_INIT)
            : world_{ nullptr }
            , index_{ NO_INIT }
        {}

        [[nodiscard]]
        Real<u.area> area() const {
            return sceneContact().area();
        }

        [[nodiscard]]
        Vector3<u.force> forceVector() const {
            auto const sContact = sceneContact();
            auto const& structureData = *world_->structures.at(sContact.structure());
            assert(structureData.state() == WorldStructureState::Solved);
            return structureData.solution().contacts().at(sContact.solverIndex()).forceVector();
        }

        [[nodiscard]]
        ContactIndex const& index() const {
            return index_;
        }

        [[nodiscard]]
        std::string invalidMessage() const {
            return sceneContact().invalidMessage();
        }

        [[nodiscard]]
        bool isValid() const {
            return world_->scene.contacts().find(index_).isValid();
        }

        [[nodiscard]]
        BlockReference localBlock() const {
            return BlockReference{ *world_, sceneContact().localBlock().index() };
        }

        [[nodiscard]]
        MaxStress maxStress() const {
            return sceneContact().maxStress();
        }

        [[nodiscard]]
        NormalizedVector3 normal() const {
            return sceneContact().normal();
        }

        [[nodiscard]]
        ContactReference opposite() const {
            std::optional<ContactIndex> oppositeId = index_.opposite();
            if (!oppositeId) {
                std::stringstream msg;
                msg << "Invalid contact index: " << index_ << ".";
                throw std::out_of_range(msg.str());
            }
            return ContactReference{ *world_, *oppositeId };
        }

        [[nodiscard]]
        StructureReference structure() const {
            SceneStructure sceneStructure = sceneContact().structure();
            return StructureReference{ world_->structures.at(sceneStructure) };
        }

        [[nodiscard]]
        bool operator==(ContactReference const&) const = default;
    private:
        [[nodiscard]]
        SceneContact sceneContact() const {
            return world_->scene.contacts().at(index_);
        }

        WorldData const* world_;
        ContactIndex index_;
    };
}
