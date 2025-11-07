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

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UserData_>
    class Links {
    public:
        using ContactReference = cuboidGridScene::ContactReference<libCfg, UserData_, false>;
        using ContactIndex = typename ContactReference::ContactIndex;
    private:
        using SceneData = detail::SceneData<libCfg, UserData_>;
        using BlockDataIterator = typename SceneData::Blocks::const_iterator;
        using InternalLinks = detail::InternalLinks<libCfg, UserData_>;
        using LinkIterator = typename InternalLinks::Iterator;

        class Enumerator {
        public:
            [[nodiscard]]
            Enumerator()
                : blockIt_{}
                , blockLinks_{ utils::NO_INIT }
                , internalLinkId_{ 0 }
            {}

            [[nodiscard]]
            explicit Enumerator(SceneData const& scene)
                : scene_{ &scene }
                , blockIt_{ scene.blocks.begin() }
                , blockLinks_{ utils::NO_INIT }
                , internalLinkId_{ 0 }
                , value_{ utils::NO_INIT }
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
            ContactReference const& operator*() const {
                return value_;
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
                if (internalLinkId_ < blockLinks_.size()) {
                    updateValue();
                } else {
                    ++blockIt_;
                    while (blockIt_ != scene_->blocks.end()) {
                        blockLinks_ = InternalLinks{ *scene_, blockIt_->first };
                        if (blockLinks_.size() > 0) {
                            internalLinkId_ = 0;
                            updateValue();
                            return;
                        }
                        ++blockIt_;
                    }
                }
            }

            void updateValue() {
                value_ = ContactReference{ *scene_, ContactIndex{ blockIt_->first, blockLinks_[internalLinkId_].direction } };
            }

            SceneData const* scene_;
            BlockDataIterator blockIt_;
            InternalLinks blockLinks_;
            std::size_t internalLinkId_;
            ContactReference value_;
        };
    public:
        using Iterator = utils::ForwardIterator<Enumerator>;

        [[nodiscard]]
        explicit Links(SceneData const& scene)
            : scene_{ &scene }
        {}

        [[nodiscard]]
        Iterator begin() const {
            return Iterator{ *scene_ };
        }

        [[nodiscard]]
        utils::EndIterator end() const {
            return {};
        }
    private:
        SceneData const* scene_;
    };
}
