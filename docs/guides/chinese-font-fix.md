# 中文字体渲染修复指南

## 问题现象

ImGui 覆盖层中所有中文字符显示为方块（□）或问号（？），英文正常。

## 排查过程

### 第一层：源码编码检查

确认 C++ 源文件中的中文字符串是否正确到达运行时。

检查 `.obj` 编译产物中的字节：

```
目标字符串: "通用捕获"
编译后的十六进制: e9 80 9a e7 94 a8 e6 8d 95 e8 8e b7
解码结果: UTF-8 编码，完全正确 ✓
```

源文件是 UTF-8 编码，加上 CMakeLists.txt 中 MSVC `/utf-8` 编译选项，确保源字符集和执行字符集均为 UTF-8 → **编码无问题**。

### 第二层：字形范围检查

ImGui 默认只加载 Basic Latin (ASCII) 字形范围。中文需要 CJK Unified Ideographs。

原始代码：

```cpp
io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisb.ttf", 16.5f, &fontConfig);
```

`AddFontFromFileTTF` 不传 `glyph_ranges` 参数时，默认使用 `GetGlyphRangesDefault()`，仅包含 0x0020-0x00FF，没有中文字符。

**尝试修复**: 构建包含中文字形的范围：

```cpp
ImFontGlyphRangesBuilder builder;
builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
builder.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
builder.BuildRanges(&glyphRanges);
fontConfig.GlyphRanges = glyphRanges.Data;
```

**遇到问题 1**: `ImVector<ImWchar> glyphRanges` 作为局部变量，作用域结束后指针悬空。ImGui 在首帧才调用 `Build()` 构建字体图集，那时数据已无效 → 调用 `io.Fonts->Build()` 立即构建。

**遇到问题 2**: ImGui 1.92.8 更新了后端架构，要求 `ImGui_ImplDX11_Init()` 执行后才能构建字体图集，提前调用 `Build()` 报错：

```
Called ImFontAtlas::Build() before ImGuiBackendFlags_RendererHasTextures got set!
```

**解决**: 将 `glyphRanges` 改为 `static`，数据持续有效，ImGui 在首帧自动构建。

**修复后仍无效** → 进入第三层。

### 第三层：字体文件本身

Windows 上 Segoe UI 字体显示中文是通过**字体链接（Font Linking）**机制：当 Segoe UI 缺少某个字符时，Windows 自动从注册表中查找链接的字体（如微软雅黑）来渲染。

**ImGui 不依赖 Windows 字体链接**。它用 stb_truetype 直接从 .ttf 文件中提取字形。如果字体文件本身不包含 CJK 字形，ImGui 就无法渲染中文。Segoe UI 的 .ttf 文件不包含 CJK 字符，所以显示方块。

**验证**: 查找系统中包含 CJK 字形的 .ttf（非 .ttc，因为 stb_truetype 不支持 TrueType Collection）：

```
C:\Windows\Fonts\simhei.ttf   (黑体, 9.3MB)   → 包含 CJK ✓
C:\Windows\Fonts\Deng.ttf     (等线, 15.6MB)   → 包含 CJK ✓
C:\Windows\Fonts\simkai.ttf   (楷体, 11.2MB)   → 包含 CJK ✓
C:\Windows\Fonts\simfang.ttf  (仿宋, 10.1MB)   → 包含 CJK ✓
C:\Windows\Fonts\msyh.ttc     (微软雅黑, 18.8MB) → TTC 格式，不支持 ✗
```

## 最终方案

1. **优先加载包含 CJK 字形的 TTF 字体**（SimHei → Deng → KaiTi → FangSong）
2. **兜底保留 Segoe UI 字体链**
3. **字形范围用 static 变量持活**，避免指针悬空

```cpp
// Build glyph ranges: Default Latin + Chinese Simplified for CJK UI text.
// Must be static — ImGui stores a raw pointer and builds the atlas on first frame.
static ImVector<ImWchar> glyphRanges;
if (glyphRanges.empty())
{
    ImFontGlyphRangesBuilder rangesBuilder;
    rangesBuilder.AddRanges(io.Fonts->GetGlyphRangesDefault());
    rangesBuilder.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    rangesBuilder.BuildRanges(&glyphRanges);
}
fontConfig.GlyphRanges = glyphRanges.Data;

// Try fonts with native CJK glyphs first (SimHei, Microsoft YaHei via simhei),
// then fall back to Segoe UI (lacks CJK, will show boxes).
if (!io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simhei.ttf", 16.5f, &fontConfig) &&
    !io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Deng.ttf", 16.5f, &fontConfig) &&
    !io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simkai.ttf", 16.5f, &fontConfig) &&
    !io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\simfang.ttf", 16.5f, &fontConfig) &&
    !io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\seguisb.ttf", 16.5f, &fontConfig) &&
    !io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\SegUIVar.ttf", 16.5f, &fontConfig) &&
    !io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 16.5f, &fontConfig))
{
    io.Fonts->AddFontDefault();
}
```

## 经验总结

| 诊断步骤 | 方法 | 本次结果 |
|----------|------|----------|
| 源码编码 | 检查 .obj 中字符串的十六进制字节 | UTF-8 正确 |
| 编译选项 | 确认 `/utf-8` 标志是否生效 | 已生效 |
| 字形范围 | GlyphRanges 是否包含 CJK | 已包含（但数据持活有坑） |
| 字体文件 | 字体是否直接包含目标字形 | **根因在此** |
| 字体格式 | TTC vs TTF（stb_truetype 兼容性） | TTC 不支持 |

**核心教训**: ImGui 不依赖操作系统字体回退机制，所需的每个字形必须在指定的字体文件中直接存在。Segoe UI 在 Windows 上能显示中文是依靠注册表字体链接，ImGui 的 stb_truetype 无法利用此机制。
