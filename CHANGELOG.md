# Changelog

<!--
  手动维护。每次发版前在此追加新段落，标题格式必须为：

  ## v<version> - <YYYY-MM-DD>

  CI 会按 tag 名(如 v0.1.1)提取对应段落作为 Release 正文，
  tag 在 CHANGELOG.md 中缺失会导致发布失败。
-->

## v0.1.0 - 2026-08-22

### Added

- 初始版本：DLL 注入 The Binding of Isaac: Repentance，事件经 WebSocket 转发至 DG-LAB Coyote 脉冲设备
- ImGui 覆盖层监控与配置界面
- 启动器 IsaacCoyoteLauncher.exe

### Changed

- CI: x86 配置改用 msvc-dev-cmd，固定 CMake 版本

### Fixed

- 无
