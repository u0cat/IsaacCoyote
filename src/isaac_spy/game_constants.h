//
// Created by TsCat on 2026/8/18.
//

#ifndef ISAACSPY_GAME_CONSTANTS_H
#define ISAACSPY_GAME_CONSTANTS_H

#include <string_view>

namespace isaac_spy::constants
{
    // ------------------------------------------------------------------
    // AOB signatures (game EXE code section)
    // ------------------------------------------------------------------

    inline constexpr std::string_view kPatternEntityPlayerTakeDamage =
            "FF7518660F6E9500FFFFFF8B0D????????56FF75100F5BD2FFB5E0FEFFFF57E8????????84C0";

    inline constexpr std::string_view kPatternEntityPlayerTriggerDeath =
            "558BEC83E4F883EC08568BF15780BE????????00";

    inline constexpr std::string_view kPatternManagerRestartGame =
            "558BEC6AFF68????????64A1????????5083EC60A1????????33C58945??535657508D45??64A3????????C745??00000000";

    inline constexpr std::string_view kPatternManagerRestartGameConsoleStart =
            "558BEC6AFF68????????64A1000000005081EC3C020000A1????????33C58945F0535657508D45F464A3000000008BD9";

    inline constexpr std::string_view kPatternManagerRestartGameConsoleEnd =
            "8B4DF033CDE8????????8BE55DC3";

    inline constexpr std::string_view kPatternManagerStartNewGame =
            "558BEC6AFF68????????64A1000000005083EC60A1????????33C58945F05650";

    inline constexpr std::string_view kPatternEntityPlayerUseActiveItem =
            "538BDC83EC0883E4F883C404558B6B??896C24??8BEC6AFF68????????64A1????????5053B8B0190000";

    // Entity_Player::UseCard, all success/failure paths converge at the Lua MC_USE_CARD
    // call (0x7B89D7, CALL 0x00863cc0); hook offset 0x10 from the pattern start (0x7B89C7).
    // Absolute addresses are wildcarded: the EXE is ASLR-relocated at runtime.
    inline constexpr std::string_view kPatternEntityPlayerUseCard =
            "FF750C8B0D????????57FFB570FDFFFFE8????????";

    // Entity_Player::UsePill, all branches converge at 0x7C7557;
    // hook target = pattern + 0x3A = 0x7C7591 (after SetPillUseState, before Lua MC_USE_PILL).
    // MOVSS XMM2,[abs] 的绝对地址会被 ASLR 重定位, 必须用 ?? 通配 (运行时基址 0xAE0000 != PE 头 0x400000).
    inline constexpr std::string_view kPatternEntityPlayerUsePill =
            "80BDFAFDFFFF0074318B8DD4FDFFFF85C974278B413C85C0742084D274038B4140FFB5C8FDFFFFF30F1015????????FF71448BCF50E8????????";

    inline constexpr std::string_view kPatternEntityConfigManagerGetEntity =
            "558BEC83EC108B45??538B5D??56C1E00C";

    // Singleton capture: (????????) is the direct global pointer (capture syntax, do not strip).
    inline constexpr std::string_view kPatternGameSingleton =
            "c745fcffffffffa3(????????)e866afd9ff";

    inline constexpr std::string_view kPatternGameIsPaused =
            "8bd1568b35????????83";

    inline constexpr std::string_view kPatternGameScreenShake =
            "558BECA1????????83EC088B55085356";

    // Singleton capture: (????????) is the direct global pointer (capture syntax, do not strip).
    inline constexpr std::string_view kPatternManagerSingleton =
            "a1(????????)8b80????????83e800";

    inline constexpr std::string_view kPatternItemPoolGetPillEffect =
            "558bec83e4f851a1????????53568b75";

    inline constexpr std::string_view kPatternNetPlayManagerGetLocalPlayer =
            "568bf1e8????????85c074??8bcee8????????8bc85e";

