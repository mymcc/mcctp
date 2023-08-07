#pragma once

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

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
#include <assert.h>
#include <format>
#include <functional>

#include <codecvt>
#include <locale>

#include "GL/glew.h"
#include "GL/wglew.h"

#include "fpng.h"

namespace mcctp {
enum class ResultType { Error, Warn, ok };

struct Result {
private:
  ResultType Type;
  std::string Msg;

public:
  Result() : Type(ResultType::Error), Msg("Not Implemented Error!") {}
  Result(ResultType type, std::string msg) : Type(type), Msg(msg) {}
  explicit operator bool() const { return Type == ResultType::ok; }
  const ResultType &GetType(void) const { return Type; }
  const std::string &what(void) const { return Msg; }
};

static const std::unordered_map<ResultType, std::string> ResultTypeToString {
    {ResultType::Error, "Error"},
    {ResultType::Warn, "Warn"},
    {ResultType::ok, "Ok"},
};

struct FileMapping {
private:
  HANDLE hFile = INVALID_HANDLE_VALUE;
  HANDLE hMap = NULL;
  LPVOID lpMap = NULL;
  size_t Size = 0;

public:
  explicit FileMapping(std::wstring path, 
      uint64_t file_flags = GENERIC_READ|GENERIC_WRITE,
      uint64_t page_flags = PAGE_READWRITE,
      uint64_t view_flags = FILE_MAP_READ | FILE_MAP_WRITE);

  FileMapping(FileMapping &&other) noexcept;
  FileMapping &operator=(FileMapping &&other) noexcept;

  FileMapping(const FileMapping &) = delete;
  FileMapping &operator=(const FileMapping &) = delete;

  ~FileMapping();

  HANDLE GetFile(void) const { return hFile; }
  HANDLE GetFileMap(void) const { return hMap; }
  LPVOID GetMapView(void) const { return lpMap; }
  size_t GetSize(void) const { return Size; }

private:
  bool TryCreateMapping(std::wstring path, uint64_t file_flags, uint64_t page_flags,
                        uint64_t view_flags);
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
  None = 1 << 13,
};

static const std::unordered_map<TexturePackFlags, std::string> FlagToBasename{
    { TexturePackFlags::Controller, "controllertexturepack"},
    { TexturePackFlags::Emblems, "emblemstexturepack"},
    { TexturePackFlags::GlobalUI, "globaluitexturepack"},
    { TexturePackFlags::Hopper, "hoppertexturepack"},
    { TexturePackFlags::InGameChapter, "ingamechapterpack"},
    { TexturePackFlags::LargeAvatar, "largeavatartexturepack"},
    { TexturePackFlags::Levels, "levelstexturepack"},
    { TexturePackFlags::Loading, "loadingtexturepack"},
    { TexturePackFlags::MainMenuAndCampaign, "mainmenuandcampaigntexturepack"},
    { TexturePackFlags::MainMenu, "mainmenutexturepack"},
    { TexturePackFlags::Medals, "medalstexturepack"},
    { TexturePackFlags::Skulls, "skullstexturepack"},
    { TexturePackFlags::SPMapPreview, "spmappreviewtexturepack"},
    { TexturePackFlags::All, "all"},
    { TexturePackFlags::None, "none"},
};

static const std::unordered_map<std::string, TexturePackFlags> BasenameToFlag{
    { "controllertexturepack", TexturePackFlags::Controller},
    { "emblemstexturepack", TexturePackFlags::Emblems},
    { "globaluitexturepack", TexturePackFlags::GlobalUI},
    { "hoppertexturepack", TexturePackFlags::Hopper},
    { "ingamechapterpack", TexturePackFlags::InGameChapter},
    { "largeavatartexturepack", TexturePackFlags::LargeAvatar},
    { "levelstexturepack", TexturePackFlags::Levels},
    { "loadingtexturepack", TexturePackFlags::Loading},
    { "mainmenuandcampaigntexturepack", TexturePackFlags::MainMenuAndCampaign},
    { "mainmenutexturepack", TexturePackFlags::MainMenu},
    { "medalstexturepack", TexturePackFlags::Medals},
    { "skullstexturepack", TexturePackFlags::Skulls},
    { "spmappreviewtexturepack", TexturePackFlags::SPMapPreview},
    { "all", TexturePackFlags::All},
    { "none", TexturePackFlags::None},
};

static const std::unordered_map<TexturePackFlags, int> FlagToFileCount{
    { TexturePackFlags::Controller, 47},
    { TexturePackFlags::Emblems, 567},
    { TexturePackFlags::GlobalUI, 304},
    { TexturePackFlags::Hopper, 106},
    { TexturePackFlags::InGameChapter, 494},
    { TexturePackFlags::LargeAvatar, 236},
    { TexturePackFlags::Levels, 536},
    { TexturePackFlags::Loading, 228},
    { TexturePackFlags::MainMenuAndCampaign, 993},
    { TexturePackFlags::MainMenu, 748},
    { TexturePackFlags::Medals, 275},
    { TexturePackFlags::Skulls, 41},
    { TexturePackFlags::SPMapPreview, 142},
    { TexturePackFlags::All, 4717},
    { TexturePackFlags::None, 0},
};

enum class ResourceFormat {
  A8R8G8B8,
  DXT1,
  DXT3,
  DXT5,
  INVALID,
};

static const std::unordered_map<ResourceFormat, std::string> ResourceFormatToString{
    {ResourceFormat::A8R8G8B8, "ARGB"},
    {ResourceFormat::DXT1, "DXT1"},
    {ResourceFormat::DXT3, "DXT3"},
    {ResourceFormat::DXT5, "DXT5"},
    {ResourceFormat::INVALID, ""}};

struct TexturePackBlock {
  size_t Key;
  size_t Size;
  size_t Offset;

