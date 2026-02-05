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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

#include <CLI/CLI.hpp>

#include <gustave/distribs/std/strictUnit/Gustave.hpp>
#include <gustave/examples/JsonGustave.hpp>

using Float = double;
using G = gustave::distribs::std::strictUnit::Gustave<Float>;
using JG = gustave::examples::JsonGustave<G>;
inline constexpr auto u = G::units();

struct Arguments {
    std::string inputWorldFileName;
    std::optional<std::string> rendererFileName;
};

static void initArgumentsParser(CLI::App& parser, Arguments& output) {
    CLI::Option* input = parser.add_option("-i,--input-world,input-world", output.inputWorldFileName, "Input file containing the JSON world");
    input->required();

    parser.add_option("-r,--renderer", output.rendererFileName, "Input file describing the SVG renderer");

    parser.validate_positionals();
}

[[nodiscard]]
static std::ifstream openFile(std::string_view filename) {
    std::ifstream result;
    result.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        result.open(filename.data());
    }
    catch (std::system_error const&) {
        std::stringstream msg;
        msg << "Could not open '" << filename << "': " << std::error_code{ errno, std::generic_category() }.message() << '.';
        throw std::invalid_argument(msg.str());
    }
    return result;
}

[[nodiscard]]
static JG::JsonWorld parseWorldFile(std::string_view fileName) {
    std::ifstream inputFile = openFile(fileName);
    try {
        auto const json = JG::Json::parse(inputFile);
        return json.get<JG::JsonWorld>();
    } catch (std::exception const& e) {
        std::stringstream msg;
        msg << "Could not parse '" << fileName << "': " << e.what();
        throw std::invalid_argument(msg.str());
    }
}

[[nodiscard]]
static JG::SvgRenderer makeRenderer(std::optional<std::string> const& fileName) {
    if (fileName) {
        std::ifstream inputFile = openFile(*fileName);
        try {
            auto const json = JG::Json::parse(inputFile);
            return json.get<JG::SvgRenderer>();
        } catch (std::exception const& e) {
            std::stringstream msg;
            msg << "Could not parse '" << *fileName << "': " << e.what();
            throw std::invalid_argument(msg.str());
        }
    } else {
        // default renderer
        using Phases = JG::SvgRenderer::Phases;
        JG::SvgRenderer result;
        result.addPhase(Phases::WorldFramePhase{});
        result.addPhase(Phases::BlockTypePhase{});
        result.addPhase(Phases::ContactStressPhase{});
        return result;
    }
}

int main(int argc, char* argv[]) {
    CLI::App argsParser{ "Gustave's SVG viewer." };
    try {
        Arguments args;
        initArgumentsParser(argsParser, args);
        argsParser.parse(argc, argv);
        auto const renderer = makeRenderer(args.rendererFileName);
        auto const world = parseWorldFile(args.inputWorldFileName);
        renderer.run(world, std::cout);
        return EXIT_SUCCESS;
    } catch (CLI::ParseError const& e) {
        if (e.get_name() == "CallForHelp") {
            std::cout << argsParser.help();
            return EXIT_SUCCESS;
        }
        if (e.get_name() == "CallForAllHelp") {
            std::cout << argsParser.help("", CLI::AppFormatMode::All);
            return EXIT_SUCCESS;
        }
        std::cerr << "[ERROR] " << e.what() << '\n';
        return EXIT_FAILURE;
    } catch (std::exception const& e) {
        std::cerr << "[ERROR] " << e.what() << '\n';
        return EXIT_FAILURE;
    }
}
