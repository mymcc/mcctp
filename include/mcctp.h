#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

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

#include <codecvt>
#include <locale>

#include "GL/glew.h"
#include "GL/wglew.h"

#include "fpng.h"

namespace mcctp {
struct FileMapping {
private:
  HANDLE hFile = INVALID_HANDLE_VALUE;
  HANDLE hMap = NULL;
  LPVOID lpMap = NULL;

public:
  explicit FileMapping(std::wstring path);

  FileMapping(FileMapping&& other) noexcept;
  FileMapping& operator=(FileMapping&& other) noexcept;

  FileMapping(const FileMapping &) = delete;
  FileMapping &operator=(const FileMapping &) = delete;

  ~FileMapping();

  HANDLE GetFile(void) const { return hFile; }
  HANDLE GetFileMap(void) const { return hMap; }
  LPVOID GetMapView(void) const { return lpMap; }

private:
    bool TryCreateMapping(std::wstring path);
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

static const std::unordered_map<TexturePackFlags, const char *> FlagToBasename{
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

enum class ResourceFormat {
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

  friend std::ostream& operator<<(std::ostream& os, const TexturePackBlock& block);
};

struct TexturePackResource {
  std::string Name;
  size_t Offset;
  size_t Size;
  int32_t Width;
  int32_t Height;
  ResourceFormat Format;

  friend std::ostream& operator<<(std::ostream& os, const TexturePackResource& res);
};

/* PERM == Resource data is in .perm.bin | TEMP == Resource data is in .temp.bin */
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

enum class OutputFlags {
    NativeFormat,
    EncodeToPNG
};

class ctx {
public:
  static ctx* Instance();
  
  void SetField(uint16_t field = (uint16_t)TexturePackFlags::All) {
      m_Field = (TexturePackField)field;
  }

  TexturePackField GetField(void) const { return m_Field; }
  void SetPathPrefix(std::filesystem::path path_prefix) { m_PathPrefix = path_prefix; }
  bool MemoryMapTexturePack(TexturePackFlags flag, std::wstring texture_pack_basename);
  bool _IndexTexturePack(TexturePackFlags flag);
  bool IndexTexturePack(TexturePackFlags flag);
  bool DumpTexturePack(TexturePackFlags flag, OutputFlags format_flag = OutputFlags::NativeFormat);

private:
  TexturePackField m_Field;
  std::filesystem::path m_PathPrefix;
  std::unordered_map<TexturePackFlags, TexturePack> m_TexturePackFileMap;
  std::unordered_map<TexturePackFlags, TexturePackIndex> m_TexturePackIndexMap;
  GLuint vertexShader;
  GLuint fragmentShader;
  GLuint program;
};

void Initialize(std::filesystem::path path, TexturePackFlags flags = TexturePackFlags::All);
bool MemoryMapTexturePacks(void);
bool MemoryMapAndIndexTexturePacks(void);
bool IndexTexturePacks(void);
bool DumpTexturePacks(OutputFlags format_flag = OutputFlags::NativeFormat);
} // namespace mcctp