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

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include <gustave/distribs/std/strictUnit/Gustave.hpp>
#include <gustave/examples/JsonGustave.hpp>

using Float = float;
using G = gustave::distribs::std::strictUnit::Gustave<Float>;
using JG = gustave::examples::JsonGustave<G>;
inline constexpr auto u = G::units();

[[nodiscard]]
static std::ifstream openFIle(char const* filename) {
    std::ifstream result;
    result.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        result.open(filename);
    }
    catch (std::system_error const&) {
        std::stringstream msg;
        msg << "Could not open '" << filename << "': " << std::error_code{ errno, std::generic_category() }.message() << '.';
        throw std::invalid_argument(msg.str());
    }
    return result;
}

[[nodiscard]]
static std::unique_ptr<JG::JsonWorld> parseWorldFile(char const* filename) {
    std::ifstream inputFile = openFIle(filename);
    try {
        auto const json = JG::Json::parse(inputFile);
        return JG::JsonWorld::fromJson(json);
    } catch (std::exception const& e) {
        std::stringstream msg;
        msg << "Could not parse '" << filename << "': " << e.what();
        throw std::invalid_argument(msg.str());
    }
}

static void doRender(JG::JsonWorld const& world, std::ostream& output) {
    using Phases = JG::SvgRenderer::Phases;
    JG::SvgRenderer renderer;
    renderer.addPhase(Phases::WorldFramePhase{});
    renderer.addPhase(Phases::BlockTypePhase{});
    renderer.addPhase(Phases::ContactStressPhase{});
    renderer.run(world, output);
}

int main(int argc, char* argv[]) {
    try {
        if (argc <= 1) {
            std::cerr << "[ERROR] Missing <path> to Json file as argument";
            return 1;
        }
        auto const world = parseWorldFile(argv[1]);
        doRender(*world, std::cout);
    } catch (std::exception const& e) {
        std::cerr << "\n[ERROR] " << e.what() << '\n';
        return 1;
    }
    return 0;
}
