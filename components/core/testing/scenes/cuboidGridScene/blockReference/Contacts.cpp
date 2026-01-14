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

#include <array>
#include <ranges>

#include <gustave/core/scenes/cuboidGridScene/blockReference/Contacts.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>
#include <gustave/testing/ConstDetector.hpp>

#include <TestHelpers.hpp>
#include <SceneUserData.hpp>

namespace cuboid = gustave::core::scenes::cuboidGridScene;

using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, SceneUserData>;

template<bool mut>
using Contacts = cuboid::blockReference::Contacts<libCfg, SceneUserData, mut>;

using BlockIndex = SceneData::BlockIndex;
using ContactIndex = Contacts<false>::ContactReference<false>::ContactIndex;
using Direction = Contacts<false>::Direction;
using Transaction = SceneUpdater::Transaction;

static_assert(std::ranges::forward_range<Contacts<true>>);
static_assert(std::ranges::forward_range<Contacts<false>>);

TEST_CASE("core::scenes::cuboidGridScene::blockReference::Contacts") {
    auto const blockSize = vector3(1.f, 1.f, 1.f, u.length);
    SceneData scene{ blockSize };

    Transaction t;
    auto newBlock = [&](BlockIndex const& index, bool isFoundation) -> void {
        t.addBlock({ index, concrete_20m, 1000.f * u.mass, isFoundation });
    };
    newBlock({ 1,1,1 }, false);
    newBlock({ 0,1,1 }, true);
    newBlock({ 2,1,1 }, false);
    newBlock({ 1,0,1 }, true);
    newBlock({ 1,2,1 }, false);
    newBlock({ 1,1,0 }, false);
    newBlock({ 1,1,2 }, false);
    newBlock({ 0,0,9 }, false);
    SceneUpdater{ scene }.runTransaction(t);

    auto mContacts111 = Contacts<true>{ scene, {1,1,1} };
    auto const& cmContacts111 = mContacts111;
    auto iContacts111 = Contacts<false>{ scene, {1,1,1} };

    auto mContacts009 = Contacts<true>{ scene, {0,0,9} };

    auto contactId = [](BlockIndex const& localBlockId, Direction direction) {
        return ContactIndex{ localBlockId, direction };
    };

    SECTION(".along()") {
        auto runValidTest = [&](auto&& contacts, bool expectedConst) {
            auto result = contacts.along(Direction::minusY());
            REQUIRE(result.isValid());
            CHECK(expectedConst == result.structure().userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runValidTest(mContacts111, false);
        }

        SECTION("// const") {
            runValidTest(cmContacts111, true);
        }

        SECTION("// immutable") {
            runValidTest(iContacts111, true);
        }

        SECTION("// invalid") {
            CHECK_THROWS_AS(mContacts009.along(Direction::plusY()), std::out_of_range);
        }
    }

    SECTION(".begin() // & .end()") {
        SECTION("// 6 contacts") {
            auto runTest = [&](auto&& contacts, bool expectedConst) {
                auto expectedIds = std::array{
                    contactId({1,1,1}, Direction::minusX()),
                    contactId({1,1,1}, Direction::plusX()),
                    contactId({1,1,1}, Direction::minusY()),
                    contactId({1,1,1}, Direction::plusY()),
                    contactId({1,1,1}, Direction::minusZ()),
                    contactId({1,1,1}, Direction::plusZ()),
                };
                auto ids = contacts | std::views::transform([](auto&& contact) { return contact.index(); });
                REQUIRE_THAT(ids, matchers::c2::UnorderedRangeEquals(expectedIds));
                CHECK(expectedConst == (*contacts.begin()).structure().userData().isCalledAsConst());
            };

            SECTION("// mutable") {
                runTest(mContacts111, false);
            }

            SECTION("// const") {
                runTest(cmContacts111, true);
            }

            SECTION("// immutable") {
                runTest(iContacts111, true);
            }
        }

        SECTION("// empty") {
            CHECK(mContacts009.begin() == mContacts009.end());
        }
    }
}
