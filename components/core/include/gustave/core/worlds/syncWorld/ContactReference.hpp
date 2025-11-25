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

#include <sstream>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/model/Stress.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/forwardDecls.hpp>
#include <gustave/core/worlds/syncWorld/BlockReference.hpp>
#include <gustave/core/worlds/syncWorld/StructureReference.hpp>

namespace gustave::core::worlds::syncWorld {
    template<cfg::cLibConfig auto libCfg>
    class ContactReference {
    private:
        using WorldData = detail::WorldData<libCfg>;
        using SceneContact = WorldData::Scene::template ContactReference<false>;
        using StructureState = WorldData::StructureState;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Real = cfg::Real<libCfg, unit>;

        template<cfg::cUnitOf<libCfg> auto unit>
        using Vector3 = cfg::Vector3<libCfg, unit>;

        static constexpr auto u = cfg::units(libCfg);
    public:
        using BlockReference = syncWorld::BlockReference<libCfg>;
        using ContactIndex = WorldData::Scene::ContactIndex;
        using ForceStress = model::ForceStress<libCfg>;
        using PressureStress = SceneContact::PressureStress;
        using NormalizedVector3 = cfg::NormalizedVector3<libCfg>;
        using StressRatio = model::StressRatio<libCfg>;
        using StructureReference = syncWorld::StructureReference<libCfg>;

        [[nodiscard]]
        explicit ContactReference(WorldData const& world, ContactIndex const& index)
            : sceneContact_{ world.scene.contacts().find(index) }
            , world_{ &world }
        {}

        [[nodiscard]]
        explicit ContactReference(utils::NoInit NO_INIT)
            : sceneContact_{ NO_INIT }
            , world_{ nullptr }
        {}

        [[nodiscard]]
        Real<u.area> area() const {
            return sceneContact_.area();
        }

        [[nodiscard]]
        ForceStress forceStress() const {
            Vector3<u.force> const vector = forceVector();
            NormalizedVector3 const norm = normal();
            Real<u.force> const normalCoord = vector.dot(norm);
            Real<u.force> compression = 0.f * u.force;
            Real<u.force> tensile = 0.f * u.force;
            if (normalCoord < 0.f * u.force) {
                compression = -normalCoord;
            } else {
                tensile = normalCoord;
            }
            Real<u.force> shear = (vector - normalCoord * norm).norm();
            return ForceStress{ compression, shear, tensile };
        }

        [[nodiscard]]
        Vector3<u.force> forceVector() const {
            auto const structure = sceneContact_.structure();
            return structure.userData().solution().contacts().at(sceneContact_.solverIndex()).forceVector();
        }

        [[nodiscard]]
        ContactIndex const& index() const {
            return sceneContact_.index();
        }

        [[nodiscard]]
        std::out_of_range invalidError() {
            return sceneContact_.invalidError();
        }

        [[nodiscard]]
        bool isSolved() const {
            if (sceneContact_.isValid()) {
                return sceneContact_.structure().userData().state() == StructureState::Solved;
            } else {
                return false;
            }
        }

        [[nodiscard]]
        bool isValid() const {
            return sceneContact_.isValid();
        }

        [[nodiscard]]
        BlockReference localBlock() const {
            return BlockReference{ *world_, sceneContact_.localBlock().index() };
        }

        [[nodiscard]]
        PressureStress maxPressureStress() const {
            return sceneContact_.maxPressureStress();
        }

        [[nodiscard]]
        NormalizedVector3 normal() const {
            return sceneContact_.normal();
        }

        [[nodiscard]]
        ContactReference opposite() const {
            std::optional<ContactIndex> oppositeId = sceneContact_.index().opposite();
            if (!oppositeId) {
                std::stringstream msg;
                msg << "Invalid contact index: " << sceneContact_.index() << ".";
                throw std::out_of_range(msg.str());
            }
            return ContactReference{ *world_, *oppositeId };
        }

        [[nodiscard]]
        BlockReference otherBlock() const {
            return BlockReference{ *world_, sceneContact_.otherBlock().index() };
        }

        [[nodiscard]]
        PressureStress pressureStress() const {
            return forceStress() / sceneContact_.area();
        }

        [[nodiscard]]
        StressRatio stressRatio() const {
            return forceStress() / (sceneContact_.maxPressureStress() * sceneContact_.area());
        }

        [[nodiscard]]
        StructureReference structure() const {
            auto sceneStructure = sceneContact_.structure();
            return StructureReference{ *world_, sceneStructure.index() };
        }

        [[nodiscard]]
        bool operator==(ContactReference const&) const = default;
    private:
        SceneContact sceneContact_;
        WorldData const* world_;
    };
}
