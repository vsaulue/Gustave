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

#include <catch2/catch_test_macros.hpp>

#include <gustave/core/scenes/cuboidGridScene/Contacts.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

template<bool mut>
using Contacts = gustave::core::scenes::cuboidGridScene::Contacts<libCfg, SceneUserData, mut>;

template<bool mut>
using ContactReference = Contacts<false>::ContactReference<mut>;

using SceneData = gustave::core::scenes::cuboidGridScene::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = gustave::core::scenes::cuboidGridScene::detail::SceneUpdater<libCfg, SceneUserData>;

using ContactIndex = Contacts<false>::ContactIndex;
using Direction = ContactIndex::Direction;
using Transaction = SceneUpdater::Transaction;

TEST_CASE("core::scenes::cuboidGridScene::Contacts") {
    SceneData scene{ vector3(2.f, 3.f, 1.f, u.length) };

    Transaction t;
    t.addBlock({ {0,0,0}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {0,1,0}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {0,2,0}, concrete_20m, 1000.f * u.mass, false });
    SceneUpdater{ scene }.runTransaction(t);

    auto mContacts = Contacts<true>{ scene };
    auto const& cmContacts = mContacts;
    auto iContacts = Contacts<false>{ scene };

    SECTION(".at()") {
        auto runValidTest = [&](auto&& contacts, bool expectedConst) {
            auto const id = ContactIndex{ {0,1,0}, Direction::plusY() };
            auto result = contacts.at(id);
            CHECK(result == ContactReference<false>{ scene, id });
            CHECK(expectedConst == result.structure().userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mContacts, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmContacts, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iContacts, true);
        }

        SECTION("// invalid") {
            auto const id = ContactIndex{ {0,0,0}, Direction::plusY() };
            CHECK_THROWS_AS(iContacts.at(id), std::out_of_range);
        }
    }

    SECTION(".find()") {
        auto runValidTest = [&](auto&& contacts, bool expectedConst) {
            ContactIndex id{ {0,2,0}, Direction::minusY() };
            auto result = contacts.find(id);
            REQUIRE(result.isValid());
            CHECK(result == ContactReference<false>{ scene, id });
            CHECK(expectedConst == result.structure().userData().isCalledAsConst());
        };

        SECTION("// valid - mutable") {
            runValidTest(mContacts, false);
        }

        SECTION("// valid - const") {
            runValidTest(cmContacts, true);
        }

        SECTION("// valid - immutable") {
            runValidTest(iContacts, true);
        }

        SECTION("// invalid") {
            auto const id = ContactIndex{ {0,0,0}, Direction::plusY() };
            auto result = iContacts.find(id);
            CHECK_FALSE(result.isValid());
        }
    }
}
