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
#include <gustave/cfg/LibTraits.hpp>
#include <gustave/core/scenes/common/cSceneUserData.hpp>
#include <gustave/core/scenes/cuboidGridScene/detail/SceneData.hpp>
#include <gustave/core/scenes/cuboidGridScene/ContactReference.hpp>
#include <gustave/utils/Prop.hpp>

namespace gustave::core::scenes::cuboidGridScene {
    template<cfg::cLibConfig auto libCfg, common::cSceneUserData UD_, bool isMut_>
    class Contacts {
    private:
        template<typename T>
        using Prop = utils::Prop<isMut_, T>;

        using SceneData = detail::SceneData<libCfg, UD_>;
    public:
        template<bool mut>
        using ContactReference = cuboidGridScene::ContactReference<libCfg, UD_, mut>;

        using ContactIndex = typename ContactReference<false>::ContactIndex;

        [[nodiscard]]
        explicit Contacts(Prop<SceneData>& scene)
            : scene_{ &scene }
        {}

        [[nodiscard]]
        ContactReference<true> at(ContactIndex const& index)
            requires (isMut_)
        {
            return doAt(*this, index);
        }

        [[nodiscard]]
        ContactReference<false> at(ContactIndex const& index) const {
            return doAt(*this, index);
        }

        [[nodiscard]]
        ContactReference<true> find(ContactIndex const& index)
            requires (isMut_)
        {
            return ContactReference<true>{ *scene_, index };
        }

        [[nodiscard]]
        ContactReference<false> find(ContactIndex const& index) const {
            return ContactReference<false>{ *scene_, index };
        }
    private:
        [[nodiscard]]
        static auto doAt(meta::cCvRefOf<Contacts> auto&& self, ContactIndex const& index) {
            using Result = decltype(self.at(index));
            auto result = Result{ *self.scene_, index };
            if (!result.isValid()) {
                throw result.invalidError();
            }
            return result;
        }

        Prop<SceneData>* scene_;
    };
}
