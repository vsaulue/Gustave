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
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/InternalLinks.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/utils/EndIterator.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    namespace links::detail {
        template<cfg::cLibConfig auto libCfg, common::cSceneUserData UD_, bool isMut_>
        class Enumerator {
        private:
            template<typename T>
            using Prop = utils::Prop<isMut_, T>;

            using InternalLinks = cuboidGridScene::detail::InternalLinks<libCfg, UD_>;
            using SceneData = cuboidGridScene::detail::SceneData<libCfg, UD_>;

            using BlockDataIterator = typename SceneData::Blocks::const_iterator;
        public:
            using Value  = cuboidGridScene::ContactReference<libCfg, UD_, isMut_>;

            [[nodiscard]]
            Enumerator()
                : blockIt_{}
                , blockLinks_{ utils::NO_INIT }
                , internalLinkId_{ 0 }
            {}

            [[nodiscard]]
            explicit Enumerator(Prop<SceneData>& scene)
                : scene_{ &scene }
                , blockIt_{ scene.blocks.begin() }
                , blockLinks_{ utils::NO_INIT }
                , internalLinkId_{ 0 }
            {
                if (blockIt_ != scene_->blocks.end()) {
                    blockLinks_ = InternalLinks{ *scene_, blockIt_->first };
                    next();
                }
            }

            void operator++() {
                ++internalLinkId_;
                next();
            }

            [[nodiscard]]
            Value operator*() const {
                assert(not isEnd());
                return Value{ *scene_, { blockIt_->first, blockLinks_[internalLinkId_].direction } };;
            }

            [[nodiscard]]
            bool isEnd() const {
                return blockIt_ == scene_->blocks.end();
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return (blockIt_ == other.blockIt_) && (internalLinkId_ == other.internalLinkId_);
            }
        private:
            void next() {
                if (internalLinkId_ >= blockLinks_.size()) {
                    ++blockIt_;
                    internalLinkId_ = 0;
                    while (blockIt_ != scene_->blocks.end()) {
                        blockLinks_ = InternalLinks{ *scene_, blockIt_->first };
                        if (blockLinks_.size() > 0) {
                            return;
                        }
                        ++blockIt_;
                    }
                }
            }

            Prop<SceneData>* scene_;
            BlockDataIterator blockIt_;
            InternalLinks blockLinks_;
            std::size_t internalLinkId_;
        };
    }

    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UserData_, bool isMut_>
    class Links {
    public:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        template<bool mut>
        using ContactReference = cuboidGridScene::ContactReference<libCfg, UserData_, mut>;

        using ContactIndex = ContactReference<false>::ContactIndex;
    private:
        using SceneData = detail::SceneData<libCfg, UserData_>;

        template<bool mut>
        using Enumerator = links::detail::Enumerator<libCfg, UserData_, mut>;
    public:
        using ConstIterator = utils::ForwardIterator<Enumerator<false>>;
        using Iterator = utils::ForwardIterator<Enumerator<isMut_>>;

        [[nodiscard]]
        explicit Links(Prop<SceneData>& scene)
            : scene_{ &scene }
        {}

        [[nodiscard]]
        Iterator begin() {
            return Iterator{ *scene_ };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ *scene_ };
        }

        [[nodiscard]]
        utils::EndIterator end() const {
            return {};
        }
    private:
        PropPtr<SceneData> scene_;
    };
}
