#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>
#include <ostream>
#include <fstream>
#include <tuple>
#include <memory>

#include "fpng.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <codecvt>
#include <locale>

namespace mcctp {
#pragma warning(disable : 4996)
using converter = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>;

struct FileMapping {
private:
  HANDLE hFile = INVALID_HANDLE_VALUE;
  HANDLE hMap = NULL;
  LPVOID lpMap = NULL;

public:
  explicit FileMapping(std::wstring path) {
    if (!TryCreateMapping(path)) {
      throw std::exception("File mapping could not be created!");
    }
  }

  FileMapping(FileMapping &&other) noexcept {
    // Transfer ownership of the handles from the other object
    hFile = other.hFile;
    hMap = other.hMap;
    lpMap = other.lpMap;

    // Reset the other object's members to safe defaults
    other.hFile = INVALID_HANDLE_VALUE;
    other.hMap = NULL;
    other.lpMap = NULL;
  }

  FileMapping &operator=(FileMapping &&other) noexcept {
    if (this != &other) {
      UnmapViewOfFile(lpMap);
      CloseHandle(hMap);
      CloseHandle(hFile);

      // Transfer ownership of the handles from the other object
      hFile = other.hFile;
      hMap = other.hMap;
      lpMap = other.lpMap;

      // Reset the other object's members to safe defaults
      other.hFile = INVALID_HANDLE_VALUE;
      other.hMap = NULL;
      other.lpMap = NULL;
    }
    return *this;
  }

  // Delete copy constructor and copy assignment operator
  FileMapping(const FileMapping &) = delete;
  FileMapping &operator=(const FileMapping &) = delete;

  ~FileMapping() {
    if (lpMap != NULL)
      UnmapViewOfFile(lpMap);
    if (hMap != NULL)
      CloseHandle(hMap);
    if (hFile != INVALID_HANDLE_VALUE)
      CloseHandle(hFile);
  }

  HANDLE GetFileHandle(void) const {
      return hFile;
  }

  HANDLE GetFileMap(void) const {
      return hMap;
  }

  LPVOID GetMapView(void) const {
      return lpMap;
  }

private:
  bool TryCreateMapping(std::wstring path) {
    hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
      std::cerr << "Could not open file: ";
      std::cerr << GetLastError() << std::endl;
      return false;
    }

    hMap = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    if (hMap == NULL) {
      std::cerr << "Could not create file mapping ";
      std::cerr << GetLastError() << std::endl;
      CloseHandle(hFile);
      return false;
    }

    lpMap = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    if (lpMap == NULL) {
      std::cerr << "Could not map view of file ";
      std::cerr << GetLastError() << std::endl;
      CloseHandle(hMap);
      CloseHandle(hFile);
      return false;
    }

    std::cout << "Success!" << std::endl;
    return true;
  }
};

static constexpr uint8_t TexturePackCount = 13;

enum TexturePackFlags {
  Controller = 1 << 0,
  Emblems = 1 << 1,
  GlobalUI = 1 << 2,
  Hopper = 1 << 3,
  InGameChapter = 1 << 4,
  LargeAvatar = 1 << 5,
  Levels = 1 << 6,
  Loading = 1 << 7,
  MainMenuAndCampaign = 1 << 8,
  MainMenu = 1 << 9,
  Medals = 1 << 10,
  Skulls = 1 << 11,
  SPMapPreview = 1 << 12,
  All = (Controller | Emblems | GlobalUI | Hopper | InGameChapter | LargeAvatar | Levels | Loading |
         MainMenuAndCampaign | MainMenu | Medals | Skulls | SPMapPreview),
};

