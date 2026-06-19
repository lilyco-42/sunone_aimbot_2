# 构建过程记录

## 环境

| 项目 | 版本 |
|------|------|
| Windows | 11 Pro for Workstations 10.0.26200 |
| CMake | 4.3.4 |
| Ninja | 1.13.2 |
| MSVC | 19.51.36246 (VS 2026) |
| OpenCV | 4.13.0 (DML 预编译) |
| ONNX Runtime | 1.22.0 (NuGet) |
| DirectML | 1.15.4 (NuGet) |

## 遇到的问题和修复

### 问题 1: `NOMINMAX` 未定义导致 SimpleIni.h 编译失败

**现象**:
```
SimpleIni.h(1333): error C2589: "(":"::"右边的非法标记
SimpleIni.h(1333): error C2059: 语法错误:")"
```

**原因**: `windows.h` 定义了 `max` 宏，与 `SimpleIni.h` 中的 `std::numeric_limits<size_t>::max()` 冲突。项目代码中到处定义了 `WIN32_LEAN_AND_MEAN`，但没有定义 `NOMINMAX`。

**修复**: 在 `CMakeLists.txt` 的 `target_compile_definitions(ai PRIVATE ...)` 中添加 `NOMINMAX`：

```cmake
target_compile_definitions(ai PRIVATE
    UNICODE
    _UNICODE
    _CONSOLE
    NOMINMAX               # <-- 新增
    _SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS
)
```

### 问题 2: x86 交叉编译器与 x64 库不匹配

**现象**:
```
LNK4272: 库计算机类型"x64"与目标计算机类型"x86"冲突
LNK1120: 34 个无法解析的外部命令 (cv::Mat 等所有 OpenCV 函数)
```

**原因**: CMake 自动检测到的编译器路径为 `Hostx86/x86/cl.exe`（32位），但项目依赖的 OpenCV、ONNX Runtime、DirectML 库全部是 x64 架构。`build_common.ps1` 中的 `Import-VisualStudioEnvironment` 函数在 PATH 中已存在 x86 `cl.exe` 时提前返回，跳过了 `VsDevCmd.bat -arch=x64` 的环境设置。

**编译器和链接器路径对比**:

| | x86 (错误) | x64 (正确) |
|---|---|---|
| cl.exe | `Hostx86\x86\cl.exe` | `Hostx64\x64\cl.exe` |
| link.exe | `Hostx86\x86\link.exe` | `Hostx64\x64\link.exe` |
| /machine | X86 | X64 |

**修复**: 清理 CMake 缓存后重新配置，确保 CMake 检测到 x64 编译器：

```powershell
# 删除旧缓存
Remove-Item build/dml/CMakeCache.txt
Remove-Item -Recurse build/dml/CMakeFiles

# VsDevCmd.bat 未正确设置环境时，CMake 仍可能从 PATH 找到 x86 编译器
# 最终通过确保 x64 cl.exe 在 PATH 中优先被找到来解决
```

## 构建步骤（已验证通过）

```powershell
# 前提：已安装 Visual Studio (含 C++ 桌面开发)、CMake、Ninja
# 依赖：OpenCV DML 预编译包、NuGet 包已就位

cd D:\CODE\aim

# 方法1：使用项目构建脚本（推荐）
.\build_dml.bat -OpenCvAlreadyBuilt $true -DownloadOrUpdateNeeded $false

# 方法2：手动构建
cmake -S . -B build/dml -G "Ninja Multi-Config" `
    -DCMAKE_MAKE_PROGRAM=ninja.exe `
    -DAIMBOT_USE_CUDA=OFF `
    -DAIMBOT_ONNXRUNTIME_DIR=packages/Microsoft.ML.OnnxRuntime.DirectML.1.22.0 `
    -DAIMBOT_DIRECTML_DIR=packages/Microsoft.AI.DirectML.1.15.4

cmake --build build/dml --config Release --target ai --parallel
```

## 输出文件

```
build/dml/Release/
├── ai.exe                     (1.8 MB)  主程序
├── opencv_world4130.dll       (64.8 MB) OpenCV 运行时
├── onnxruntime.dll            (15.7 MB) ONNX Runtime
├── DirectML.dll               (17.7 MB) DirectML 运行时
├── onnxruntime_providers_shared.dll
├── ghub_mouse.dll                       罗技鼠标驱动
├── rzctl.dll                            雷蛇鼠标驱动
├── config.ini                           默认配置文件
├── models/                              模型目录（需手动放入 .onnx 文件）
└── depth_models/                        深度模型目录
```

## 运行

```powershell
cd build/dml/Release
.\ai.exe
```

首次运行会自动生成 `config.ini`。将 `.onnx` 模型文件放入 `models/` 目录，然后在覆盖层中按 `HOME` 键选择模型。

## 发布

发布到 GitHub Release 的独立包（无需安装 VS/CUDA/NuGet）：[Releases](https://github.com/lilyco-42/sunone_aimbot_2/releases)
