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

#include <cassert>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/BlockData.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/math3d/BasicDirection.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::blockReference {
    namespace detail {
        template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
        class Enumerator {
        private:
            template<typename T>
            using Prop = utils::Prop<isMut_, T>;

            using Direction = math3d::BasicDirection;

            using BlockData = cuboidGridScene::detail::BlockData<libCfg_, UD_>;
        public:
            using Value = cuboidGridScene::ContactReference<libCfg_, UD_, isMut_>;

            [[nodiscard]]
            Enumerator()
                : block_{ nullptr }
                , direction_{ 6 }
            {}

            [[nodiscard]]
            explicit Enumerator(Prop<BlockData>& block)
                : block_{ &block }
                , direction_{ 0 }
            {
                next();
            }

            [[nodiscard]]
            bool isEnd() const {
                return direction_ >= 6;
            }

            void operator++() {
                ++direction_;
                next();
            }

            [[nodiscard]]
            Value operator*() const {
                assert(not isEnd());
                return value();
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const = default;
        private:
            void next() {
                while (!isEnd() && !value().isValid()) {
                    ++direction_;
                }
            }

            [[nodiscard]]
            Value value() const {
                return Value{ block_->sceneData(), { block_->index(), static_cast<Direction::Id>(direction_) } };
            }

            Prop<BlockData>* block_;
            int direction_;
        };
    }

    template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
    class Contacts {
    public:
        template<bool mut>
        using ContactReference = cuboidGridScene::ContactReference<libCfg_, UD_, mut>;

        using Direction = ContactReference<false>::ContactIndex::Direction;
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        using BlockData = cuboidGridScene::detail::BlockData<libCfg_, UD_>;

        template<bool mut>
        using Enumerator = detail::Enumerator<libCfg_, UD_, mut>;
    public:
        using ConstIterator = utils::ForwardIterator<Enumerator<false>>;
        using Iterator = utils::ForwardIterator<Enumerator<isMut_>>;

        [[nodiscard]]
        explicit Contacts(Prop<BlockData>& block)
            : block_{ &block }
        {}

        [[nodiscard]]
        ContactReference<true> along(Direction direction)
            requires (isMut_)
        {
            return doAlong(*this, direction);
        }

        [[nodiscard]]
        ContactReference<false> along(Direction direction) const {
            return doAlong(*this, direction);
        }

        [[nodiscard]]
        Iterator begin()
            requires (isMut_)
        {
            return Iterator{ *block_ };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ *block_ };
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }
    private:
        [[nodiscard]]
        static auto doAlong(meta::cCvRefOf<Contacts> auto&& self, Direction direction) {
            using Result = decltype(self.along(direction));
            auto result = Result{ self.block_->sceneData(), {self.block_->index(), direction}};
            if (!result.isValid()) {
                throw result.invalidError();
            }
            return result;
        }

        PropPtr<BlockData> block_;
    };
}
