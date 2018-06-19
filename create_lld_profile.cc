// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gflags/gflags.h"
#include "profile_creator.h"
#include "gcov.h"

#include <fstream>
#include <iostream>

DEFINE_string(profiler, "perf",
              "Profile type");
DEFINE_string(cgprofile, "cgprofile.txt",
              "Output file name");
DEFINE_string(binary, "data.binary",
              "Binary file name");

using namespace autofdo;

namespace {

struct LLDProfileBuilder : SymbolTraverser {
  std::ostream &os;
  LLDProfileBuilder(std::ostream &os) : os(os) {}

  void Visit(const Symbol *node) {
    for (const auto &pos_count : node->pos_counts)
      for (const auto &target_count : pos_count.second.target_map)
        os << node->name() << ' ' << target_count.first
           << ' ' << target_count.second << '\n';
  }

  void Start(const SymbolMap &symbol_map) {
    SymbolTraverser::Start(symbol_map);
  }
};

struct LLDProfileWriter : ProfileWriter {
  virtual bool WriteToFile(const string &output_file) {
    std::ofstream os(output_file);

    LLDProfileBuilder builder(os);
    builder.Start(*symbol_map_);
  }
};

}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::vector<std::string> profiles(argv + 1, argv + argc);

  LLDProfileWriter writer;
  autofdo::ProfileCreator creator(FLAGS_binary);
  if (creator.CreateProfile(profiles, FLAGS_profiler, &writer,
                            FLAGS_cgprofile)) {
    return 0;
  } else {
    return -1;
  }
}
