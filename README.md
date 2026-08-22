<div align="center">

# IsaacCoyote

**以撒的结合 忏悔+(The Binding of Isaac Repentance+) 的郊狼联动Mod**

[![Release](https://img.shields.io/github/v/release/u0cat/IsaacCoyote?style=flat-square)](https://github.com/u0cat/IsaacCoyote/releases)
[![License](https://img.shields.io/badge/license-MIT-green?style=flat-square)](#-许可证)
[![Platform](https://img.shields.io/badge/platform-Windows%20x86-blue?style=flat-square)]()
[![C++](https://img.shields.io/badge/C%2B%2B-23-00599C?style=flat-square&logo=cplusplus&logoColor=white)]()
[![Game](https://img.shields.io/badge/game-TBoI%20Repentance+-red?style=flat-square)]()

</div>

---

## 简介

**IsaacCoyote**
是一个注入 [The Binding of Isaac: Repentance+](https://store.steampowered.com/app/3353470/The_Binding_of_Isaac_Repentance/)
的 x86 DLL Mod。<br />
它通过读取游戏内部状态，获取玩家的事件和状态，按自定义的规则计算强度, 推送到 **DG-LAB Coyote**(郊狼)。

## 安全须知

> [!WARNING]
> 本项目控制的是真实的电刺激设备，使用前请务必阅读：
>
> - **首次使用请将 `strength.limit_a` / `limit_b` 设置为较低值**（建议 <= 50），并从最低强度开始适应；
> - 心脏起搏器佩戴者、孕妇、癫痫患者及存在相关疾病者**请勿使用**；
> - 请将电极贴附于安全部位，避免心脏电流通路（如双手之间、左右乳之间）；
> - 游戏中若出现不适，立即按 <kbd>END</kbd> 卸载模组或直接断开设备；
> - 请勿在无法随时中断的场景下使用（如驾驶、睡眠）。

> [!NOTE]
> 本项目为非官方第三方模组，与 Nicalis
> 及游戏原作者无关。使用本模组产生的任何后果由使用者自行承担，详见[免责声明](#免责声明)。

## 支持的事件

| 事件       | 配置标识              | 触发时机            |
|----------|-------------------|-----------------|
| 受到伤害     | `OnHurt`          | 玩家受到伤害          |
| 死亡       | `OnDeath`         | 玩家死亡但仍可复活       |
| GameOver | `OnGameOver`      | 显示GameOver界面时   | 
| Reroll   | `OnRerollGame`    | 启动新游戏时存在进行中的游戏  | 
| 使用主动道具   | `OnUseActiveItem` | 使用主动道具          | 
| 使用胶囊     | `OnUsePill`       | 使用胶囊            | 
| 使用卡牌     | `OnUseCard`       | 使用塔罗牌 / 符文 / 魂石 |

## 快速开始

> DG-LAB V4 App 的保护性功能繁多<br />
> 如遇到 无输出 / 体感强度偏低，查看[FAQ](#faq)。

### 启动

1. 前往 [Releases](https://github.com/u0cat/IsaacCoyote/releases) 下载最新的 `IsaacCoyote-vX.Y.Z-x86.zip`
2. 解压得到两个文件，放在同一目录：

   ```
   IsaacCoyote/
   ├── IsaacCoyote.dll
   └── IsaacCoyoteLauncher.exe
   ```

3. **先启动游戏**（出现主菜单即可）。
4. 运行 `IsaacCoyoteLauncher.exe`，看到弹出 `IsaacCoyote Console` 控制台窗口即注入成功。
5. 游戏内按 <kbd>INSERT</kbd> 打开菜单 → **Connection** 页 → 用 DG-LAB V4 App 扫描二维码完成连接。
6. 切到 **Config** 页调整规则，开始游戏！

### 基本操作

| 按键                | 功能         |
|-------------------|------------|
| <kbd>INSERT</kbd> | 显示 / 隐藏 菜单 |
| <kbd>END</kbd>    | 停止并卸载Mod   |
| <kbd>F2</kbd>     | 显示日志       |

### 日志

运行日志在`游戏根目录`的 `isaac-coyote.log`

## 规则系统

由两类规则构成，均可在菜单或 JSON 中编辑：<br />
目前处于早期阶段，Action 均为并行<br />
后续会探索 Action 以及不同规则之间的 的趣⬆味⬇组合(

### 事件规则（Event Rules）

事件触发时依次执行 `actions` 列表：

| 动作类型       | 字段                                 | 说明                       |
|------------|------------------------------------|--------------------------|
| `pulse`    | `duration` / `pulse_a` / `pulse_b` | 在 A/B 通道推送指定名称的波形，持续指定时间 |
|            | `shake` / `shake_duration`         | 可选：同时触发游戏震屏              |
| `strength` | `channel_a` / `channel_b`          | 改变通道强度                   |

事件效果具有持续时间，到期自动过期并重新计算静态部分。

### 静态规则（Static Rules）

随游戏状态实时变化：

- **红心规则（health）**：按当前红心与最大红心数的差值，应用 `per_red_heart`；
- **藏品规则（collectible）**：按持有藏品的品质（quality 0–4），应用 `modifiers_by_quality` 中对应档位的强度。

### 强度计算详解

```
base_a/b ──► 静态规则求值 ──► 有效Action叠加 ──► limit_a/b 限制
                                                      │
        CoyoteService::tick() ◄── OutputSmoother ◄────┘
             │                (climb 爬升 / decay 衰减 平滑化)
             ▼
      WebSocket ► DG-LAB Coyote
```

另提供 `constant_mode`（常开循环波形）作为独立于规则的模式

## 配置

首次运行会以构建时内嵌的默认配置在`游戏根目录`生成 `isaac-coyote.json`，之后可直接修改文件或游戏内编辑。

### 波形

与 DG-LAB 官方格式一致：每个波形是字符串数组，**每帧 16 个十六进制字符（8 字节）**——前 4 字节为 A/B 通道交替的频率，后 4
字节为交替的强度（`00`–`64` 即 0–100）。规则中引用的 `pulse_a` / `pulse_b` 名称须在此定义。

```jsonc
"挤压": [
  "0A0A0A0A00000000",
  "0A0A0A0A64646464"
]
```

## FAQ

**连接后 无输出/体感强度过低**

- 确认设备已连接到APP
- 关闭 `屏蔽输出` 功能
- 检查 `输出模式` 中的轻柔模式
- 关闭 `强度保护上限` 中的 `上限自适应提升`
- 调整 `强度上限保护` 的模式为 `简单模式`
- 关闭 `软启动` 模式

**杀毒软件报毒？**

Launcher 通过 `CreateRemoteThread + LoadLibraryW` 注入，属于此类工具的常见误报特征。可加入信任列表，不放心请自行从源码构建。

**提示 `isaac-ng.exe is not running`？**

等待游戏完全启动再运行 Launcher。

**游戏更新后失效？**

内存签名（AOB）可能随版本更新偏移，等待更新即可。

**支持联机吗？**

玩家过滤器支持 `Others` / `Any` 与指定 `player_ids`，可为联机中的不同玩家分别配置规则。

## 免责声明

1. 本项目涉及真实电刺激设备的远程控制。使用者应充分了解 DG-LAB 设备的安全使用规范，因不当使用造成的人身伤害由使用者自行负责；
2. 请遵守所在地区法律法规，未满 18 周岁请勿使用本项目。

## 贡献

欢迎提交 Issue 与 Pull Request！反馈问题时请附上：游戏版本、`isaac-coyote.log` 相关片段、复现步骤。

## 从源码构建

### 环境要求

| 依赖    | 要求                                            |
|-------|-----------------------------------------------|
| 系统    | Windows 10+                                   |
| 工具链   | MSVC (VS2022)，**x86**                         |
| CMake | >= 4.2                                        |
| 构建器   | Ninja（推荐）                                     |
| 包管理   | [vcpkg](https://vcpkg.io)（manifest 模式，依赖自动安装） |

### 构建步骤

```powershell
git clone https://github.com/u0cat/IsaacCoyote.git
cd IsaacCoyote

# 配置 x86 MSVC 环境
call "<VS安装路径>\VC\Auxiliary\Build\vcvarsall.bat" x86

cmake -S . -B build -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_TOOLCHAIN_FILE="<vcpkg根目录>/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x86-windows-static `

cmake --build build --config Release
```

> [!IMPORTANT]
> 项目强制 x86（`isaac-ng.exe` 为 x86）

## 架构

```
isaac-ng.exe 内存
     │  AOB 扫描 + mid-hook (safetyhook)
     ▼
isaac_spy ── MemoryRef 对象包装 (Player/Game/Manager…)
     │  游戏事件
     ▼
EventEngine（Post/Dispatch 队列，每事件类型一个 Source）
     │  EventVariant
     ▼
RuleEngine::on_event() ── RuleHandlerManager（7 event + 2 static handlers）
     │                     │
     │  ActiveEffects      │  StaticRule → ChannelModifiers
     ▼                     ▼
  StrengthComposer（集中计算 Modifier）
                     │
                     ▼
              OutputSmoother（climb/decay 平滑）
                     │
                     ▼
        CoyoteService::tick() ── PulseHelper（每通道波形队列）
                     │
                     ▼
        WsController (ix::WebSocket) ──► DG-LAB Socket 服务
```

```
src/
├── app/            # 应用层：编排、事件引擎、规则引擎、coyote 服务、overlay
│   ├── service/    #   config / coyote / overlay / log
│   ├── event/      #   事件源与描述符（catalog 驱动的 variant dispatch）
│   └── rule/       #   规则编译、handler 注册、强度合成
├── isaac_spy/      # 内存扫描、hook 管理、游戏对象包装
├── launcher/       # 注入器
└── utils/
```
