#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

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

struct TexturePackResource {
  std::string Name;
  size_t Offset;
  size_t Size;
};

typedef std::unordered_map<std::string, TexturePackResource> TexturePackIndex;

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
    return Context;
  }
  void SetField(uint16_t field = (uint16_t)TexturePackFlags::All) {
    m_Field = (TexturePackField)field;
  }
  TexturePackField GetField(void) const { return m_Field; }
  void SetPathPrefix(std::filesystem::path path_prefix) { m_PathPrefix = path_prefix; }
  bool MemoryMapTexturePack(TexturePackFlags flag, std::wstring texture_pack_basename) {
    try {
      m_TexturePackFileMap.insert(
          {flag, std::move(FileMapping(m_PathPrefix.wstring() + L'\\' + texture_pack_basename +
                                       L".perm.bin"))});
      return true;
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  }
  bool IndexTexturePack(TexturePackFlags flag) {
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

private:
private:
  TexturePackField m_Field;
  std::filesystem::path m_PathPrefix;
  std::unordered_map<TexturePackFlags, FileMapping> m_TexturePackFileMap;
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
} // namespace mcctp