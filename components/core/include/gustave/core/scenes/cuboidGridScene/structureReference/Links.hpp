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

#include <cassert>
#include <cstdint>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/InternalLinks.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/StructureData.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactIndex.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::structureReference {
    namespace links::detail {
        template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
        class Enumerator {
        private:
            template<typename T>
            using Prop = utils::Prop<isMut_, T>;

            using InternalLinks = cuboidGridScene::detail::InternalLinks<libCfg_, UD_>;
            using StructureData = cuboidGridScene::detail::StructureData<libCfg_, UD_>;

            using SolverIndexIterator = StructureData::SolverIndices::const_iterator;
        public:
            using Value = ContactReference<libCfg_, UD_, isMut_>;

            Enumerator()
                : structure_{ nullptr }
                , solverIndexIt_{}
                , internalLinks_{ utils::NO_INIT }
                , linkIndex_{ 0 }
            {}

            explicit Enumerator(Prop<StructureData>& structure)
                : structure_{ &structure }
                , solverIndexIt_{ structure.solverIndices().begin() }
                , internalLinks_{ utils::NO_INIT }
                , linkIndex_{ 0 }
            {
                if (!isEnd()) {
                    updateInternalLinks();
                    next();
                }
            }

            void operator++() {
                ++linkIndex_;
                next();
            }

            [[nodiscard]]
            Value operator*() const {
                auto const index = ContactIndex{ solverIndexIt_->first, internalLinks_[linkIndex_].direction };
                return Value{ structure_->sceneData(), index };
            }

            [[nodiscard]]
            bool isEnd() const {
                return solverIndexIt_ == structure_->solverIndices().end();
            }

            [[nodiscard]]
            bool operator==(Enumerator const& other) const {
                return (solverIndexIt_ == other.solverIndexIt_) && (linkIndex_ == other.linkIndex_);
            }
        private:
            void next() {
                auto const structId = structure_->index();
                while (true) {
                    while (linkIndex_ < internalLinks_.size()) {
                        if ((structId == internalLinks_.source().structureId()) || (structId == internalLinks_[linkIndex_].otherBlock.structureId())) {
                            return;
                        }
                        ++linkIndex_;
                    }
                    ++solverIndexIt_;
                    if (!isEnd()) {
                        updateInternalLinks();
                        linkIndex_ = 0;
                    } else {
                        return;
                    }
                }
            }

            void updateInternalLinks() {
                internalLinks_ = InternalLinks{ structure_->sceneData(), solverIndexIt_->first };
            }

            Prop<StructureData>* structure_;
            SolverIndexIterator solverIndexIt_;
            InternalLinks internalLinks_;
            std::size_t linkIndex_;
        };
    }

    template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
    class Links {
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        using StructureData = detail::StructureData<libCfg_, UD_>;

        template<bool mut>
        using Enumerator = links::detail::Enumerator<libCfg_, UD_, mut>;
    public:
        using ConstIterator = utils::ForwardIterator<Enumerator<false>>;
        using Iterator = utils::ForwardIterator<Enumerator<isMut_>>;

        using ContactIndex = cuboidGridScene::ContactIndex;

        template<bool mut>
        using ContactReference = cuboidGridScene::ContactReference<libCfg_, UD_, mut>;

        [[nodiscard]]
        explicit Links(Prop<StructureData>& structure)
            : structure_{ &structure }
        {
            assert(structure_);
        }

        [[nodiscard]]
        Iterator begin()
            requires (isMut_)
        {
            return Iterator{ *structure_ };
        }

        [[nodiscard]]
        ConstIterator begin() const {
            return ConstIterator{ *structure_ };
        }

        [[nodiscard]]
        constexpr std::default_sentinel_t end() const {
            return {};
        }
    private:
        PropPtr<StructureData> structure_;
    };
}