  friend std::ostream &operator<<(std::ostream &os, const TexturePackBlock &block);
};

/* PERM == Resource data is in .perm.bin | TEMP == Resource data is in .temp.bin */
enum class TexturePackType { PERM, TEMP, INVALID };

static const std::unordered_map<TexturePackType, std::string> PackTypeToExt{
    {TexturePackType::PERM, ".perm.bin"},
    {TexturePackType::TEMP, ".temp.bin"},
    {TexturePackType::INVALID, ""}};

struct TexturePackResource {
  std::string Name;
  char *Origin;
  size_t Offset;
  size_t Size;
  int32_t Width;
  int32_t Height;
  ResourceFormat Format;
  TexturePackType PackType;
  TexturePackFlags TexturePackFlag; //

  bool Fits(const TexturePackResource &other) const;
  friend std::ostream &operator<<(std::ostream &os, const TexturePackResource &res);
  friend bool operator==(const TexturePackResource& lhs, const TexturePackResource& rhs) {
    return (lhs.Name == rhs.Name) && (lhs.TexturePackFlag == rhs.TexturePackFlag);
  }
};

typedef const TexturePackResource *const ManagedResource;

typedef std::tuple<FileMapping, FileMapping, TexturePackType> TexturePack;
static const FileMapping &GetPerm(const TexturePack &tp) { return std::get<0>(tp); }
static const FileMapping &GetTemp(const TexturePack &tp) { return std::get<1>(tp); }
static TexturePackType GetType(const TexturePack &tp) { return std::get<2>(tp); }

typedef std::variant<TexturePackResource, TexturePackBlock> TexturePackEntry;

typedef std::unordered_map<std::string, TexturePackEntry> TexturePackIndex;

struct TexturePackField {
private:
  uint16_t Value;

public:
  TexturePackField(uint16_t value = TexturePackFlags::All) : Value(value) {}
  void SetFlag(TexturePackFlags flags) { Value |= (int)flags; }
  void UnsetFlag(TexturePackFlags flags) { Value &= ~(int)flags; }
  void FlipFlag(TexturePackFlags flags) { Value ^= (int)flags; }
  bool HasFlag(TexturePackFlags flags) { return (Value & (int)flags) == (int)flags; }
  bool HasAnyFlag(TexturePackFlags flags) { return (Value & (int)flags) != 0; }
  operator uint16_t() const { return Value; }
  uint16_t to_uint16() const { return Value; }
};

enum class DumpFormatFlags { Native, EncodeToPNG };

enum class DumpCompressionFlags {
  Fastest,
  Smallest,
  None,
};

enum class SourceType { Internal, External, Invalid };

static const std::unordered_map<SourceType, std::string> SourceTypeToString {
    {SourceType::Internal, "Internal"},
    {SourceType::External, "External"},
    {SourceType::Invalid,  "Invalid"},
};

typedef std::variant<ManagedResource, std::filesystem::path> SourceProvider;

enum class TexturePackInjectFlags {
    Strict,
    None
};

struct PatchSource {
private:
  std::string ID;
  SourceType Type;
  SourceProvider Provider;

public:
  explicit PatchSource(const std::string &id, const std::filesystem::path &path,
                       const SourceType &type);

  // Copy constructor and copy assignment operator
  PatchSource(const PatchSource &) = default;
  PatchSource &operator=(const PatchSource &) = default;

  ~PatchSource();

  const std::string &GetID(void) const { return ID; }
  const SourceType &GetType(void) const { return Type; }
  const SourceProvider &GetProvider(void) const { return Provider; }

  //move to .cpp
  friend bool operator==(const PatchSource &lhs, const PatchSource &rhs) {
    return lhs.GetID() == rhs.GetID();
  }

  friend bool operator==(const PatchSource &lhs, const ManagedResource &rhs) {
    if (lhs.GetType() == SourceType::Internal) {
      const auto &provider = std::get<ManagedResource>(lhs.GetProvider());
      return provider == rhs;
    } else {
      return false;
    }
  }

private:
  SourceProvider InitializeProvider(const std::filesystem::path &path, const SourceType &type);
};
} // namespace mcctp