static std::unordered_map<TexturePackFlags, const char *> FlagToBasename = {
    {TexturePackFlags::Controller, "controllertexturepack"},
    {TexturePackFlags::Emblems, "emblemstexturepack"},
    {TexturePackFlags::GlobalUI, "globaluitexturepack"},
    {TexturePackFlags::Hopper, "hoppertexturepack"},
    {TexturePackFlags::InGameChapter, "ingamechapterpack"},
    {TexturePackFlags::LargeAvatar, "largeavatartexturepack"},
    {TexturePackFlags::Levels, "levelstexturepack"},
    {TexturePackFlags::Loading, "loadingtexturepack"},
    {TexturePackFlags::MainMenuAndCampaign, "mainmenuandcampaigntexturepack"},
    {TexturePackFlags::MainMenu, "mainmenutexturepack"},
    {TexturePackFlags::Medals, "medalstexturepack"},
    {TexturePackFlags::Skulls, "skullstexturepack"},
    {TexturePackFlags::SPMapPreview, "spmappreviewtexturepack"},
};

enum class TextureResourceFormat {
    A8R8G8B8,
    DXT1,
    DXT3,
    DXT5,
    INVALID,
};

struct TexturePackBlock {
  size_t Key;
  size_t Size;
  size_t Offset;

  friend inline std::ostream &operator<<(std::ostream &os, const TexturePackBlock &block) {
    os << "Key: " << block.Key << ", "
       << "Size: " << block.Size << ", "
       << "Offset: " << block.Offset << ", ";
    return os;
  }
};

struct TexturePackResource {
  std::string Name;
  size_t Offset;
  size_t Size;
  int32_t Width;
  int32_t Height;
  TextureResourceFormat Format;

  friend inline std::ostream &operator<<(std::ostream &os, const TexturePackResource &res) {
    os << "Name: " << res.Name << ", "
       << "Offset: " << res.Offset << ", "
       << "Size: " << res.Size << ", "
       << "Width: " << res.Width << ", "
       << "Height: " << res.Height << ", "
       << "Format: " << (int)res.Format; // Ensure TextureResourceFormat has an << operator.
    return os;
  }
};

enum class TexturePackType {
    PERM,
    TEMP,
    INVALID
};

typedef std::tuple<FileMapping, FileMapping, TexturePackType> TexturePack;

static const FileMapping &GetPerm(const TexturePack &tp) { return std::get<0>(tp); }
static const FileMapping &GetTemp(const TexturePack &tp) { return std::get<1>(tp); }
static   TexturePackType  GetType(const TexturePack &tp) { return std::get<2>(tp); }

typedef std::variant<TexturePackResource, TexturePackBlock> TexturePackEntry;

typedef std::unordered_map<std::string, TexturePackEntry> TexturePackIndex;

struct TexturePackField {
  uint16_t m_Field = 0;

  void SetFlag(TexturePackFlags flags) { m_Field |= (int)flags; }

  void UnsetFlag(TexturePackFlags flags) { m_Field &= ~(int)flags; }

  void FlipFlag(TexturePackFlags flags) { m_Field ^= (int)flags; }

  bool HasFlag(TexturePackFlags flags) { return (m_Field & (int)flags) == (int)flags; }

  bool HasAnyFlag(TexturePackFlags flags) { return (m_Field & (int)flags) != 0; }
};

