/*
 * Copyright (C) 2021, 2024 nukeykt
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
#include "emu.h"
#include "mcu.h"
#include "submcu.h"
#include "mcu_timer.h"
#include "lcd.h"
#include "pcm.h"
#include <string>
#include <fstream>
#include <span>

bool EMU_Init(emu_t& emu)
{
    return EMU_Init(emu, EMU_Options {
        .want_lcd = true,
    });
}

bool EMU_Init(emu_t& emu, const EMU_Options& options)
{
    emu.mcu   = std::make_unique<mcu_t>();
    emu.sm    = std::make_unique<submcu_t>();
    emu.timer = std::make_unique<mcu_timer_t>();
    emu.lcd   = std::make_unique<lcd_t>();
    emu.pcm   = std::make_unique<pcm_t>();

    if (!MCU_Init(*emu.mcu, *emu.sm, *emu.pcm, *emu.timer, *emu.lcd))
    {
        return false;
    }

    SM_Init(*emu.sm, *emu.mcu);
    PCM_Init(*emu.pcm, *emu.mcu);
    TIMER_Init(*emu.timer, *emu.mcu);
    LCD_Init(*emu.lcd, *emu.mcu);

    return true;
}

void EMU_Reset(emu_t& emu)
{
    MCU_PatchROM(*emu.mcu);
    MCU_Reset(*emu.mcu);
    SM_Reset(*emu.sm);
}

void EMU_SetSampleCallback(emu_t& emu, mcu_sample_callback callback, void* userdata)
{
    emu.mcu->callback_userdata = userdata;
    emu.mcu->sample_callback = callback;
}

const char* rs_name[(size_t)ROMSET_COUNT] = {
    "SC-55mk2",
    "SC-55st",
    "SC-55mk1",
    "CM-300/SCC-1",
    "JV-880",
    "SCB-55",
    "RLP-3237",
    "SC-155",
    "SC-155mk2"
};

const char* roms[(size_t)ROMSET_COUNT][5] =
{
    {
        "rom1.bin",
        "rom2.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
    },

    {
        "rom1.bin",
        "rom2_st.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
    },

    {
        "sc55_rom1.bin",
        "sc55_rom2.bin",
        "sc55_waverom1.bin",
        "sc55_waverom2.bin",
        "sc55_waverom3.bin",
    },

    {
        "cm300_rom1.bin",
        "cm300_rom2.bin",
        "cm300_waverom1.bin",
        "cm300_waverom2.bin",
        "cm300_waverom3.bin",
    },

    {
        "jv880_rom1.bin",
        "jv880_rom2.bin",
        "jv880_waverom1.bin",
        "jv880_waverom2.bin",
        "jv880_waverom_expansion.bin",
    },

    {
        "scb55_rom1.bin",
        "scb55_rom2.bin",
        "scb55_waverom1.bin",
        "scb55_waverom2.bin",
        "",
    },

    {
        "rlp3237_rom1.bin",
        "rlp3237_rom2.bin",
        "rlp3237_waverom1.bin",
        "",
        "",
    },

    {
        "sc155_rom1.bin",
        "sc155_rom2.bin",
        "sc155_waverom1.bin",
        "sc155_waverom2.bin",
        "sc155_waverom3.bin",
    },

    {
        "rom1.bin",
        "rom2.bin",
        "waverom1.bin",
        "waverom2.bin",
        "rom_sm.bin",
    },
};

void unscramble(uint8_t *src, uint8_t *dst, int len)
{
    for (int i = 0; i < len; i++)
    {
        int address = i & ~0xfffff;
        static const int aa[] = {
            2, 0, 3, 4, 1, 9, 13, 10, 18, 17, 6, 15, 11, 16, 8, 5, 12, 7, 14, 19
        };
        for (int j = 0; j < 20; j++)
        {
            if (i & (1 << j))
                address |= 1<<aa[j];
        }
        uint8_t srcdata = src[address];
        uint8_t data = 0;
        static const int dd[] = {
            2, 0, 4, 5, 7, 6, 3, 1
        };
        for (int j = 0; j < 8; j++)
        {
            if (srcdata & (1 << dd[j]))
                data |= 1<<j;
        }
        dst[i] = data;
    }
}

Romset EMU_DetectRomset(const std::filesystem::path& base_path)
{
    for (size_t i = 0; i < (size_t)ROMSET_COUNT; i++)
    {
        bool good = true;
        for (size_t j = 0; j < 5; j++)
        {
            if (roms[i][j][0] == '\0')
                continue;
            if (!std::filesystem::exists(base_path / roms[i][j]))
            {
                good = false;
                break;
            }
        }
        if (good)
        {
            return (Romset)i;
        }
    }
    return Romset::MK2;
}

bool EMU_ReadStreamExact(std::ifstream& s, void* into, std::streamsize byte_count)
{
    if (s.read((char*)into, byte_count))
    {
        return s.gcount() == byte_count;
    }
    return false;
}

bool EMU_ReadStreamExact(std::ifstream& s, std::span<uint8_t> into, std::streamsize byte_count)
{
    return EMU_ReadStreamExact(s, into.data(), byte_count);
}

std::streamsize EMU_ReadStreamUpTo(std::ifstream& s, void* into, std::streamsize byte_count)
{
    if (s.read((char*)into, byte_count))
    {
        return s.gcount();
    }
    return 0;
}

bool EMU_LoadRoms(emu_t& emu, Romset romset, const std::filesystem::path& base_path)
{
    std::vector<uint8_t> tempbuf(0x800000);

    const size_t rf_num = 5;
    std::ifstream s_rf[rf_num];

    emu.mcu->romset = romset;
    emu.mcu->mcu_mk1 = false;
    emu.mcu->mcu_cm300 = false;
    emu.mcu->mcu_st = false;
    emu.mcu->mcu_jv880 = false;
    emu.mcu->mcu_scb55 = false;
    emu.mcu->mcu_sc155 = false;
    switch (romset)
    {
        case Romset::MK2:
        case Romset::SC155MK2:
            if (romset == Romset::SC155MK2)
                emu.mcu->mcu_sc155 = true;
            break;
        case Romset::ST:
            emu.mcu->mcu_st = true;
            break;
        case Romset::MK1:
        case Romset::SC155:
            emu.mcu->mcu_mk1 = true;
            emu.mcu->mcu_st = false;
            if (romset == Romset::SC155)
                emu.mcu->mcu_sc155 = true;
            break;
        case Romset::CM300:
            emu.mcu->mcu_mk1 = true;
            emu.mcu->mcu_cm300 = true;
            break;
        case Romset::JV880:
            emu.mcu->mcu_jv880 = true;
            emu.mcu->rom2_mask /= 2; // rom is half the size
            break;
        case Romset::SCB55:
        case Romset::RLP3237:
            emu.mcu->mcu_scb55 = true;
            break;
    }

    std::filesystem::path rpaths[5];

    bool r_ok = true;
    std::string errors_list;

    for(size_t i = 0; i < 5; ++i)
    {
        if (roms[(size_t)romset][i][0] == '\0')
        {
            continue;
        }
        rpaths[i] = base_path / roms[(size_t)romset][i];
        s_rf[i] = std::ifstream(rpaths[i].c_str(), std::ios::binary);
        bool optional = emu.mcu->mcu_jv880 && i == 4;
        r_ok &= optional || s_rf[i];
        if (!s_rf[i])
        {
            if(!errors_list.empty())
                errors_list.append(", ");

            errors_list.append(rpaths[i].generic_string());
        }
    }

    if (!r_ok)
    {
        fprintf(stderr, "FATAL ERROR: One of required data ROM files is missing: %s.\n", errors_list.c_str());
        fflush(stderr);
        return false;
    }

    if (!EMU_ReadStreamExact(s_rf[0], emu.mcu->rom1, ROM1_SIZE))
    {
        fprintf(stderr, "FATAL ERROR: Failed to read the mcu ROM1.\n");
        fflush(stderr);
        return false;
    }

    std::streamsize rom2_read = EMU_ReadStreamUpTo(s_rf[1], emu.mcu->rom2, ROM2_SIZE);

    if (rom2_read == ROM2_SIZE || rom2_read == ROM2_SIZE / 2)
    {
        emu.mcu->rom2_mask = rom2_read - 1;
    }
    else
    {
        fprintf(stderr, "FATAL ERROR: Failed to read the mcu ROM2.\n");
        fflush(stderr);
        return false;
    }

    if (emu.mcu->mcu_mk1)
    {
        if (!EMU_ReadStreamExact(s_rf[2], tempbuf, 0x100000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom1.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), emu.pcm->waverom1, 0x100000);

        if (!EMU_ReadStreamExact(s_rf[3], tempbuf, 0x100000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom2.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), emu.pcm->waverom2, 0x100000);

        if (!EMU_ReadStreamExact(s_rf[4], tempbuf, 0x100000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom3.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), emu.pcm->waverom3, 0x100000);
    }
    else if (emu.mcu->mcu_jv880)
    {
        if (!EMU_ReadStreamExact(s_rf[2], tempbuf, 0x200000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom1.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), emu.pcm->waverom1, 0x200000);

        if (!EMU_ReadStreamExact(s_rf[3], tempbuf, 0x200000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom2.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), emu.pcm->waverom2, 0x200000);

        if (s_rf[4] && EMU_ReadStreamExact(s_rf[4], tempbuf, 0x800000))
            unscramble(tempbuf.data(), emu.pcm->waverom_exp, 0x800000);
        else
            printf("WaveRom EXP not found, skipping it.\n");
    }
    else
    {
        if (!EMU_ReadStreamExact(s_rf[2], tempbuf, 0x200000))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom1.\n");
            fflush(stderr);
            return false;
        }

        unscramble(tempbuf.data(), emu.pcm->waverom1, 0x200000);

        if (s_rf[3])
        {
            if (!EMU_ReadStreamExact(s_rf[3], tempbuf, 0x100000))
            {
                fprintf(stderr, "FATAL ERROR: Failed to read the WaveRom2.\n");
                fflush(stderr);
                return false;
            }

            unscramble(tempbuf.data(), emu.mcu->mcu_scb55 ? emu.pcm->waverom3 : emu.pcm->waverom2, 0x100000);
        }

        if (s_rf[4] && !EMU_ReadStreamExact(s_rf[4], emu.sm->sm_rom, ROMSM_SIZE))
        {
            fprintf(stderr, "FATAL ERROR: Failed to read the sub mcu ROM.\n");
            fflush(stderr);
            return false;
        }
    }

    return true;
}

const char* EMU_RomsetName(Romset romset)
{
    return rs_name[(size_t)romset];
}

void EMU_PostMIDI(emu_t& emu, std::span<const uint8_t> data)
{
    for (uint8_t byte : data)
    {
        MCU_PostUART(*emu.mcu, byte);
    }
}

constexpr uint8_t GM_RESET_SEQ[] = { 0xF0, 0x7E, 0x7F, 0x09, 0x01, 0xF7 };
constexpr uint8_t GS_RESET_SEQ[] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7 };

void EMU_PostSystemReset(emu_t& emu, EMU_SystemReset reset)
{
    switch (reset)
    {
        case EMU_SystemReset::NONE:
            // explicitly do nothing
            break;
        case EMU_SystemReset::GS_RESET:
            EMU_PostMIDI(emu, GS_RESET_SEQ);
            break;
        case EMU_SystemReset::GM_RESET:
            EMU_PostMIDI(emu, GM_RESET_SEQ);
            break;
    }
}

