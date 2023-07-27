#pragma once
#include <iostream>
#include <filesystem>
#include <unordered_map>

namespace mcctp {
	static constexpr uint8_t TexturePackCount = 13;

	enum TexturePackFlags {
		Controller          = 1 << 0,
		Emblems             = 1 << 1,
		GlobalUI            = 1 << 2,
		Hopper              = 1 << 3,
		InGameChapter       = 1 << 4,
		LargeAvatar         = 1 << 5,
		Levels              = 1 << 6,
		Loading             = 1 << 7,
		MainMenuAndCampaign = 1 << 8,
		MainMenu            = 1 << 9,
		Medals              = 1 << 10,
		Skulls              = 1 << 11,
		SPMapPreview        = 1 << 12,
		All = (Controller | Emblems | 
		GlobalUI | Hopper | InGameChapter | 
			LargeAvatar | Levels | Loading | 
			MainMenuAndCampaign | MainMenu | 
			Medals | Skulls | SPMapPreview),
	};

	static std::unordered_map<TexturePackFlags, const char*> FlagToBasename = {
		{ TexturePackFlags::Controller,          "controllertexturepack" },
		{ TexturePackFlags::Emblems,             "emblemstexturepack" },
		{ TexturePackFlags::GlobalUI,            "globaluitexturepack" },
		{ TexturePackFlags::Hopper,              "hoppertexturepack" },
		{ TexturePackFlags::InGameChapter,       "ingamechapterpack" },
		{ TexturePackFlags::LargeAvatar,         "largeavatartexturepack" },
		{ TexturePackFlags::Levels,              "levelstexturepack" },
		{ TexturePackFlags::Loading,             "loadingtexturepack" },
		{ TexturePackFlags::MainMenuAndCampaign, "mainmenuandcampaigntexturepack" },
		{ TexturePackFlags::MainMenu,            "mainmenutexturepack" },
		{ TexturePackFlags::Medals,              "medalstexturepack" },
		{ TexturePackFlags::Skulls,              "skullstexturepack" },
		{ TexturePackFlags::SPMapPreview,        "spmappreviewtexturepack" },
	};

	struct TexturePackField {
		uint16_t m_Field = 0;

		void SetFlag(TexturePackFlags flags) {
			m_Field |= (int)flags;
		}

		void UnsetFlag(TexturePackFlags flags) {
			m_Field &= ~(int)flags;
		}

		void FlipFlag(TexturePackFlags flags) {
			m_Field ^= (int)flags;
		}

		bool HasFlag(TexturePackFlags flags) {
			return (m_Field & (int)flags) == (int)flags;
		}

		bool HasAnyFlag(TexturePackFlags flags) {
			return (m_Field & (int)flags) != 0;
		}
	};

	class ctx {
	public:
		static ctx* Instance() {
			static ctx* Context = new ctx;
			return Context;
		}
		void SetField(uint16_t field = (uint16_t) TexturePackFlags::All) {
			m_Field = (TexturePackField) field;
		}
		TexturePackField GetField(void) const {
			return m_Field;
		}
		void SetPathPrefix(std::filesystem::path path_prefix) {
			m_PathPrefix = path_prefix;
		}

	private:
		TexturePackField m_Field;
		std::filesystem::path m_PathPrefix;
	};

	static void Initialize(std::filesystem::path path, TexturePackFlags flags = TexturePackFlags::All)
	{
		ctx::Instance()->SetPathPrefix(path);
		ctx::Instance()->SetField(flags);
	}

	static bool MemoryMapTextures(void)
	{
		TexturePackField field = ctx::Instance()->GetField();
		for (uint8_t i = 0; i < TexturePackCount; ++i) {
			TexturePackFlags flag = (TexturePackFlags)(1 << i);
			if (field.HasFlag(flag)) {
				std::cout << FlagToBasename.at(flag) << std::endl;
			}
		}
		return true;
	}
}