class ctx {
public:
  static ctx *Instance() {
    static ctx *Context = new ctx;
    static bool init = true;
    if (init) {
      fpng::fpng_init();
      init = false;
    }
    return Context;
  }
  void SetField(uint16_t field = (uint16_t)TexturePackFlags::All) {
    m_Field = (TexturePackField)field;
  }
  TexturePackField GetField(void) const { return m_Field; }
  void SetPathPrefix(std::filesystem::path path_prefix) { m_PathPrefix = path_prefix; }
  bool MemoryMapTexturePack(TexturePackFlags flag, std::wstring texture_pack_basename) {
    try {
      using namespace std::filesystem;
      path perm =
          m_PathPrefix.wstring() + L'\\' + texture_pack_basename + L".perm.bin";
      path temp =
          m_PathPrefix.wstring() + L'\\' + texture_pack_basename + L".temp.bin";
      path idx =
          m_PathPrefix.wstring() + L'\\' + texture_pack_basename + L".perm.idx";

      if (!(exists(perm) && (exists(temp) || exists(idx))))
        throw std::exception("Exception in MemoryMapTexturePack() \"Invalid TexturePack\"");

      if (std::filesystem::exists(temp)) {
        TexturePack tp = {
            FileMapping(perm),
            FileMapping(temp),
            TexturePackType::TEMP
        };
        m_TexturePackFileMap.insert(std::make_pair(flag, std::move(tp)));
        return true;
      } else {
        TexturePack tp = {
            FileMapping(perm), 
            FileMapping(idx), 
            TexturePackType::PERM
        };
        m_TexturePackFileMap.insert(std::make_pair(flag, std::move(tp)));
      }
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  }
  bool _IndexTexturePack(TexturePackFlags flag) {
    try {
      const auto &fm = m_TexturePackFileMap.at(flag);
      // impl
      std::cout << "Success!" << std::endl;
      return true;
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  }
  bool IndexTexturePack(TexturePackFlags flag) {
      const auto& fm = GetPerm(m_TexturePackFileMap.at(flag));
      const LPVOID lpMap = fm.GetMapView();
      const HANDLE hFile = fm.GetFileHandle();
      char* fptr = static_cast<char*>(lpMap);
      char* fend = fptr + GetFileSize(hFile, NULL);
      std::cout << "TexturePackSize: (" << GetFileSize(hFile, NULL) << ")" << std::endl;

      std::vector<TexturePackBlock> blocks;
      std::vector<TexturePackResource> resources;
      int neither = 0;

      while (fptr < fend) {
          uint32_t pos = fptr - static_cast<char*>(lpMap);
          uint32_t block_key = *(uint32_t*)fptr; fptr += sizeof(uint32_t);
          int block_size = *(int*)fptr; fptr += sizeof(int);
          
          if (block_key == 0x5E73CDD7) {
            TexturePackBlock block = {block_key, block_size, pos};
            auto it = m_TexturePackIndexMap.find(flag);
            if (it != m_TexturePackIndexMap.end()) {
              it->second.insert({std::to_string(blocks.size()), (TexturePackEntry)block});
              std::cout << "Added: " << block << std::endl;
            } else {
              TexturePackIndex index;
              index.insert({std::to_string(blocks.size()), (TexturePackEntry)block});
              m_TexturePackIndexMap.insert({flag, index});
              std::cout << "Added: " << block << std::endl;
            }
            blocks.push_back(block);
          }
          else if (block_key == 0xCDBFA090) {
              int start = (int)pos;
              fptr = static_cast<char*>(lpMap) + start + 0x44;

              char namebytes[0x20];
              std::memcpy(namebytes, fptr, sizeof(namebytes));
              std::string name = std::string(namebytes).substr(0, strlen(namebytes));
              fptr += sizeof(namebytes);

              fptr = static_cast<char *>(lpMap) + start + 0x6c;
              char form = *(char *)fptr;
              fptr += sizeof(char);

              if (form >= static_cast<char>(TextureResourceFormat::INVALID)) {
                std::cout << name << " has an unrecognized format? Skipping." << std::endl;
                fptr = static_cast<char *>(lpMap) + pos + block_size + 0x10;
                continue;
              }

              fptr = static_cast<char *>(lpMap) + start + 0x74;
              short wid = *(short *)fptr;
              fptr += sizeof(short);
              short hig = *(short *)fptr;
              fptr += sizeof(short);

              fptr = static_cast<char *>(lpMap) + start + 0x88;
              int size = *(int *)fptr;
              fptr += sizeof(int);

              fptr = static_cast<char *>(lpMap) + start + 0x90;
              uint32_t off = *(uint32_t *)fptr;
              fptr += sizeof(uint32_t);

              TexturePackResource res = {name, 0, size, wid, hig, static_cast<TextureResourceFormat>(form)};
              res.Offset = blocks.empty() ? off : blocks.back().Offset + off;
              
              auto it = m_TexturePackIndexMap.find(flag);
              if (it != m_TexturePackIndexMap.end()) {
                it->second.insert({res.Name, (TexturePackEntry)res});
                std::cout << "Added: " << res << std::endl;
              } else {
                TexturePackIndex index;
                index.insert({res.Name, (TexturePackEntry)res});
                m_TexturePackIndexMap.insert({flag, index});
                std::cout << "Added: " << res << std::endl;
              }
              resources.push_back(res);
          }
          else {
              neither += 1;
          }
          fptr = static_cast<char *>(lpMap) + pos + block_size + 0x10;
      }
      std::cout << "TexturePackStats: " << "Files: " << resources.size() << " Blocks: " << blocks.size() << " Neither: " << neither << std::endl;

      return true;
  }
  bool DumpTexturePack(TexturePackFlags flag) {
      std::filesystem::path out_dir = m_PathPrefix / FlagToBasename.at(flag);
      if (std::filesystem::exists(out_dir)) {
          std::cout << "Error out dir (" << out_dir.generic_string() << ")" << " already exists!" << std::endl;
          return false;
      }
      std::filesystem::create_directory(out_dir); //can err
      std::cout << "Dumping to " << out_dir.generic_string() << "..." << std::endl;
      
      const TexturePackIndex &tpi = m_TexturePackIndexMap.at(flag);
      for (const auto &entry : tpi) {
          if (std::holds_alternative<TexturePackResource>(entry.second)) {
              const auto& res = std::get<TexturePackResource>(entry.second);
              std::filesystem::path file_path = out_dir / (res.Name + ".png");
              //std::ofstream file; // if (!ofs) ...
              //file.open(file_path, std::ios::binary);
              if (res.Format != TextureResourceFormat::A8R8G8B8) {
                //  std::vector<unsigned char> buf(124);
                //  unsigned char data1[] = {0x44, 0x44, 0x53, 0x20, 0x7C, 0x00,
                //                          0x00, 0x00, 0x07, 0x10, 0x08, 0x00};
                //  size_t data1_size = sizeof(data1) / sizeof(data1[0]);
                //
                //  memcpy(buf.data(), data1, data1_size);
                //  //file.write((const char *)data1, data1_size);
                //
                //  //file.write(reinterpret_cast<const char *>(&res.Height), sizeof(res.Height));
                //  memcpy(buf.data() + data1_size, reinterpret_cast<const char *>(&res.Height),
                //  sizeof(res.Height));
                //  //file.write(reinterpret_cast<const char *>(&res.Width), sizeof(res.Width));
                //  memcpy(buf.data() + data1_size + sizeof(res.Width),
                //         reinterpret_cast<const char *>(&res.Width),
                //         sizeof(res.Width));
                //
                //  uint32_t dw = 0x0400;
                //  //file.write(reinterpret_cast<const char *>(&dw), sizeof(dw));
                //  //memcpy(buf.data())
                //  unsigned char data2[] = {
                //      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                //      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                //      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                //      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                //      0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                //      0x44, 0x58, 0x54};
                //  size_t data2_size = sizeof(data2) / sizeof(data2[0]);
                //
                //  file.write((const char *)data2, data2_size);
                //
                //  if (res.Format == TextureResourceFormat::DXT1) {
                //    unsigned char b = 0x35;
                //    file.write(reinterpret_cast<const char *>(&b), sizeof(b));
                //  } else if (res.Format == TextureResourceFormat::DXT3) {
                //    unsigned char b = 0x35;
                //    file.write(reinterpret_cast<const char *>(&b), sizeof(b));
                //  } else if (res.Format == TextureResourceFormat::DXT5) {
                //    unsigned char b = 0x35;
                //    file.write(reinterpret_cast<const char *>(&b), sizeof(b));
                //  } // else ...
                //
                //  unsigned char data3[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                //                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                //                           0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
                //                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                //                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                //  size_t data3_size = sizeof(data3) / sizeof(data3[0]);
                //
                //  file.write((const char *)data3, data3_size);
                //  file.close();
                //
                //  const LPVOID lpMap = m_TexturePackFileMap.at(flag).GetMapView();
                //  char *fptr = static_cast<char *>(lpMap);
                //  char *start = fptr + res.Offset;
                //
                //  //fpng::decode
                //  //fpng::fpng_encode_image_to_file(file_path.generic_string().c_str(), (void
                //  *)start,
                //  //                                res.Width, res.Height, 3,
                //  fpng::FPNG_FORCE_UNCOMPRESSED);
                //
              
              } else {
                TexturePackType type = GetType(m_TexturePackFileMap.at(flag));
                LPVOID lpMap = NULL;
                bool allgood = true;
                if (type == TexturePackType::PERM) {
                  lpMap = GetPerm(m_TexturePackFileMap.at(flag)).GetMapView();
                } else if (type == TexturePackType::TEMP) {
                  lpMap = GetTemp(m_TexturePackFileMap.at(flag)).GetMapView();
                } else {
                  std::cerr << "Invalid TexturePackType" << std::endl;
                  allgood = false;
                }
                if (allgood) {
                  char *fptr = static_cast<char *>(lpMap);
                  char *start = fptr + res.Offset;
                  fpng::fpng_encode_image_to_file(file_path.generic_string().c_str(), start,
                                                  res.Width, res.Height, 4,
                                                  fpng::FPNG_FORCE_UNCOMPRESSED);
                }
              }
          }
      }
      return true;
  }

private:
  TexturePackField m_Field;
  std::filesystem::path m_PathPrefix;
  std::unordered_map<TexturePackFlags, TexturePack> m_TexturePackFileMap;
  std::unordered_map<TexturePackFlags, TexturePackIndex> m_TexturePackIndexMap;
};

static void Initialize(std::filesystem::path path, TexturePackFlags flags = TexturePackFlags::All) {
  ctx::Instance()->SetPathPrefix(path);
  ctx::Instance()->SetField(flags);
}

static bool MemoryMapTexturePacks(void) {
  TexturePackField field = ctx::Instance()->GetField();
  for (uint8_t i = 0; i < TexturePackCount; ++i) {
    TexturePackFlags flag = (TexturePackFlags)(1 << i);
    if (field.HasFlag(flag)) {
      std::cout << "Mapping " << FlagToBasename.at(flag) << "..." << std::flush;
      bool success = ctx::Instance()->MemoryMapTexturePack(
          flag, converter().from_bytes(FlagToBasename.at(flag)));
    }
  }
  return true;
}

static bool MemoryMapAndIndexTexturePacks(void) {
  TexturePackField field = ctx::Instance()->GetField();
  for (uint8_t i = 0; i < TexturePackCount; ++i) {
    TexturePackFlags flag = (TexturePackFlags)(1 << i);
    if (field.HasFlag(flag)) {
      std::cout << "Mapping " << FlagToBasename.at(flag) << "..." << std::flush;
      ctx::Instance()->MemoryMapTexturePack(flag, converter().from_bytes(FlagToBasename.at(flag)));
      std::cout << "Indexing " << FlagToBasename.at(flag) << "..." << std::flush;
      ctx::Instance()->IndexTexturePack(flag);
    }
  }
  return true;
}

static bool IndexTexturePacks(void) {
  TexturePackField field = ctx::Instance()->GetField();
  for (uint8_t i = 0; i < TexturePackCount; ++i) {
    TexturePackFlags flag = (TexturePackFlags)(1 << i);
    if (field.HasFlag(flag)) {
      ctx::Instance()->IndexTexturePack(flag);
    }
  }
  return true;
}

static bool DumpTexturePacks(void) {
    TexturePackField field = ctx::Instance()->GetField();
    for (uint8_t i = 0; i < TexturePackCount; ++i) {
        TexturePackFlags flag = (TexturePackFlags)(1 << i);
        if (field.HasFlag(flag)) {
            ctx::Instance()->DumpTexturePack(flag);
        }
    }
    return true;
}
} // namespace mcctp