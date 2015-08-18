/*******************************************************************************
 * benchmarks/word_count/word_count.cpp
 *
 * Runner program for WordCount example. See c7a/examples/word_count.hpp for the
 * source to the example.
 *
 * Part of Project c7a.
 *
 * Copyright (C) 2015 Timo Bingmann <tb@panthema.net>
 *
 * This file has no license. Only Chuck Norris can compile it.
 ******************************************************************************/

#include <c7a/common/cmdline_parser.hpp>
#include <c7a/common/logger.hpp>
#include <c7a/examples/word_count.hpp>

using namespace c7a; // NOLINT

int main(int argc, char* argv[]) {

    common::CmdlineParser clp;

    clp.SetVerboseProcess(false);

    unsigned int elements = 1000;
    clp.AddUInt('s', "elements", "S", elements,
                "Create wordcount example with S generated words");

    if (!clp.Process(argc, argv)) {
        return -1;
    }

    clp.PrintResult();

    auto start_func =
        [elements](api::Context& ctx) {
            size_t uniques = examples::WordCountGenerated(ctx, elements);
            sLOG1 << "wrote counts of" << uniques << "unique words";
        };

    return api::Run(start_func);
}

/******************************************************************************/