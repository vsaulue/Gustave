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

namespace gustave::core::scenes::cuboidGridScene {
    namespace detail {
        template<cfg::cLibConfig auto, common::cSceneUserData>
        class StructureData;

        template<cfg::cLibConfig auto, common::cSceneUserData>
        struct SceneData;

        template<cfg::cLibConfig auto, common::cSceneUserData>
        class SceneStructures;
    }

    namespace blockReference {
        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        class Contacts;

        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        class Structures;
    }

    namespace structureReference {
        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        class Blocks;

        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        class Contacts;

        template<cfg::cLibConfig auto, common::cSceneUserData, bool>
        class Links;
    }

    template<cfg::cLibConfig auto, common::cSceneUserData, bool>
    class BlockReference;

    template<cfg::cLibConfig auto, common::cSceneUserData, bool>
    class StructureReference;

    template<cfg::cLibConfig auto, common::cSceneUserData, bool>
    class ContactReference;
}
