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
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include <CLI/CLI.hpp>

class Tutorial {
public:
    [[nodiscard]]
    explicit Tutorial(std::string_view title, int argc, char** argv) {
        auto const desc = appDesc(title);
        auto app = CLI::App{ desc };
        argv = app.ensure_utf8(argv);
        app.add_flag("--snippets", enableSnippets_, "Print MkDocs snippets delimiters in stdout");
        try {
            app.parse(argc, argv);
        } catch (CLI::ParseError const& e) {
            earlyExitCode_ = app.exit(e);
            return;
        }
        std::cout << desc << "\n\n";
    }

    ~Tutorial() {
        assert(!inSection_); // Last section of the program not closed.
    }

    void section(std::string_view snippetName, std::string_view description) {
        endSection();
        ++sectionIndex_;
        inSection_ = true;
        sectionSnippetName_ = snippetName;
        std::cout << "--------------------\n";
        std::cout << "Section " << sectionIndex_ << ": " << description << "\n\n";
        if (enableSnippets_) {
            std::cout << "--8<-- [start:" << snippetName << "]\n";
        }
    }

    void endSection() {
        if (inSection_) {
            if (enableSnippets_) {
                std::cout << "--8<-- [end:" << sectionSnippetName_ << "]\n";
            }
            std::cout << "\n\n";
            inSection_ = false;
        }
    }

    [[nodiscard]]
    std::optional<int> const& earlyExitCode() const {
        return earlyExitCode_;
    }
private:
    [[nodiscard]]
    static std::string appDesc(std::string_view title) {
        std::stringstream appDesc;
        appDesc << "Gustave's tutorial: " << title;
        return appDesc.str();
    }

    std::optional<int> earlyExitCode_;
    std::size_t sectionIndex_ = 0;
    bool enableSnippets_ = false;
    bool inSection_ = false;
    std::string sectionSnippetName_;
};
