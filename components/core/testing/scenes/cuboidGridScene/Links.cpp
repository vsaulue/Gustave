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

#include <gustave/core/scenes/cuboidGridScene/BlockIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/Links.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneUpdater.hpp>

#include <SceneUserData.hpp>
#include <TestHelpers.hpp>

namespace cuboid = ::gustave::core::scenes::cuboidGridScene;

template<bool mut>
using Links = cuboid::Links<libCfg, SceneUserData,mut>;

using SceneData = cuboid::detail::SceneData<libCfg, SceneUserData>;
using SceneUpdater = cuboid::detail::SceneUpdater<libCfg, SceneUserData>;

template<bool mut>
using ContactReference = Links<false>::ContactReference<mut>;

using BlockIndex = cuboid::BlockIndex;
using Direction = ContactReference<false>::Direction;
using Transaction = SceneUpdater::Transaction;

static_assert(std::ranges::forward_range<Links<false>>);
static_assert(std::ranges::forward_range<Links<true>>);

TEST_CASE("core::scenes::cuboidGridScene::Links") {
    SceneData scene{ vector3(1.f, 2.f, 3.f, u.length) };

    Transaction t;
    t.addBlock({ {2,2,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {3,2,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,3,2}, concrete_20m, 1000.f * u.mass, false });
    t.addBlock({ {2,2,3}, concrete_20m, 1000.f * u.mass, true });
    t.addBlock({ {2,3,3}, concrete_20m, 1000.f * u.mass, true });
    SceneUpdater{ scene }.runTransaction(t);

    auto mLinks = Links<true>{ scene };
    auto const& cmLinks = mLinks;
    auto iLinks = Links<false>{ scene };

    auto makeContact = [&](BlockIndex const& localBlockId, Direction direction) {
        return ContactReference<false>{ scene, { localBlockId, direction} };
    };

    SECTION(".begin() // & .end()") {
        auto runTest = [&](auto&& links, bool expectedConst) {
            auto const expected = std::array{
                makeContact({2,2,2}, Direction::plusX()),
                makeContact({2,2,2}, Direction::plusY()),
                makeContact({2,2,2}, Direction::plusZ()),
                makeContact({2,3,2}, Direction::plusZ()),
            };
            REQUIRE_THAT(links, matchers::c2::UnorderedRangeEquals(expected));
            CHECK(expectedConst == (*links.begin()).structure().userData().isCalledAsConst());
        };

        SECTION("// mutable") {
            runTest(mLinks, false);
        }

        SECTION("// const") {
            runTest(cmLinks, true);
        }

        SECTION("// immutable") {
            runTest(iLinks, true);
        }
    }
}
