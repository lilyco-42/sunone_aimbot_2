<div align="center">

# Sunone Aimbot 2 (C++) — 中文汉化版

> 🀄️ 完整中文界面 | 🖱️ RP2040 硬件鼠标 | 🔫 三角洲行动优化

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue)](https://github.com/lilyco-42/sunone_aimbot_2)
[![DirectML](https://img.shields.io/badge/DirectML-Universal_GPU-green)](https://github.com/lilyco-42/sunone_aimbot_2)
[![Chinese](https://img.shields.io/badge/UI-中文-red)](https://github.com/lilyco-42/sunone_aimbot_2)
[![RP2040](https://img.shields.io/badge/Hardware-RP2040_Mouse-orange)](https://github.com/lilyco-42/ai_aimbot_train)

> 原项目: [SunOner/sunone_aimbot_2](https://github.com/SunOner/sunone_aimbot_2) · 训练仓库: [ai_aimbot_train](https://github.com/lilyco-42/ai_aimbot_train)

</div>

## 🚀 本 Fork 新增功能

| 功能 | 说明 |
|------|------|
| 🀄️ **完整中文界面** | 覆盖层 365+ 字符串汉化，SimHei 中文字体支持 |
| 🖱️ **RP2040 硬件鼠标** | Pico 固件模拟 HID 鼠标，绕过反作弊拦截 |
| 🔫 **三角洲行动优化** | 调优配置 + 专用 AI 模型训练管道 |
| 🛠️ **编译修复** | NOMINMAX、/utf-8、x64 MSVC 修复 |
| 📦 **RP2040 固件** | PlatformIO 工程，串口转 USB HID |
| 📓 **Kaggle 免费训练** | Notebook + 数据集打包，CPU 免费训 YOLO |

## 📋 快速开始

### 1. 下载预编译版本

从 [Releases](../../releases) 下载 `sunone_aimbot_dml_x64.zip`，解压直接运行。

### 2. 放置模型

将 `.onnx` 模型放入 `models/` 目录，覆盖层中 `HOME` 键选择。

### 3. 管理员运行

```powershell
Start-Process ai.exe -Verb RunAs
```

首次启动自动生成 `config.ini`。

### 硬件鼠标（可选）

烧录 RP2040 Pico 固件后改用硬件 HID：

```ini
input_method = RP2350
rp2350_port = COM9
rp2350_baudrate = 115200
rp2350_16_bit_mouse = true
```

固件在 [ai_aimbot_train/rp2040_aim](https://github.com/lilyco-42/ai_aimbot_train/tree/main/rp2040_aim)。

---

# 原项目说明

---

# Ready-to-Use Builds (Recommended)

**You do NOT need to compile anything if you just want to use the aimbot!**
Precompiled `.exe` builds are provided for both CUDA (NVIDIA only) and DirectML (all GPUs).

* **Download**
	* Pre-built binaries can be downloaded from the [Discord server](https://discord.gg/37WVp6sNEh) in the **pre-releases** channel.

---


### DirectML (DML) Build — Universal (All GPUs)

* **Works on:**

	* Any modern GPU (NVIDIA, AMD, Intel, including integrated graphics)
	* Windows 10/11 (x64)
	* No need for CUDA or special drivers
* **Recommended for:**

	* GTX 10xx/9xx/7xx series (old NVIDIA)
	* Any AMD Radeon or Intel Iris/Xe GPU
	* Laptops and office PCs with integrated graphics

### CUDA + TensorRT Build — High Performance (NVIDIA Only)

* **Works on:**

	* NVIDIA GPUs **GTX 1660, RTX 2000/3000/4000/5000**
	* **Requires:** latest NVIDIA graphics driver and CUDA 13.1
	* Windows 10/11 (x64)
* **Not supported:** GTX 10xx/Pascal and older (TensorRT limitation)
* **Includes CUDA+TensorRT runtime files in the precompiled build**
* TensorRT-10.14.1.48 is only needed separately when building from source.

Before using the CUDA build, update your NVIDIA graphics driver to the latest available version, then install [CUDA 13.1](https://developer.nvidia.com/cuda-13-1-0-download-archive). Do this before troubleshooting missing CUDA DLLs or launch-close issues.

**Both versions are ready-to-use: just download, unpack, run `ai.exe`.**

---

## How to Run (For Precompiled Builds)

1. **Download and unpack your chosen version (see links above).**
2. For CUDA build, first update your NVIDIA graphics driver to the latest version, then install [CUDA 13.1](https://developer.nvidia.com/cuda-13-1-0-download-archive) if not already installed.
3. For DML build, no extra software is needed.
4. **Run `ai.exe`.**
   On first launch, the model will be exported (may take up to 5 minutes).
5. Place your `.onnx` model in the `models` folder and select it in the overlay (HOME key).
6. All settings are available in the overlay.
   Use the HOME key to open/close overlay.

### Controls

* **Right Mouse Button:** Aim at the detected target
* **F2:** Exit
* **F3:** Pause aiming
* **F4:** Reload config
* **Home:** Open/close overlay and settings

---

# Build From Source (Advanced Users)

* [docs/build.md](docs/build.md)

---

## 📋 Documentation

* C++ config reference (`config.ini`):
  [docs/config.md](docs/config.md)
* Setup, FAQ, troubleshooting:
  [docs/guides.md](docs/guides.md)
* Source of truth in code:
  [sunone_aimbot_2/config/config.cpp](sunone_aimbot_2/config/config.cpp),
  [sunone_aimbot_2/config/config.h](sunone_aimbot_2/config/config.h)

---

## 📚 References & Useful Links

* [TensorRT Documentation](https://docs.nvidia.com/deeplearning/tensorrt/)
* [OpenCV Documentation](https://docs.opencv.org/4.x/d1/dfb/intro.html)
* [ImGui](https://github.com/ocornut/imgui)
* [CppWinRT](https://github.com/microsoft/cppwinrt)
* [GLFW](https://www.glfw.org/)
* [WindMouse](https://ben.land/post/2021/04/25/windmouse-human-mouse-movement/)
* [KMBOX](https://www.kmbox.top/)
* [MAKCU](https://makcu.com)
* [depth-anything-tensorrt](https://github.com/spacewalk01/depth-anything-tensorrt)

---

## 📄 Licenses

### OpenCV

* **License:** [Apache License 2.0](https://opencv.org/license.html)

### ImGui

* **License:** [MIT License](https://github.com/ocornut/imgui/blob/master/LICENSE)

---
## ❤️ Support the Project & Get Better AI Models

This project is actively developed thanks to the people who support it on [Boosty](https://boosty.to/sunone) and [Patreon](https://www.patreon.com/c/sunone).
**By supporting the project, you get access to improved and better-trained AI models!**

---

**Need help or want to contribute? Join our [Discord server](https://discord.gg/37WVp6sNEh) or open an issue on GitHub!**

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=SunOner/sunone_aimbot_2&type=date&legend=top-left)](https://www.star-history.com/#SunOner/sunone_aimbot_2&type=date&legend=top-left)
