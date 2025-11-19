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

#include <gustave/core/scenes/cuboidGridScene/structureReference/Contacts.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <TestHelpers.hpp>
#include <SceneUserData.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, SceneUserData>;

template<bool mut>
using Contacts = cuboid::structureReference::Contacts<libCfg, SceneUserData, mut>;

using BlockIndex = SceneData::Blocks::BlockIndex;
using ContactIndex = Contacts<false>::ContactIndex;
using Direction = ContactIndex::Direction;
using StructureData = SceneData::Structures::StructureData;
using Transaction = SceneUpdater::Transaction;

TEST_CASE("core::scenes::cuboidGridScene::structureReference::Contacts") {
    auto const blockSize = vector3(1.f, 1.f, 1.f, u.length);
    SceneData scene{ blockSize };

    Transaction t;
    t.addBlock({ {1,0,0}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,0,0}, concrete_20m, 2000.f * u.mass, true });
    t.addBlock({ {3,0,0}, concrete_20m, 3000.f * u.mass, false });
    t.addBlock({ {4,0,0}, concrete_20m, 4000.f * u.mass, false });
    SceneUpdater{ scene }.runTransaction(t);

    auto structDataOf = [&](BlockIndex const& index) -> StructureData& {
        return scene.structures.at(scene.blocks.at(index).structureId());
    };

    auto mContacts300 = Contacts<true>{ structDataOf({3,0,0}) };
    auto const& cmContacts300 = mContacts300;
    auto iContacts300 = Contacts<false>{ structDataOf({3,0,0}) };

    SECTION(".at()") {
        auto runValidTest = [&](auto&& contacts, bool expectedConst) {
            auto contact = contacts.at(ContactIndex{ {3,0,0}, Direction::plusX() });
            REQUIRE(contact.isValid());
            CHECK(contact.localBlock().mass() == 3000.f * u.mass);
            CHECK(contact.otherBlock().mass() == 4000.f * u.mass);
            CHECK(expectedConst == contact.structure().userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mContacts300, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmContacts300, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iContacts300, true);
        }

        SECTION("// invalid source") {
            CHECK_THROWS_AS(iContacts300.at(ContactIndex{ {0,0,0}, Direction::plusX() }), std::out_of_range);
        }

        SECTION("// invalid other") {
            CHECK_THROWS_AS(iContacts300.at(ContactIndex{ {1,0,0}, Direction::plusY() }), std::out_of_range);
        }

        SECTION("// invalid structure") {
            CHECK_THROWS_AS(iContacts300.at(ContactIndex{ {1,0,0}, Direction::plusX() }), std::out_of_range);
        }
    }
}
