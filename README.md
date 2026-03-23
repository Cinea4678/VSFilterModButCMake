# VSFilterMod (CMake Build)

A cross-platform fork of [VSFilterMod](https://github.com/sorayuki/VSFilterMod) with CMake build system support, enabling builds on **macOS** and **Linux** in addition to Windows.

这是 [VSFilterMod](https://github.com/sorayuki/VSFilterMod) 的跨平台 CMake 构建分支，支持在 **macOS** 和 **Linux** 上编译运行（同时保留 Windows 支持）。

> For the latest upstream releases, check: https://github.com/sorayuki/VSFilterMod/network

---

## Platform Support / 平台支持

| Platform | VapourSynth Plugin | AviSynth Plugin | DirectShow Filter |
|----------|-------------------|-----------------|-------------------|
| Windows (x64) | ✓ | ✓ | ✓ |
| macOS (x64 / Apple Silicon) | ✓ | ✓ | — |
| Linux (x64 / aarch64) | ✓ | ✓ | — |

DirectShow filter (`VSFilterMod.dll` / `regsvr32`) is Windows-only.

DirectShow 滤镜（`VSFilterMod.dll` / `regsvr32` 注册）仅限 Windows。

---

## Building / 编译

### Prerequisites / 依赖

**All platforms:**
- CMake ≥ 3.16
- C++17 compatible compiler

**macOS:**
```bash
brew install freetype harfbuzz fontconfig pkg-config
```

**Linux (Debian/Ubuntu):**
```bash
sudo apt install build-essential cmake libfreetype-dev libharfbuzz-dev libfontconfig-dev pkg-config
```

**Windows:**
- Visual Studio 2019+ with C++ desktop workload (MSVC)
- Dependencies are bundled in the repository

### Build Steps / 编译步骤

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

On Apple Silicon (arm64), the build system automatically fetches [sse2neon](https://github.com/DLTcollab/sse2neon) to translate SSE2 intrinsics to NEON.

在 Apple Silicon (arm64) 平台上，构建系统会自动下载 [sse2neon](https://github.com/DLTcollab/sse2neon) 以将 SSE2 指令转换为 NEON。

The built plugin will be located at `build/src/plugins/libvsfm.dylib` (macOS) or `build/src/plugins/libvsfm.so` (Linux).

---

## Usage / 用法

### VapourSynth

```python
vsfm.TextSubMod(clip clip, string file[, int charset=1, float fps=-1.0, string vfr='', int accurate=0])
vsfm.VobSub(clip clip, string file)
```

- **clip**: Clip to process. Supported formats: YUV420P8, YUV420P10, YUV420P16, RGB24.
- **accurate**: `1` = enable accurate render for 10/16-bit (~2x slower) / `0` = disable (default).

**clip**：待处理的视频片段。支持格式：YUV420P8、YUV420P10、YUV420P16、RGB24。
**accurate**：`1` = 启用 10/16 位精确渲染（速度约慢一倍） / `0` = 禁用（默认）。

### DirectShow (Windows only / 仅 Windows)

1. Run `regsvr32.exe VSFilterMod.dll` with administrator privileges.
2. In MPC-BE: Options → Subtitles → Subtitle renderer → select "VSFilter/xy-VSFilter".

---

## VSFilterMod Extended Tags / VSFilterMod 扩展标签

All VSFilterMod custom override tags are fully supported on all platforms (Windows, macOS, Linux). They are enabled via the `_VSMOD` compile definition.

所有 VSFilterMod 自定义特效标签在全平台（Windows、macOS、Linux）上均完整可用，通过 `_VSMOD` 编译宏启用。

| Tag | Description (EN) | 说明 (中文) |
|-----|-------------------|------------|
| `\fsc<scale>` | Uniform font scale (aggregate `\fscx` + `\fscy`) | 等比字体缩放 |
| `\fsvp<leading>` | Text leading (line spacing) | 行距调整 |
| `\frs<angle>` | Baseline obliquity (rotation) | 基线倾斜旋转 |
| `\z<depth>` | Z coordinate (perspective depth) | Z 坐标（景深） |
| `\distort(u1,v1,u2,v2,u3,v3)` | Corner-pin distortion | 四角变形 |
| `\rnd<n>` `\rndx` `\rndy` `\rndz` | Random boundary deformation | 边界随机变形 |
| `\$vc(c1,c2,c3,c4)` | Four-corner color gradient | 四角颜色渐变 |
| `\$va(a1,a2,a3,a4)` | Four-corner alpha gradient | 四角透明度渐变 |
| `\$img(path[,x,y])` | Image pattern fill | 图片纹理填充 |
| `\mover(x1,y1,x2,y2,a1,a2,r1,r2[,t1,t2])` | Polar (arc/spiral) movement | 极坐标（弧线/螺旋）运动 |
| `\moves3(...)` / `\moves4(...)` | Cubic/bicubic Bezier spline movement | 三次/四次贝塞尔样条运动 |
| `\jitter(l,r,u,d,period[,seed])` | Position shaking | 位置抖动 |
| `\movevc(...)` | Moveable vector clip (independent of `\pos`/`\move`) | 可独立移动的矢量裁剪 |

All tags above are animatable via `\t`.

以上所有标签均支持通过 `\t` 进行动画。

---

## Known Issues / 已知问题

- OpenType fonts (e.g. Source Han Sans) may appear smaller when displayed vertically (using `@` prefix like `@Source Han Sans`). This is because the VSFilter rendering pipeline uses GDI for font metrics, which handles OpenType fonts poorly.
- SSF subtitle format is not supported on macOS/Linux. The SSF renderer depends on GDI-specific font metrics (`TEXTMETRIC`, kerning pairs) which have no implementation on non-Windows platforms. ASS/SRT subtitles work normally.

- OpenType 字体（如思源黑体）在竖排显示时（使用 `@` 前缀，如 `@Source Han Sans`）可能显示偏小。这是由于 VSFilter 渲染管线使用 GDI 进行字体度量，GDI 对 OpenType 字体支持较差。
- SSF 字幕格式在 macOS/Linux 上不支持。SSF 渲染器依赖 GDI 特有的字体度量信息（`TEXTMETRIC`、字偶距），在非 Windows 平台上没有对应实现。ASS/SRT 字幕正常工作。
