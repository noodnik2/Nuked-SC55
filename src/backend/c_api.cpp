// Implements the interface in include/nuked-sc55/nuked-sc55.h

#include "emu.h"
#include "nuked-sc55/nuked-sc55.h"

#ifdef _WIN32
#include <Windows.h>
#endif

extern "C" struct SC55_Emulator
{
    Emulator emu;

    // We use a different type for the callback in the C API for ABI reasons. Since the function pointer types are
    // incompatible we need to convert from AudioFrame to a pair of ints. One upside of this is that the C API is
    // insulated from whatever changes we make to the callback interface on the C++ side of things.
    SC55_SampleCallback callback;
    void*               userdata;
};

extern "C" SC55_Error SC55_Create(SC55_Emulator** out_emu)
{
    *out_emu = new SC55_Emulator();
    if (!*out_emu)
    {
        return SC55_ALLOC_FAILED;
    }

    // TODO: C API should probably also support passing emu options
    if (!(*out_emu)->emu.Init({}))
    {
        return SC55_ALLOC_FAILED;
    }

    return SC55_OK;
}

extern "C" void SC55_Destroy(SC55_Emulator* emu)
{
    if (emu)
    {
        delete emu;
    }
}

static SC55_Error LoadRomsTypeToRomset(SC55_LoadRomsType type, Romset& out)
{
    switch (type)
    {
    case SC55_LOADROMS_AUTODETECT:
        return SC55_INVALID_PARAM;
    case SC55_LOADROMS_SC55MK1:
        out = Romset::MK1;
        return SC55_OK;
    case SC55_LOADROMS_SC55MK2:
        out = Romset::MK2;
        return SC55_OK;
    case SC55_LOADROMS_ST:
        out = Romset::ST;
        return SC55_OK;
    case SC55_LOADROMS_CM300:
        out = Romset::CM300;
        return SC55_OK;
    case SC55_LOADROMS_JV880:
        out = Romset::JV880;
        return SC55_OK;
    case SC55_LOADROMS_SCB55:
        out = Romset::SCB55;
        return SC55_OK;
    case SC55_LOADROMS_RLP3237:
        out = Romset::RLP3237;
        return SC55_OK;
    case SC55_LOADROMS_SC155:
        out = Romset::SC155;
        return SC55_OK;
    case SC55_LOADROMS_SC155MK2:
        out = Romset::SC155MK2;
        return SC55_OK;
    }
    return SC55_INVALID_PARAM;
}

extern "C" SC55_Error SC55_LoadRoms(SC55_Emulator* emu, const char* directory, SC55_LoadRomsType type)
{
    std::filesystem::path dir_path;

    try
    {
#ifdef _WIN32
        // On Windows we need to convert the UTF-8 string to UTF-16.
        const size_t in_size = strlen(directory);

        int out_size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, directory, (int)in_size, nullptr, 0);
        if (out_size == 0)
        {
            return SC55_INVALID_PARAM;
        }

        std::wstring out_utf16;
        out_utf16.resize(out_size);

        int cvt_err =
            MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, directory, (int)in_size, out_utf16.data(), out_size);
        if (cvt_err == 0)
        {
            return SC55_INVALID_PARAM;
        }

        dir_path = out_utf16;
#else
        dir_path = directory;
#endif
    }
    catch (...)
    {
        return SC55_ALLOC_FAILED;
    }

    Romset rs;

    if (type == SC55_LOADROMS_AUTODETECT)
    {
        rs = EMU_DetectRomset(dir_path);
    }
    else
    {
        SC55_Error err = LoadRomsTypeToRomset(type, rs);
        if (err)
        {
            return err;
        }
    }

    if (!emu->emu.LoadRoms(rs, dir_path))
    {
        return SC55_LOADROMS_FAILED;
    }

    return SC55_OK;
}

void ProxyCallback(void* our_userdata, const AudioFrame<int32_t>& frame)
{
    SC55_Emulator* emu = (SC55_Emulator*)our_userdata;
    emu->callback(emu->userdata, frame.left, frame.right);
};

extern "C" void SC55_SetSampleCallback(SC55_Emulator* emu, SC55_SampleCallback callback, void* userdata)
{
    emu->callback = callback;
    emu->userdata = userdata;
    emu->emu.SetSampleCallback(ProxyCallback, emu);
}

extern "C" void SC55_Step(SC55_Emulator* emu)
{
    emu->emu.Step();
}

extern "C" void SC55_PostMIDI(SC55_Emulator* emu, const void* ptr, size_t count)
{
    const uint8_t* as_u8 = (const uint8_t*)ptr;
    emu->emu.PostMIDI(std::span<const uint8_t>(as_u8, count));
}

extern "C" uint32_t SC55_GetOutputFrequency(SC55_Emulator* emu)
{
    return PCM_GetOutputFrequency(emu->emu.GetPCM());
}