namespace std {
template <> struct hash<mcctp::PatchSource> {
  std::size_t operator()(const mcctp::PatchSource &ps) const {
    return std::hash<std::string>{}(ps.GetID());
  }
};
} // namespace std

namespace mcctp {
typedef std::unordered_map<TexturePackFlags, TexturePack> TexturePackFileMap;
typedef std::unordered_map<TexturePackFlags, TexturePackIndex> TexturePackIndexMap;
typedef std::unordered_map<std::string, TexturePackFlags> ResourceToTextureMap;
typedef std::unordered_map<PatchSource, ManagedResource> PatchMap;

class ctx {
public:
  static ctx* Instance();
  
  void SetField(uint16_t field = (uint16_t)TexturePackFlags::All) {
      m_Field = (TexturePackField)field;
  }
  TexturePackField GetField(void) const { return m_Field; }
  void SetPathPrefix(std::filesystem::path path_prefix) { m_PathPrefix = path_prefix; }

  bool MemoryMapTexturePack(TexturePackFlags flag, std::wstring texture_pack_basename);
  void UnmapTexturePack(TexturePackFlags flag);
  size_t GetMappedTexturePackCount(void) const { return m_TexturePackFileMap.size(); }
  bool IsTexturePackMapped(TexturePackFlags flag) const;

  bool _IndexTexturePack(TexturePackFlags flag);
  bool IndexTexturePack(TexturePackFlags flag);
  size_t GetIndexedTexturePackCount(void) const { return m_TexturePackIndexMap.size(); }
  bool IsTexturePackIndexed(TexturePackFlags flag) const; 

  bool DumpTexturePack(TexturePackFlags flag, DumpFormatFlags format_flag = DumpFormatFlags::Native,
                       DumpCompressionFlags compression_flag = DumpCompressionFlags::None);
  bool DeleteTexturePackDump(TexturePackFlags flag);

  bool InjectAndWritePack(std::string src, std::string dst, TexturePackFlags flag);
  TexturePackResource GetMappedResource(std::string res_name);
  std::optional<ManagedResource> GetManagedResource(std::string res_name);

  Result VerifyPatchMap(const PatchMap &map);
  Result SetCurrentPatchMap(PatchMap map);
  Result InjectCurrentPatchMapAndExport(TexturePackInjectFlags flags);
  Result DumpCurrentPatchMap(const std::filesystem::path &path);
  Result DumpPatchMap(PatchMap map);

  std::string GetTexturePackFilenameForResource(TexturePackResource res);
  std::vector<TexturePackResource> GetResourcesFromTexturePack(TexturePackFlags flag) const;
  TexturePackFlags GetTexturePackFlagFromResourceName(std::string name) const;
  GLuint RenderDDSToTexture(TexturePackResource res);
  bool ShareWithWGLContext(HGLRC hrc = wglGetCurrentContext());

  void SetDumpCallback(std::function<void()> callback);
  void SetDumpEndCallback(std::function<void()> callback);

private:
  void InitializeGLContext(void);

private:
  TexturePackField m_Field;
  std::filesystem::path m_PathPrefix;

  TexturePackFileMap m_TexturePackFileMap; /* Texture pack managed file mappings */
  TexturePackIndexMap m_TexturePackIndexMap; /* Resource index for a mapped texture pack */
  ResourceToTextureMap m_ResourceToTexturePackMap; /* Reverse resource index for a mapped texture pack (reduces patch time complexity) */
  PatchMap m_CurrentPatchMap;

  HDC m_hDC;
  HGLRC m_hRC;
  bool m_ClientHasWGLContext;
  GLuint m_OutputVertexShader;
  GLuint m_OutputFragmentShader;
  GLuint m_OutputProgram;

private:
    std::function<void()> m_DumpCallback;
    std::function<void()> m_DumpEndCallback;
};

void Initialize(std::filesystem::path path, TexturePackFlags flags = TexturePackFlags::All);

bool MemoryMapTexturePacks(void);
void UnmapTexturePacks(void);

bool IndexTexturePacks(void);
bool MemoryMapAndIndexTexturePacks(void);
void SetDumpCallback(std::function<void()> callback);
void SetDumpEndCallback(std::function<void()> callback);
bool DumpTexturePacks(DumpFormatFlags format_flag = DumpFormatFlags::Native,
                      DumpCompressionFlags compression_flag = DumpCompressionFlags::None);
void ClearTexturePackDumps(void);

int GetStagedDumpFileCount(void);

static bool SetPatchMap() { return false; }


bool InjectResource(std::string src, std::string dst, TexturePackFlags flag);
std::stringstream BuildDDSHeaderForResource(TexturePackResource res);
GLuint RenderDDSToTexture(TexturePackResource res);
bool ShareWithWGLContext(HGLRC hrc = wglGetCurrentContext());
} // namespace mcctp