#include <filesystem>
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>
#include "mcctp.h"

std::filesystem::path GetTexturePackPath(void);
void ExitWithError(const char* msg, int err);

typedef std::pair<std::string, std::string> Patch;
std::vector<Patch> ParsePatchesFromArgs(int argc, char** argv);
mcctp::PatchMap ParsePatchMapFromPatches(const std::vector<Patch> &patches);
void PrintPatchMap(const mcctp::PatchMap &map);

bool PatchAndWriteBins(const std::vector<Patch> &patches);

std::filesystem::path GetTexturePackPath(void) {
  static const auto path(std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() /
                         "ui/texturepacks");

  return path;
}

void ExitWithError(const char *msg, int err) {
  std::cerr << msg << std::endl;
  std::exit(err);
}

std::vector<Patch> ParsePatchesFromArgs(int argc, char **argv) {
  std::vector<Patch> patches;

  for (int i = 1; i < argc; ++i) {
    if (argv[i] != nullptr) {
      std::vector<std::string> patch_pair;
      std::string resource;
      std::stringstream arg(argv[i]);
      while (std::getline(arg, resource, '=')) {
        patch_pair.push_back(resource);
      }
      if (patch_pair.size() != 2) {
        ExitWithError("Invalid Patch Pair", -1);
      } else {
        std::cout << "Patch: " << patch_pair[0] << " -> " << patch_pair[1] << std::endl;
        patches.push_back({patch_pair[0], patch_pair[1]});
      }
    }
  }
  return patches;
}

mcctp::PatchMap ParsePatchMapFromPatches(const std::vector<Patch>& patches) {
  mcctp::PatchMap patch_map;
  if (patches.size() == 0) {
    return mcctp::PatchMap();
  } else {
    auto *inst = mcctp::ctx::Instance();
    for (const auto &patch : patches) {
      const auto &src_name = patch.first;
      const auto &dst_name = patch.second;
      
      const auto res1 = inst->GetManagedResource(src_name);
      const auto res2 = inst->GetManagedResource(dst_name);

      //mcctp::PatchSource patch_src(src_name, src_name, mcctp::SourceType::Internal);
      //if (patch_src.GetType() != mcctp::SourceType::Invalid) {
      //}

      if (res1.has_value() && res2.has_value()) {
        mcctp::PatchSource patch_src(src_name, src_name, mcctp::SourceType::Internal);
        patch_map.insert({patch_src, res2.value()});
      } else {
        std::cout << "No Resource Found: res1: " << res1.has_value()
                  << " res2: " << res2.has_value() << std::endl;
        //throw std::exception("No Resource found");
      }
    }
  }
  return patch_map;
}

bool PatchAndWriteBins(const std::vector<Patch> &patches) {
  // Inject and overwrite resource
  if (patches.size() == 0) {
    return false;
  } else {
    auto *inst = mcctp::ctx::Instance();
    for (const auto &patch : patches) {
      const auto &src = patch.first;
      const auto &dst = patch.second;
      const auto target_pack = inst->GetTexturePackFlagFromResourceName(dst);
      bool ok = mcctp::InjectResource(src, dst, target_pack);
      if (!ok)
        return false;
    }
  }
  return true;
}

void PrintPatchMap(const mcctp::PatchMap &map) {
  int count = 1;
  std::cout << "Patch Map: " << std::endl;
  for (const auto &entry : map) {
    const auto &src = entry.first;
    const auto &dst = entry.second;
    int align = max(src.GetID().length(), dst->Name.length());
    std::cout << std::to_string(count) << " (" << mcctp::SourceTypeToString.at(src.GetType()) << ")"
              << ":\n\t" << std::setw(align) << src.GetID() << " | "
              << *std::get<mcctp::ManagedResource>(src.GetProvider()) 
        << std::endl << "\t" << dst->Name << " | " << *dst << std::endl;

    count += 1;
  }
  std::cout << std::endl;
}