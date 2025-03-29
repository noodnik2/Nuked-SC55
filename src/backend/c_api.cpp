// Implements the interface in include/nuked-sc55/nuked-sc55.h

#include "emu.h"
#include "nuked-sc55/nuked-sc55.h"

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
    case SC55_LOADROMS_SC55:
        out = Romset::MK1;
        return SC55_OK;
    case SC55_LOADROMS_SC55MK2:
        out = Romset::MK2;
        return SC55_OK;
    case SC55_LOADROMS_JV880:
        out = Romset::JV880;
        return SC55_OK;
    }
    return SC55_INVALID_PARAM;
}

extern "C" SC55_Error SC55_LoadRoms(SC55_Emulator* emu, const char* directory, SC55_LoadRomsType type)
{
    std::filesystem::path dir_path;

    try
    {
        dir_path = directory;
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
        return SC55_LOADROM_FAILED;
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
    // TODO: This should probably be wrapped in the emulator class.
    MCU_Step(emu->emu.GetMCU());
}

extern "C" void SC55_PostMIDI(void* ptr, size_t count)
{
    (void)ptr;
    (void)count;
    // TODO
}
