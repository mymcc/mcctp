#include "mcctppch.h"
#include "StreamedImage.h"

extern HGLRC g_hRC;
void DoTexturePackCheckbox(mcctp::TexturePackFlags texture_pack, bool* active);
void DoAllTexturePackCheckboxes(void);

mcctp::StreamedImage::StreamedImage(std::filesystem::path path) {
  m_TexturePacksPath = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  m_TexturePacksPath += "/ui/texturepacks";

  m_DumpFormatFlag = mcctp::DumpFormatFlags::Native;
  m_CompressionFlag = mcctp::DumpCompressionFlags::None;

  mcctp::Initialize(m_TexturePacksPath, mcctp::TexturePackFlags::All);
}

void mcctp::StreamedImage::OnAttach() {
	
}

void mcctp::StreamedImage::OnUpdate(float ts) {}

void mcctp::StreamedImage::OnUIRender() {
    ImGui::Begin("Instance Configuration");

    DoInstanceConfiguration();
    DoTexturePackDumper();

    GLint texture_count;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texture_count);
    ImGui::Text(std::to_string(texture_count).c_str());

    ImGui::End();

    DoTextureViewer();

}

void mcctp::StreamedImage::DoInstanceConfiguration(void) {
    ImGui::SeparatorText("Configuration");
    DoAllTexturePackCheckboxes();

    ImGui::SeparatorText("Modify Instance");
    
    mcctp::ctx *inst = ctx::Instance();
    bool can_mem_map = inst->GetField() != TexturePackFlags::None;

    if (!can_mem_map)
      ImGui::BeginDisabled();

    if (ImGui::Button("MemMap TexturePacks")) {
      mcctp::MemoryMapTexturePacks();
    }

    if (!can_mem_map)
      ImGui::EndDisabled();

    bool should_disable = inst->GetMappedTexturePackCount() < 1;

    if (should_disable)
      ImGui::BeginDisabled();

    if (ImGui::Button("Index TexturePacks")) {
      mcctp::IndexTexturePacks();
    }

    if (ImGui::Button("Unmap TexturePacks")) {
      mcctp::UnmapTexturePacks();
    }

    if (should_disable)
      ImGui::EndDisabled();
}

void mcctp::StreamedImage::DoTexturePackDumper(void) {
    mcctp::ctx *inst = ctx::Instance();
    bool can_dump = (inst->GetMappedTexturePackCount() == 
        inst->ctx::GetIndexedTexturePackCount()) && 
        (inst->GetMappedTexturePackCount() > 1);

    if (!can_dump)
        ImGui::BeginDisabled();

    if (ImGui::Button("Dump TexturePacks")) {
      mcctp::DumpTexturePacks(m_DumpFormatFlag, m_CompressionFlag);
    }

    if (!can_dump)
      ImGui::EndDisabled();

    if (ImGui::Button("Remove TexturePack Dumps")) {
      mcctp::ClearTexturePackDumps();
    }

    static int native = static_cast<int>(m_DumpFormatFlag);
    ImGui::SeparatorText("Dump Format");
    ImGui::RadioButton("Native", &native, 0);
    ImGui::RadioButton("PNG", &native, 1);
    m_DumpFormatFlag = static_cast<mcctp::DumpFormatFlags>(native);

    static int compress = static_cast<int>(m_CompressionFlag);
    ImGui::SeparatorText("PNG Compression");
    ImGui::RadioButton("Fastest", &compress, 0);
    ImGui::RadioButton("Smallest", &compress, 1);
    ImGui::RadioButton("None", &compress, 2);
    m_CompressionFlag = static_cast<mcctp::DumpCompressionFlags>(compress);
}