    // NetManager::GetDeviceById — __thiscall(this = devices vector, device_id);
    // returns the Device* (0 when not found).
    inline constexpr std::string_view kPatternNetManagerGetDeviceById =
            "558BEC538BD933D256578B7D088B73048B032BF0C1FE0285F674??0F1F4400008B0839790C74??4283C0043BD672??";

    // NetManager::FormatUserProfileName — __stdcall(low, high, string_table, max_len);
    inline constexpr std::string_view kPatternNetManagerFormatUserProfileName =
            "558BEC6AFF68????????64A1000000005083EC0C535657A1????????33C5508D45F464A3000000008D4D08C745EC????????E8????????8945F0";

    inline constexpr std::string_view kPatternPlayerManagerIsMultiPlay =
            "558bec518b01538b59??2bd8";

    // TODO: exact pattern with reloc bytes:
    // 558bec6aff68201bb10064a1000000005083ec10535657a1b493bf0033c5508d45f464a3000000008bf98b5d0c
    inline constexpr std::string_view kPatternStringTableGetString =
            "558BEC6AFF6820????????A1000000005083EC10535657A1";

    // ------------------------------------------------------------------
    // Hook frame / install offsets (pattern-coupled)
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetHurtPostCall = 0x24;            // install offset
    inline constexpr std::ptrdiff_t kOffsetHurtRawDamage = 0x8;            // EBP-relative
    inline constexpr std::ptrdiff_t kOffsetHurtFinalDamage = -0x100;       // EBP-relative
    inline constexpr std::ptrdiff_t kOffsetHurtDamageFlags = -0x120;       // EBP-relative
    inline constexpr std::ptrdiff_t kOffsetHurtEntityRef = 0x14;           // EBP-relative

    inline constexpr std::ptrdiff_t kOffsetStartNewGamePlayerType = 0x4;   // ESP-relative
    inline constexpr std::ptrdiff_t kOffsetStartNewGameChallenge = 0x8;    // ESP-relative
    inline constexpr std::ptrdiff_t kOffsetStartNewGameDifficulty = 0x68;  // ESP-relative

    inline constexpr std::ptrdiff_t kOffsetUseActiveItemHook = 0x0;        // install offset (entry ESP)
    inline constexpr std::ptrdiff_t kOffsetUseActiveItemResultFlags = 0x4;
    inline constexpr std::ptrdiff_t kOffsetUseActiveItemItemId = 0x8;
    inline constexpr std::ptrdiff_t kOffsetUseActiveItemUseFlags = 0xC;
    inline constexpr std::ptrdiff_t kOffsetUseActiveItemActiveSlot = 0x10;
    inline constexpr std::ptrdiff_t kOffsetUseActiveItemVarData = 0x14;

    inline constexpr std::ptrdiff_t kOffsetUseCardHook = 0x10;             // install offset
    inline constexpr std::ptrdiff_t kOffsetUseCardCardId = -0x290;         // EBP-relative
    inline constexpr std::ptrdiff_t kOffsetUseCardUseFlags = 0xC;          // EBP-relative
    inline constexpr std::ptrdiff_t kOffsetUseCardCardConfig = -0x368;     // EBP-relative
    inline constexpr std::ptrdiff_t kOffsetUseCardCardSlot = -0x294;       // EBP-relative

    inline constexpr std::ptrdiff_t kOffsetUsePillHook = 0x3A;             // install offset
    inline constexpr std::ptrdiff_t kOffsetUsePillPillEffect = 0x8;        // EBX-relative (entry ESP)
    inline constexpr std::ptrdiff_t kOffsetUsePillPillColor = 0xC;         // EBX-relative (entry ESP)
    inline constexpr std::ptrdiff_t kOffsetUsePillUseFlags = 0x10;         // EBX-relative (entry ESP)

