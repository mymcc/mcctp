#include "mcctppch.h"
#include "StreamedImage.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
extern HGLRC g_hRC;

std::shared_ptr<mcctp::Image> LoadAppIcon(void);
void DoTexturePackCheckbox(mcctp::TexturePackFlags texture_pack, bool *active);
void DoAllTexturePackCheckboxes(void);
std::pair<ImVec2, ImVec2> GetScreenFill(void);
std::pair<ImVec2, ImVec2> GetScreenMaintainAspectRatio(uint32_t width, uint32_t height);
std::pair<ImVec2, ImVec2> GetItemRectMaintainAspectRatio(uint32_t width, uint32_t height, ImVec2 item_extents);

mcctp::StreamedImage::StreamedImage(std::filesystem::path path) {
  m_TexturePacksPath = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  m_TexturePacksPath += "/ui/texturepacks";

  m_DumpFormatFlag = mcctp::DumpFormatFlags::Native;
  m_CompressionFlag = mcctp::DumpCompressionFlags::None;

  mcctp::Initialize(m_TexturePacksPath, mcctp::TexturePackFlags::All);

  std::filesystem::path app_icon =
      std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
  app_icon += "\\res\\mcctp.png";

  std::vector<uint8_t> data;
  uint32_t w;
  uint32_t h;
  uint32_t channels = 4;
  uint32_t desired_channels = 4;
  // fpng::fpng_decode_file(app_icon.generic_string().c_str(), data, w, h, channels,
  // desired_channels);

  // Load from file
  int image_width = 0;
  int image_height = 0;
  unsigned char *image_data =
      stbi_load(app_icon.generic_string().c_str(), &image_width, &image_height, NULL, 4);
  m_AppIcon = std::make_shared<mcctp::Image>(image_data, image_width, image_height, 4);
  if (image_data != NULL)
    stbi_image_free(image_data);
}

void mcctp::StreamedImage::OnAttach() {
	
}

void mcctp::StreamedImage::OnUpdate(float ts) {}

void mcctp::StreamedImage::OnUIRender() {
    
    if (show_demo) {
      ImGui::ShowDemoWindow();
    }

    DoMainMenuBar();

    
    ImGui::Begin("Instance Configuration");

    DoInstanceConfiguration();
    DoTexturePackDumper();

    GLint texture_count;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &texture_count);
    ImGui::Text(std::to_string(texture_count).c_str());

    ImGui::End();

    DoTextureViewer();

    DoTextureInfo();
}

