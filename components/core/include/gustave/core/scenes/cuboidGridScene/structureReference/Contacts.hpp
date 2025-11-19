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
#include <sstream>

#include <gustave/cfg/cLibConfig.hpp>
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/DataNeighbours.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/BlockReference.hpp>
#include <gustave/core/scenes/cuboidGridScene/forwardDecls.hpp>
#include <gustave/utils/ForwardIterator.hpp>
#include <gustave/utils/NoInit.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene::structureReference {
    template<cfg::cLibConfig auto libCfg_, common::cSceneUserData UD_, bool isMut_>
    class Contacts {
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        template<typename T>
        using PropPtr = utils::PropPtr<isMut_, T>;

        using StructureData = cuboidGridScene::detail::StructureData<libCfg_, UD_>;
    public:
        template<bool mut>
        using ContactReference = cuboidGridScene::ContactReference<libCfg_, UD_, mut>;

        using ContactIndex = cuboidGridScene::ContactIndex;

        [[nodiscard]]
        explicit Contacts(Prop<StructureData>& structure)
            : structure_{ &structure }
        {
            assert(structure_);
        }

        [[nodiscard]]
        ContactReference<true> at(ContactIndex const& contactId)
            requires (isMut_)
        {
            return doAt(*this, contactId);
        }

        [[nodiscard]]
        ContactReference<false> at(ContactIndex const& contactId) const {
            return doAt(*this, contactId);
        }
    private:
        [[nodiscard]]
        static auto doAt(meta::cCvRefOf<Contacts> auto&& self, ContactIndex const& contactId) {
            using Result = decltype(self.at(contactId));
            auto const structId = self.structure_->index();
            auto& scene = self.structure_->sceneData();
            auto result = Result{ scene, contactId };
            auto const& srcId = contactId.localBlockIndex();
            auto srcBlock = scene.blocks.find(srcId);
            if (srcBlock) {
                auto const optOtherId = srcId.neighbourAlong(contactId.direction());
                if (optOtherId) {
                    auto otherBlock = scene.blocks.find(*optOtherId);
                    if (otherBlock) {
                        if ((structId == srcBlock.structureId()) || (structId == otherBlock.structureId())) {
                            return result;
                        }
                    }
                }
            }
            std::stringstream msg;
            msg << "Structure (id=" << self.structure_->index();
            msg << ") does not contain the contact at " << contactId << '.';
            throw std::out_of_range{ msg.str() };
        }

        PropPtr<StructureData> structure_;
    };
}