    inline constexpr std::ptrdiff_t kOffsetRestartClearSeedEffects = 0x60;      // ESP-relative
    inline constexpr std::ptrdiff_t kOffsetRestartProgressScaredHeart = 0x64;   // ESP-relative

    // ------------------------------------------------------------------
    // Player
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetPlayerMaxHearts = 0x1340;
    inline constexpr std::ptrdiff_t kOffsetPlayerRedHearts = 0x1344;
    inline constexpr std::ptrdiff_t kOffsetPlayerEternalHearts = 0x1348;
    inline constexpr std::ptrdiff_t kOffsetPlayerSoulHearts = 0x134C;
    inline constexpr std::ptrdiff_t kOffsetPlayerBlackHearts = 0x1350;
    inline constexpr std::ptrdiff_t kOffsetPlayerKeys = 0x135C;
    inline constexpr std::ptrdiff_t kOffsetPlayerBombs = 0x1364;
    inline constexpr std::ptrdiff_t kOffsetPlayerCoins = 0x1368;
    inline constexpr std::ptrdiff_t kOffsetPlayerPlayerType = 0x13BC;
    inline constexpr std::ptrdiff_t kOffsetPlayerCanFly = 0x1564;
    inline constexpr std::ptrdiff_t kOffsetPlayerDeviceId = 0x1618;
    inline constexpr std::ptrdiff_t kOffsetPlayerCollectibles = 0x16C8;
    inline constexpr std::ptrdiff_t kOffsetPlayerCollectiblesEnd = 0x16CC;
    inline constexpr std::ptrdiff_t kOffsetPlayerPocketItems = 0x17A0;
    inline constexpr std::ptrdiff_t kOffsetPlayerTrinkets = 0x16C0;

    // ------------------------------------------------------------------
    // Entity
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetEntityType = 0x28;
    inline constexpr std::ptrdiff_t kOffsetEntityVariant = 0x2C;
    inline constexpr std::ptrdiff_t kOffsetEntitySubType = 0x30;
    inline constexpr std::ptrdiff_t kOffsetEntitySpawner = 0x3C8;

    // ------------------------------------------------------------------
    // EntityRef
    // ------------------------------------------------------------------

    // struct EntityRef {            // sizeof 0x28
    //     int      Type;            // +0x00
    //     int      Variant;         // +0x04
    //     int      SpawnerType;     // +0x08
    //     int      SpawnerVariant;  // +0x0C
    //     Vector   Position;        // +0x10   (8B)
    //     Vector   Velocity;        // +0x18   (8B)
    //     unsigned Flags;           // +0x20
    //     Entity*  Entity;          // +0x24
    // };
    inline constexpr std::ptrdiff_t kOffsetEntityRefEntity = 0x24;

    // ------------------------------------------------------------------
    // ItemConfig & ItemConfigManager tables
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetItemConfigType = 0x00;
    inline constexpr std::ptrdiff_t kOffsetItemConfigItemId = 0x04;
    inline constexpr std::ptrdiff_t kOffsetItemConfigCardPillId = 0x00;
    inline constexpr std::ptrdiff_t kOffsetItemConfigName = 0x08;
    inline constexpr std::ptrdiff_t kOffsetItemConfigDesc = 0x20;
    inline constexpr std::ptrdiff_t kOffsetItemConfigCardType = 0x58;
    inline constexpr std::ptrdiff_t kOffsetItemConfigQuality = 0xC0;

    inline constexpr std::ptrdiff_t kOffsetItemTableCollectiblesBegin = 0x00;
    inline constexpr std::ptrdiff_t kOffsetItemTableCollectiblesEnd = 0x04;
    inline constexpr std::ptrdiff_t kOffsetItemTableTrinketsBegin = 0x0C;
    inline constexpr std::ptrdiff_t kOffsetItemTableTrinketsEnd = 0x10;
    inline constexpr std::ptrdiff_t kOffsetItemTableCardsBegin = 0x24;
    inline constexpr std::ptrdiff_t kOffsetItemTableCardsEnd = 0x28;
    inline constexpr std::ptrdiff_t kOffsetItemTablePillsBegin = 0x30;
    inline constexpr std::ptrdiff_t kOffsetItemTablePillsEnd = 0x34;

