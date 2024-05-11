/* This file is part of Gustave, a structural integrity library for video games.
 *
 * Copyright (c) 2022-2024 Vincent Saulue-Laborde <vincent_saulue@hotmail.fr>
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

#include <iostream>
#include <string>
#include <string_view>

#include <gustave/distribs/std/strictUnit/Gustave.hpp>
#include <gustave/examples/JsonGustave.hpp>

using Float = float;
using G = gustave::distribs::std::strictUnit::Gustave<Float>;
using JG = gustave::examples::JsonGustave<G>;
inline constexpr auto u = G::units();

int main() {
    JG::JsonWorld world{ G::vector3(1.f, 1.f, 1.f, u.length), G::vector3(0.f, -1000.f, 0.f, u.acceleration) };
    auto const grey = JG::Color{ 0.5f, 0.5f, 0.5f };
    auto const maxConcreteStress = G::Model::PressureStress{ 20'000'000.f * u.pressure, 14'000'000.f * u.pressure, 2'000'000.f * u.pressure };
    auto const concrete20m = JG::JsonWorld::BlockType{ "Concrete 20MPa", grey, 2400.f * u.mass, maxConcreteStress };
    world.addBlockType(concrete20m);
    {
        JG::JsonWorld::Transaction transaction;
        transaction.addBlock({ 0,0,0 }, concrete20m, true);
        transaction.addBlock({ 0,1,0 }, concrete20m, false);
        transaction.addBlock({ -1,2,0 }, concrete20m, false);
        transaction.addBlock({ 0,2,0 }, concrete20m, false);
        transaction.addBlock({ 1,2,0 }, concrete20m, false);
        transaction.addBlock({ 2,2,0 }, concrete20m, false);
        transaction.addBlock({ 2,1,0 }, concrete20m, false);
        world.update(transaction);
    }
    {
        using Phases = JG::SvgRenderer::Phases;
        JG::SvgRenderer renderer;
        renderer.addPhase(Phases::WorldFramePhase{});
        renderer.addPhase(Phases::BlockTypePhase{});
        renderer.addPhase(Phases::ContactStressPhase{});
        renderer.run(world, std::cout);
    }
    return 0;
}