void mcctp::StreamedImage::DoTextureViewer(void) { 
    ImGui::Begin("TextureViewer");
    static unsigned int selected_thumbnail = 0;
    if (m_TextureThumbnails.size() != 0) {
      //auto texture = m_TextureThumbnails.at(selected_thumbnail);
      //ImGui::Image((ImTextureID)(intptr_t)texture->GetRendererID(),
      //             ImVec2(texture->GetWidth(), texture->GetHeight()));
      auto texture = m_TextureThumbnails.at(selected_thumbnail);
      
      ImGui::Image((ImTextureID)(intptr_t)texture->GetRendererID(),
                   ImVec2(640, 360));

    } else {
      ImGui::Image(
            (ImTextureID)(intptr_t)0,
            ImVec2(640, 360));
    }
    ImGui::BeginGroup();
    ImGuiStyle &style = ImGui::GetStyle();
    float child_w = (ImGui::GetContentRegionAvail().x - 2 * style.ItemSpacing.x) / 6;
    if (child_w < 1.0f)
      child_w = 1.0f;
    const ImGuiWindowFlags child_flags = ImGuiWindowFlags_MenuBar;
    ImGuiID child_id = ImGui::GetID((void *)(intptr_t)0);
    bool child_is_visible = ImGui::BeginChild(child_id, ImVec2(child_w, 300.0f), true, child_flags);
    if (ImGui::BeginMenuBar()) {
      ImGui::TextUnformatted("TexturePacks");
      ImGui::EndMenuBar();
    }

    static ImVec2 rect_pos{};
    static ImVec2 rect_size{};
    static bool needs_load = false;
    static mcctp::TexturePackFlags selected_texture_pack = mcctp::TexturePackFlags::None;
    for (uint8_t i = 0; i < TexturePackCount; ++i) {
      TexturePackFlags flag = (TexturePackFlags)(1 << i);
      mcctp::ctx *inst = ctx::Instance();
      if (inst->IsTexturePackMapped(flag) && inst->IsTexturePackIndexed(flag)) {
        ImVec2 last_rect_pos = ImGui::GetCursorScreenPos();
        ImVec2 last_rect_size = ImGui::GetItemRectSize();
        if (ImGui::Selectable(mcctp::FlagToBasename.at(flag))) {
          if (flag != selected_texture_pack) {
            for (auto &thumbnail : m_TextureThumbnails) {
              thumbnail.reset();
            }
            m_TextureThumbnails.clear();
            selected_thumbnail = 0;
            rect_pos = last_rect_pos;
            rect_size = last_rect_size;
            needs_load = true;
          }
          selected_texture_pack = flag;
        }
      }
    }
    
    ImU32 col = ImColor(ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
    ImGui::RenderFrame(
        rect_pos, ImVec2(rect_pos.x + ImGui::GetContentRegionMax().x, rect_pos.y + ImGui::GetTextLineHeight()),
        col, false);

    ImGui::EndChild();

    ImGui::SameLine();
    child_w = (ImGui::GetContentRegionAvail().x - 2 * style.ItemSpacing.x);
    if (child_w < 1.0f)
      child_w = 1.0f;
    child_id = ImGui::GetID((void *)(intptr_t)1);
    child_is_visible = ImGui::BeginChild(child_id, ImVec2(child_w, 300.0f), true, child_flags);
    if (ImGui::BeginMenuBar()) {
      ImGui::TextUnformatted("Textures");
      ImGui::EndMenuBar();
    }

    if ((selected_texture_pack != mcctp::TexturePackFlags::None) && needs_load) {
      mcctp::ctx *inst = ctx::Instance();
      unsigned int texture_unit = 0;
      for (const auto &res : inst->GetResourcesFromTexturePack(selected_texture_pack)) {
        auto thumbnail = std::make_shared<mcctp::Image>(res);

        thumbnail->Bind();
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        thumbnail->Unbind();

        m_TextureThumbnails.emplace_back(thumbnail);
        ++texture_unit;
      }
      needs_load = false;
    }

    int max_textures = 160;
    int buttons_count = (m_TextureThumbnails.size() == 0)      ? 0
                        : (m_TextureThumbnails.size() < max_textures) ? m_TextureThumbnails.size()
                                                               : max_textures;
    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    ImVec2 button_sz(40 * 1.975, 40);
    ImGuiStyle &style1 = ImGui::GetStyle();

    for (int n = 0; n < buttons_count; ++n) {
      ImGui::PushID(n);
      ImVec2 start = ImGui::GetCursorScreenPos();
      glBindTexture(GL_TEXTURE_2D, m_TextureThumbnails.at(n)->GetRendererID());
      if (ImGui::ImageButton((void *)(intptr_t)m_TextureThumbnails.at(n)->GetRendererID(),
                             button_sz))
      {
        selected_thumbnail = n;
      }
      float last_button_x2 = ImGui::GetItemRectMax().x;
      float next_button_x2 = last_button_x2 + style1.ItemSpacing.x +
                             button_sz.x; // Expected position if next button was on same line
      if (n + 1 < buttons_count && next_button_x2 < window_visible_x2)
        ImGui::SameLine();
      ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::EndGroup();
    ImGui::End();
}

void DoTexturePackCheckbox(mcctp::TexturePackFlags texture_pack, bool* active) {
    if (ImGui::Checkbox(mcctp::FlagToBasename.at(texture_pack), active)) {
      mcctp::TexturePackField field = mcctp::ctx::Instance()->GetField();
      if (*active) {
        field.SetFlag(texture_pack);
      } else {
        field.UnsetFlag(texture_pack);
      }
      mcctp::ctx::Instance()->SetField(field);
    }
}

void DoAllTexturePackCheckboxes() {
    // Curse you std::vector<bool>!!!
    static std::deque<bool> active_texture_packs(mcctp::TexturePackCount, true);
    for (uint8_t i = 0; i < mcctp::TexturePackCount; ++i) {
      mcctp::TexturePackFlags flag = (mcctp::TexturePackFlags)(1 << i);
      DoTexturePackCheckbox(flag, &(active_texture_packs[i]));
    }

    static bool all = true;
    if (ImGui::Checkbox("All", &all)) {
      for (auto &tp : active_texture_packs) {
          tp = all;
        if (all)
          mcctp::ctx::Instance()->SetField(mcctp::TexturePackFlags::All);
        else
          mcctp::ctx::Instance()->SetField(mcctp::TexturePackFlags::None);
      }
    }
}