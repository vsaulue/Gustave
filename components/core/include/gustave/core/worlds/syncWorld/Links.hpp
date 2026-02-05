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

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/core/worlds/syncWorld/detail/WorldData.hpp>
#include <gustave/core/worlds/syncWorld/ContactReference.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>

namespace gustave::core::worlds::syncWorld {
    template<cfg::cLibConfig auto libCfg>
    class Links {
    private:
        using WorldData = detail::WorldData<libCfg>;
        using SceneLinks = WorldData::Scene::template Links<false>;
    public:
        using ContactReference = syncWorld::ContactReference<libCfg>;
        using ContactIndex = ContactReference::ContactIndex;
    private:
        class Enumerator {
        private:
            using SceneIterator = SceneLinks::Iterator;
        public:
            [[nodiscard]]
            Enumerator()
                : links_{ nullptr }
                , sceneIt_{}
                , value_{ utils::NO_INIT }
            {}

            [[nodiscard]]
            explicit Enumerator(Links const& links)
                : links_{ &links }
                , sceneIt_{ links.sceneLinks_.begin() }
                , value_{ utils::NO_INIT }
            {
                updateValue();
            }

            void operator++() {
                ++sceneIt_;
                updateValue();
            }

            [[nodiscard]]
            bool isEnd() const {
                return sceneIt_ == links_->sceneLinks_.end();
            }

            [[nodiscard]]
            ContactReference const& operator*() const {
                return value_;
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return sceneIt_ == other.sceneIt_;
            }
        private:
            void updateValue() {
                if (!isEnd()) {
                    value_ = ContactReference{ *links_->world_, (*sceneIt_).index() };
                }
            }

            Links const* links_;
            SceneIterator sceneIt_;
            ContactReference value_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit Links(WorldData const& world)
            : world_{ &world }
            , sceneLinks_{ world.scene.links() }
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
        WorldData const* world_;
        SceneLinks sceneLinks_;
    };
}