void mcctp::StreamedImage::DoMainMenuBar(void) {
    auto& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x,5 * style.FramePadding.y));
    ImGui::BeginMainMenuBar();
    ImGui::PopStyleVar();
    m_AppIcon->Bind();
    ImGui::Image((void *)(intptr_t)m_AppIcon->GetRendererID(), ImVec2(80, 50));
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Show Demo")) {
        show_demo = true;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Project")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Window")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Help")) {
      ImGui::MenuItem("New");
      ImGui::MenuItem("Create");
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
    

    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    float height = ImGui::GetFrameHeight();

    if (ImGui::BeginViewportSideBar("##SecondaryMenuBar", NULL, ImGuiDir_Up, height,
                                    window_flags)) {
      if (ImGui::BeginMenuBar()) {
        ImGui::Text("Happy secondary menu bar");
        ImGui::EndMenuBar();
      }
    }
    ImGui::End();

    if (ImGui::BeginViewportSideBar("##TernaryMenuBar", NULL, ImGuiDir_Up, height,
                                    window_flags)) {
      if (ImGui::BeginMenuBar()) {
        ImGui::Text("Happy ternary menu bar");
        ImGui::EndMenuBar();
      }
    }
    ImGui::End();

    if (ImGui::BeginViewportSideBar("##MainStatusBar", NULL, ImGuiDir_Down, height, window_flags)) {
      if (ImGui::BeginMenuBar()) {
        ImGui::Text("Happy status bar");
        ImGui::EndMenuBar();
      }
    }
    ImGui::End();

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
    ImGui::Begin("Texture Viewer");
    static float scale = 1.0f;
    ImGui::SliderFloat("Scale", &scale, 1.0f, 20.0f);
    if (m_TextureThumbnails.size() != 0) {
      //auto texture = m_TextureThumbnails.at(selected_thumbnail);
      //ImGui::Image((ImTextureID)(intptr_t)texture->GetRendererID(),
      //             ImVec2(texture->GetWidth(), texture->GetHeight()));

      auto texture = m_TextureThumbnails.at(selected_thumbnail);
      std::pair<ImVec2, ImVec2> window_coordinates;
      window_coordinates =
          (m_MaintainAspectRatio) ? GetScreenMaintainAspectRatio(texture->GetWidth(), texture->GetHeight()) : GetScreenFill();

      //ImGui::Image((ImTextureID)(intptr_t)texture->GetRendererID(),
      //             ImVec2(640, 360));
      float w = (window_coordinates.second.x - window_coordinates.first.x);
      float h = (window_coordinates.second.y - window_coordinates.first.y);
      float xx = ((w * scale) - w) / 2.0f;
      float yy = ((h * scale) - h) / 2.0f;
      window_coordinates.first.x -= xx;
      window_coordinates.second.x += xx;
      window_coordinates.first.y -= yy;
      window_coordinates.second.y += yy;

      ImVec2 strt = ImGui::GetCursorPos();
      ImVec2 rect = ImGui::GetContentRegionAvail();
      ImVec2 end(ImGui::GetWindowSize());
      ImVec2 spos = ImGui::GetCursorScreenPos();
      rect.x += spos.x;
      rect.y += spos.y;
      ImGui::GetWindowDrawList()->AddRect(spos, rect, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.7f, 1.0f)));
      ImGui::GetWindowDrawList()->PushClipRect(spos, rect, false);
      ImGui::GetWindowDrawList()->AddImage((void *)texture->GetRendererID(),
                                           window_coordinates.first, window_coordinates.second,
          ImVec2(0 ,0), ImVec2(1, 1));
      //ImVec2(0 - (1 - scale), 0 - (1 - scale)), ImVec2(1 + (1 - scale), 1 + (1 - scale)));
      ImGui::GetWindowDrawList()->PopClipRect();

    } else {
      ImGui::Image(
            (ImTextureID)(intptr_t)0,
            ImVec2(640, 360));
    }

    ImGui::End();

    ImGui::Begin("Texture Explorer");
    ImGui::BeginGroup();
    ImGuiStyle &style = ImGui::GetStyle();
    
    float child_w = (ImGui::GetContentRegionAvail().x - 2 * style.ItemSpacing.x) / 6;
    if (child_w < 1.0f)
      child_w = 1.0f;
    child_w = ImGui::CalcTextSize("mainmenuandcampaigntexturepack").x + style.ScrollbarSize + (2 * style.WindowPadding.x);
    const ImGuiWindowFlags child_flags = ImGuiWindowFlags_MenuBar;
    ImGuiID child_id = ImGui::GetID((void *)(intptr_t)0);
    bool child_is_visible = ImGui::BeginChild(child_id, ImVec2(child_w, ImGui::GetContentRegionAvail().y), true, child_flags);
    if (ImGui::BeginMenuBar()) {
      ImGui::TextUnformatted("Texture Packs");
      ImGui::EndMenuBar();
    }

    static ImVec2 rect_pos{};
    static ImVec2 rect_size{};
    static bool needs_load = false;
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
    child_is_visible = ImGui::BeginChild(child_id, ImVec2(child_w, ImGui::GetContentRegionAvail().y), true, child_flags);
    if (ImGui::BeginMenuBar()) {
      ImGui::TextUnformatted("Textures");
      ImGui::EndMenuBar();
    }

    if ((selected_texture_pack != mcctp::TexturePackFlags::None) && needs_load) {
      mcctp::ctx *inst = ctx::Instance();
      unsigned int texture_unit = 0;
      for (const auto &res : inst->GetResourcesFromTexturePack(selected_texture_pack)) {
        auto thumbnail = std::make_shared<mcctp::Image>((TexturePackResource)res);

        //thumbnail->Bind();
        //glActiveTexture(GL_TEXTURE0 + texture_unit);
        //thumbnail->Unbind();

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
      auto text = m_TextureThumbnails.at(n);
      ImVec2 start = ImGui::GetCursorScreenPos();
      glBindTexture(GL_TEXTURE_2D, text->GetRendererID());
      //if (ImGui::ImageButton((void *)(intptr_t)m_TextureThumbnails.at(n)->GetRendererID(),
      //                       button_sz, ImVec2(), ImVec2()))
      auto wc = GetItemRectMaintainAspectRatio(text->GetWidth(), text->GetHeight(), button_sz);
      if (ImGui::Button("", button_sz))
      {
        selected_thumbnail = n;
      }
      ImGui::GetWindowDrawList()->AddImage((void*)text->GetRendererID(), wc.first, wc.second, ImVec2(0, 0), ImVec2(1, 1));
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

void mcctp::StreamedImage::DoTextureInfo(void) { 
    ImGui::Begin("Texture Info");
    if (m_TextureThumbnails.size() != 0) {
      auto texture = m_TextureThumbnails.at(selected_thumbnail);
      ImGui::SeparatorText(texture->GetName().c_str());
      ImGui::Text("Texture Pack: %s", mcctp::FlagToBasename.at(selected_texture_pack));
      ImGui::Text("Pack Type: %d", PackTypeToExt.at(texture->GetPackType()));
      ImGui::Text("Width: %d", texture->GetWidth());
      ImGui::Text("Height: %d", texture->GetHeight());
      ImGui::Text("Size: %d", texture->GetSize());
      ImGui::Text("Format: %d", ResourceFormatToString.at(texture->GetFormat()));
      ImGui::Text("Origin: %p", (void *)texture->GetOrigin());
      ImGui::Text("Offset: %p", (void *)(texture->GetOrigin() + texture->GetOffset()));
      
    }
    ImGui::End();
}

std::shared_ptr<mcctp::Image> LoadAppIcon(void) {
    std::filesystem::path app_icon = 
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    app_icon += "/res/mcctp.png";

    std::vector<uint8_t> data;
    uint32_t w;
    uint32_t h;
    uint32_t channels = 4;
    uint32_t desired_channels = 4;
    //fpng::fpng_decode_file(app_icon.generic_string().c_str(), data, w, h, channels, desired_channels);
    
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char *image_data =
        stbi_load(app_icon.generic_string().c_str(), &image_width, &image_height, NULL, 4);
    auto image = std::make_shared<mcctp::Image>(data, image_width, image_height, 4);
    if (image_data != NULL) stbi_image_free(image_data);
    return image;
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

std::pair<ImVec2, ImVec2> GetScreenFill(void) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImVec2 filled;
    filled.x = pos.x + avail.x;
    filled.y = pos.y + avail.y;

    return {pos, filled};
}

std::pair<ImVec2, ImVec2> GetScreenMaintainAspectRatio(uint32_t width, uint32_t height) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 avail = ImGui::GetContentRegionAvail();

    ImVec2 aspected = avail;
    float aspect_ratio = (float)width / (float)height;

    aspected.y = aspected.x / aspect_ratio;
    float yOff = (avail.y - aspected.y) / 2;
    if (yOff >= 0.0f) {
      pos.y += yOff;
    } else {
      aspected = avail;
      aspected.x = aspected.y * aspect_ratio;
      float xOff = (avail.x - aspected.x) / 2;
      if (xOff >= 0.0f) {
        pos.x += xOff;
      }
    }
    aspected.x += pos.x;
    aspected.y += pos.y;

    return {pos, aspected};
}
std::pair<ImVec2, ImVec2> GetItemRectMaintainAspectRatio(uint32_t width, uint32_t height, ImVec2 item_extents) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 avail = item_extents;

    ImVec2 aspected = avail;
    float aspect_ratio = (float)width / (float)height;

    aspected.y = aspected.x / aspect_ratio;
    float yOff = (avail.y - aspected.y) / 2;
    if (yOff >= 0.0f) {
        pos.y += yOff;
    }
    else {
        aspected = avail;
        aspected.x = aspected.y * aspect_ratio;
        float xOff = (avail.x - aspected.x) / 2;
        if (xOff >= 0.0f) {
            pos.x += xOff;
        }
    }
    aspected.x += pos.x;
    aspected.y += pos.y;

    return { pos, aspected };
}