    // ------------------------------------------------------------------
    // EntityConfig
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetEntityConfigId = 0x0;
    inline constexpr std::ptrdiff_t kOffsetEntityConfigVariant = 0x4;
    inline constexpr std::ptrdiff_t kOffsetEntityConfigSubtype = 0x8;
    inline constexpr std::ptrdiff_t kOffsetEntityConfigName = 0xC;

    // ------------------------------------------------------------------
    // Game
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetGamePlayerManager = 0x1BAA8;
    inline constexpr std::ptrdiff_t kOffsetGameGameOverState = 0x1D520;
    inline constexpr std::ptrdiff_t kOffsetGameCurrentRoom = 0x18300;
    inline constexpr std::ptrdiff_t kOffsetGameCurrentRoomIndex = 0x18304;
    inline constexpr std::ptrdiff_t kOffsetGameItemPool = 0x1A740;

    // ------------------------------------------------------------------
    // Manager
    // ------------------------------------------------------------------

    // Visit sibling code pattern (captured absolute):
    // 8d 5e 01 53 50 68 84 9f b6 00 8d 8f (20 a9 04 00) e8 88 6a 2f 00 80 7d ff 00 75 1e 8d 45 ff c6 45 ff 00
    inline constexpr std::ptrdiff_t kOffsetManagerStringTable = 0x4A920;

    inline constexpr std::ptrdiff_t kOffsetManagerItemConfigManager = 0x2A404;
    inline constexpr std::ptrdiff_t kOffsetManagerEntityConfigManager = 0x2A670;
    inline constexpr std::ptrdiff_t kOffsetManagerNetPlayManager = 0x4B3D8;
    inline constexpr std::ptrdiff_t kOffsetManagerProfileNameFormat = 0x4AE28;
    inline constexpr std::ptrdiff_t kOffsetManagerGameState = 0x08;
    // GameState (at this+0xfa4) +0x1fdec: normal continue state (died/quit mid-run -> rep+%s loaded)
    inline constexpr std::ptrdiff_t kOffsetManagerContinueState = 0x20D90;
    // GameState (at this+0xfa4) +0x1fded: rerun state (previous run won -> rep+rerunstate%d.dat loaded)
    inline constexpr std::ptrdiff_t kOffsetManagerRerunState = 0x20D91;

    // ------------------------------------------------------------------
    // NetDevice (online device descriptor)
    // ------------------------------------------------------------------

    // NetDevice: device_id at +0x0C, profile holder pointer at +0x374 (holder = [device + 0x370 + 0x4]).
    inline constexpr std::ptrdiff_t kOffsetDeviceProfileHolder = 0x374;

    // Profile holder: SteamID64 low/high at +0x08 / +0x0C.
    inline constexpr std::ptrdiff_t kOffsetProfileSteamIdLow = 0x08;
    inline constexpr std::ptrdiff_t kOffsetProfileSteamIdHigh = 0x0C;

    // ------------------------------------------------------------------
    // Room
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetRoomEntityList = 0x1218;
    inline constexpr std::ptrdiff_t kOffsetRoomActiveEntityList = 0x40;
    inline constexpr std::ptrdiff_t kOffsetRoomListBegin = 0x4;
    inline constexpr std::ptrdiff_t kOffsetRoomListSize = 0xC;

    // ------------------------------------------------------------------
    // StringTable
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetStringTableLanguage = 0x00;

    // ------------------------------------------------------------------
    // PlayerManager
    // ------------------------------------------------------------------

    inline constexpr std::ptrdiff_t kOffsetPlayerManagerTwin = 0x1E68;
}

#endif // ISAACSPY_GAME_CONSTANTS_H