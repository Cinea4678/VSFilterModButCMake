// Cross-platform VSFilterMod AviSynth + VapourSynth plugin
// Based on src/vsfilter/plugins.cpp with Windows-specific code removed

#ifdef _WIN32
#include "stdafx.h"
#else
#include "../compat/compat.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <filesystem>
#define DNew new
#endif

#include "../subtitles/VobSubFile.h"
#include "../subtitles/RTS.h"
#include "../subtitles/SSF.h"
#include "../subpic/MemSubPic.h"
#include "../vsfilter/vfr.h"

//
// Generic cross-platform filter base
//

namespace Plugin
{

class CFilter
{
private:
    CString m_fn;

protected:
    float m_fps;
    CCritSec m_csSubLock;
    CComPtr<ISubPicQueue> m_pSubPicQueue;
    CComPtr<ISubPicProvider> m_pSubPicProvider;
    DWORD_PTR m_SubPicProviderId;

    CSimpleTextSubtitle::YCbCrMatrix m_script_selected_yuv;
    CSimpleTextSubtitle::YCbCrRange m_script_selected_range;

public:
    CFilter() : m_fps(-1), m_SubPicProviderId(0)
    {
        m_script_selected_yuv = CSimpleTextSubtitle::YCbCrMatrix_BT601;
        m_script_selected_range = CSimpleTextSubtitle::YCbCrRange_TV;
    }
    virtual ~CFilter() {}

    CString GetFileName()
    {
        CAutoLock cAutoLock(&m_csSubLock);
        return m_fn;
    }
    void SetFileName(CString fn)
    {
        CAutoLock cAutoLock(&m_csSubLock);
        m_fn = fn;
    }

    bool Render(SubPicDesc& dst, REFERENCE_TIME rt, float fps)
    {
        if (!m_pSubPicProvider)
            return false;

        CSize size(dst.w, dst.h);

        if (!m_pSubPicQueue)
        {
            CComPtr<ISubPicAllocator> pAllocator = new CMemSubPicAllocator(
                dst.type, size, m_script_selected_yuv, m_script_selected_range);

            HRESULT hr;
            if (!(m_pSubPicQueue = new CSubPicQueueNoThread(pAllocator, &hr)) || FAILED(hr))
            {
                m_pSubPicQueue = NULL;
                return false;
            }
        }

        if (m_SubPicProviderId != (DWORD_PTR)(ISubPicProvider*)m_pSubPicProvider)
        {
            m_pSubPicQueue->SetSubPicProvider(m_pSubPicProvider);
            m_SubPicProviderId = (DWORD_PTR)(ISubPicProvider*)m_pSubPicProvider;
        }

        CComPtr<ISubPic> pSubPic;
        if (!m_pSubPicQueue->LookupSubPic(rt, pSubPic))
            return false;

        CRect r;
        pSubPic->GetDirtyRect(r);

        if (dst.type == MSP_RGB32 || dst.type == MSP_RGB24 || dst.type == MSP_RGB16 || dst.type == MSP_RGB15)
            dst.h = -dst.h;

        pSubPic->AlphaBlt(r, r, &dst);
        return true;
    }
};

class CVobSubFilter : virtual public CFilter
{
public:
    CVobSubFilter(CString fn = _T(""))
    {
        if (!fn.IsEmpty()) Open(fn);
    }

    bool Open(CString fn)
    {
        SetFileName(_T(""));
        m_pSubPicProvider = NULL;

        if (CVobSubFile* vsf = new CVobSubFile(&m_csSubLock))
        {
            m_pSubPicProvider = (ISubPicProvider*)vsf;
            if (vsf->Open(CString(fn))) SetFileName(fn);
            else m_pSubPicProvider = NULL;
        }

        return !!m_pSubPicProvider;
    }
};

class CTextSubFilter : virtual public CFilter
{
    int m_CharSet;

public:
    CTextSubFilter(CString fn = _T(""), int CharSet = DEFAULT_CHARSET, float fps = -1)
        : m_CharSet(CharSet)
    {
        m_fps = fps;
        if (!fn.IsEmpty()) Open(fn, CharSet);
    }

    int GetCharSet() { return m_CharSet; }

    bool Open(CString fn, int CharSet = DEFAULT_CHARSET)
    {
        SetFileName(_T(""));
        m_pSubPicProvider = NULL;

        if (!m_pSubPicProvider)
        {
            if (ssf::CRenderer* ssf = new ssf::CRenderer(&m_csSubLock))
            {
                m_pSubPicProvider = (ISubPicProvider*)ssf;
                if (ssf->Open(CString(fn))) SetFileName(fn);
                else m_pSubPicProvider = NULL;
            }
        }

        if (!m_pSubPicProvider)
        {
            if (CRenderedTextSubtitle* rts = new CRenderedTextSubtitle(&m_csSubLock))
            {
                m_pSubPicProvider = (ISubPicProvider*)rts;
                if (rts->Open(CString(fn), CharSet)) SetFileName(fn);
                else m_pSubPicProvider = NULL;

                m_script_selected_yuv = rts->m_eYCbCrMatrix;
                m_script_selected_range = rts->m_eYCbCrRange;
            }
        }

        return !!m_pSubPicProvider;
    }
};

} // namespace Plugin

//
// AviSynth+ interface (cross-platform)
//

#ifndef _WIN32

// Suppress macro redefinition warnings from avs/posix.h vs our compat layer
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmacro-redefined"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#endif

namespace AviSynthPlus {
#include "avisynthplus.h"

const AVS_Linkage* AVS_linkage = nullptr;

static bool s_fSwapUV = false;

class CAvisynthFilter : public GenericVideoFilter, virtual public Plugin::CFilter
{
public:
    VFRTranslator *vfr;

    CAvisynthFilter(PClip c, IScriptEnvironment* env, VFRTranslator *_vfr = 0) : GenericVideoFilter(c), vfr(_vfr) {}

    PVideoFrame __stdcall GetFrame(int n, IScriptEnvironment* env)
    {
        PVideoFrame frame = child->GetFrame(n, env);

        env->MakeWritable(&frame);

        SubPicDesc dst;
        dst.w = vi.width;
        dst.h = vi.height;
        dst.pitch = frame->GetPitch();
        dst.pitchUV = frame->GetPitch(PLANAR_U);
        dst.bits = (void**)frame->GetWritePtr();
        dst.bitsU = frame->GetWritePtr(PLANAR_U);
        dst.bitsV = frame->GetWritePtr(PLANAR_V);
        dst.bpp = dst.pitch / dst.w * 8;
        dst.type =
            vi.IsRGB32() ? MSP_RGB32 :
            vi.IsRGB24() ? MSP_RGB24 :
            vi.IsYUY2() ? MSP_YUY2 :
            vi.IsYV12() ? (s_fSwapUV ? MSP_IYUV : MSP_YV12) :
            -1;

        float fps = m_fps > 0 ? m_fps : (float)vi.fps_numerator / vi.fps_denominator;

        REFERENCE_TIME timestamp;

        if (!vfr)
            timestamp = (REFERENCE_TIME)(10000000LL * n / fps);
        else
            timestamp = (REFERENCE_TIME)(10000000.0 * vfr->TimeStampFromFrameNumber(n));

        Render(dst, timestamp, fps);

        return frame;
    }
};

class CVobSubAvisynthFilter : public Plugin::CVobSubFilter, public CAvisynthFilter
{
public:
    CVobSubAvisynthFilter(PClip c, const char* fn, IScriptEnvironment* env)
        : CVobSubFilter(CString(fn))
        , CAvisynthFilter(c, env)
    {
        if (!m_pSubPicProvider)
            env->ThrowError("VobSub: Can't open \"%s\"", fn);
    }
};

class CTextSubAvisynthFilter : public Plugin::CTextSubFilter, public CAvisynthFilter
{
public:
    CTextSubAvisynthFilter(PClip c, IScriptEnvironment* env, const char* fn, int CharSet = DEFAULT_CHARSET, float fps = -1, VFRTranslator *vfr = 0)
        : CTextSubFilter(CString(fn), CharSet, fps)
        , CAvisynthFilter(c, env, vfr)
    {
        if (!m_pSubPicProvider)
            env->ThrowError("TextSubMod: Can't open \"%s\"", fn);
    }
};

AVSValue __cdecl VobSubCreateS(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    return(new CVobSubAvisynthFilter(args[0].AsClip(), args[1].AsString(), env));
}

AVSValue __cdecl TextSubCreateGeneral(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    if (!args[1].Defined())
        env->ThrowError("TextSubMod: You must specify a subtitle file to use");
    VFRTranslator *vfr = 0;
    if (args[4].Defined())
        vfr = GetVFRTranslator(args[4].AsString());

    return(new CTextSubAvisynthFilter(
               args[0].AsClip(),
               env,
               args[1].AsString(),
               args[2].AsInt(DEFAULT_CHARSET),
               args[3].AsFloat(-1),
               vfr));
}

AVSValue __cdecl TextSubSwapUV(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    s_fSwapUV = args[0].AsBool(false);
    return(AVSValue());
}

AVSValue __cdecl MaskSubCreate(AVSValue args, void* user_data, IScriptEnvironment* env)
{
    if (!args[0].Defined())
        env->ThrowError("MaskSubMod: You must specify a subtitle file to use");
    if (!args[3].Defined() && !args[6].Defined())
        env->ThrowError("MaskSubMod: You must specify either FPS or a VFR timecodes file");
    VFRTranslator *vfr = 0;
    if (args[6].Defined())
        vfr = GetVFRTranslator(args[6].AsString());

    AVSValue rgb32("RGB32");
    AVSValue fps(args[3].AsFloat(25));
    AVSValue tab[5] =
    {
        args[1],
        args[2],
        args[3],
        args[4],
        rgb32
    };
    AVSValue value(tab, 5);
    const char * nom[5] =
    {
        "width",
        "height",
        "fps",
        "length",
        "pixel_type"
    };
    AVSValue clip(env->Invoke("Blackness", value, nom));
    env->SetVar(env->SaveString("RGBA"), true);
    return(new CTextSubAvisynthFilter(
               clip.AsClip(),
               env,
               args[0].AsString(),
               args[5].AsInt(DEFAULT_CHARSET),
               args[3].AsFloat(-1),
               vfr));
}

extern "C" __attribute__((visibility("default")))
const char* AvisynthPluginInit3(IScriptEnvironment* env, const AVS_Linkage* const vectors)
{
    AVS_linkage = vectors;

    env->AddFunction("VobSub", "cs", VobSubCreateS, 0);
    env->AddFunction("TextSubMod", "c[file]s[charset]i[fps]f[vfr]s", TextSubCreateGeneral, 0);
    env->AddFunction("TextSubModSwapUV", "b", TextSubSwapUV, 0);
    env->AddFunction("MaskSubMod", "[file]s[width]i[height]i[fps]f[length]i[charset]i[vfr]s", MaskSubCreate, 0);
    env->SetVar(env->SaveString("RGBA"), false);
    return(NULL);
}

} // namespace AviSynthPlus

#ifdef __clang__
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif // !_WIN32

//
// VapourSynth interface
//

#ifndef _WIN32
// UTF-8 to wchar_t conversion for non-Windows
static std::wstring utf8_to_wstring(const char* utf8) {
    std::wstring result;
    if (!utf8) return result;
    const unsigned char* p = (const unsigned char*)utf8;
    while (*p) {
        wchar_t ch;
        if (*p < 0x80) {
            ch = *p++;
        } else if (*p < 0xE0) {
            ch = (*p++ & 0x1F) << 6;
            if (*p) ch |= (*p++ & 0x3F);
        } else if (*p < 0xF0) {
            ch = (*p++ & 0x0F) << 12;
            if (*p) { ch |= (*p++ & 0x3F) << 6; }
            if (*p) { ch |= (*p++ & 0x3F); }
        } else {
            // Skip 4-byte sequences (outside BMP)
            p++; if (*p) p++; if (*p) p++; if (*p) p++;
            ch = L'?';
        }
        result += ch;
    }
    return result;
}
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#elif defined(__aarch64__)
#include "sse2neon.h"
#endif

namespace VapourSynth {
#include <VapourSynth.h>
#include <VSHelper.h>

    class CTextSubVapourSynthFilter : public Plugin::CTextSubFilter {
    public:
        CTextSubVapourSynthFilter(const wchar_t* file, const int charset, const float fps, int* error)
            : CTextSubFilter(CString(file), charset, fps) {
            *error = !m_pSubPicProvider ? 1 : 0;
        }
    };

    class CVobSubVapourSynthFilter : public Plugin::CVobSubFilter {
    public:
        CVobSubVapourSynthFilter(const wchar_t* file, int* error)
            : CVobSubFilter(CString(file)) {
            *error = !m_pSubPicProvider ? 1 : 0;
        }
    };

    struct VSFilterData {
        VSNodeRef* node;
        const VSVideoInfo* vi;
        float fps;
        VFRTranslator* vfr;
        CTextSubVapourSynthFilter* textsub;
        CVobSubVapourSynthFilter* vobsub;
        bool accurate16bit;
    };

    static void VS_CC vsfilterInit(VSMap* in, VSMap* out, void** instanceData, VSNode* node, VSCore* core, const VSAPI* vsapi) {
        VSFilterData* d = static_cast<VSFilterData*>(*instanceData);
        vsapi->setVideoInfo(d->vi, 1, node);
    }

    // Simplified frame buffer - YUV 8-bit only for cross-platform initial version
    static const VSFrameRef* VS_CC vsfilterGetFrame(int n, int activationReason, void** instanceData, void** frameData, VSFrameContext* frameCtx, VSCore* core, const VSAPI* vsapi) {
        const VSFilterData* d = static_cast<const VSFilterData*>(*instanceData);

        if (activationReason == arInitial) {
            vsapi->requestFrameFilter(n, d->node, frameCtx);
        } else if (activationReason == arAllFramesReady) {
            const VSFrameRef* src = vsapi->getFrameFilter(n, d->node, frameCtx);
            VSFrameRef* dst = vsapi->copyFrame(src, core);

            // Only support YUV420P8 and RGB24 for now
            if (d->vi->format->id == pfYUV420P8) {
                SubPicDesc subpic = {};
                subpic.w = d->vi->width;
                subpic.h = d->vi->height;
                subpic.pitch = vsapi->getStride(dst, 0);
                subpic.pitchUV = vsapi->getStride(dst, 1);
                subpic.bits = (void*)vsapi->getWritePtr(dst, 0);
                subpic.bitsU = vsapi->getWritePtr(dst, 1);
                subpic.bitsV = vsapi->getWritePtr(dst, 2);
                subpic.bpp = 8;
                subpic.type = MSP_YV12;

                REFERENCE_TIME timestamp;
                if (!d->vfr)
                    timestamp = static_cast<REFERENCE_TIME>(10000000LL * n / d->fps);
                else
                    timestamp = static_cast<REFERENCE_TIME>(10000000.0 * d->vfr->TimeStampFromFrameNumber(n));

                if (d->textsub)
                    d->textsub->Render(subpic, timestamp, d->fps);
                else
                    d->vobsub->Render(subpic, timestamp, d->fps);
            }

            vsapi->freeFrame(src);
            return dst;
        }
        return nullptr;
    }

    static void VS_CC vsfilterFree(void* instanceData, VSCore* core, const VSAPI* vsapi) {
        VSFilterData* d = static_cast<VSFilterData*>(instanceData);
        vsapi->freeNode(d->node);
        delete d->textsub;
        delete d->vobsub;
        if (d->vfr) delete d->vfr;
        delete d;
    }

    static void VS_CC vsfilterCreate(const VSMap* in, VSMap* out, void* userData, VSCore* core, const VSAPI* vsapi) {
        std::unique_ptr<VSFilterData> ud(new VSFilterData{});
        VSFilterData& d = *ud;
        int err;

        const std::string filterName{static_cast<const char*>(userData)};

        d.node = vsapi->propGetNode(in, "clip", 0, nullptr);
        d.vi = vsapi->getVideoInfo(d.node);

        if (!isConstantFormat(d.vi) || (d.vi->format->id != pfYUV420P8 && d.vi->format->id != pfRGB24)) {
            vsapi->setError(out, (filterName + ": only constant format YUV420P8 and RGB24 input supported on this platform").c_str());
            vsapi->freeNode(d.node);
            return;
        }

        const char* _file = vsapi->propGetData(in, "file", 0, nullptr);

#ifdef _WIN32
        int size = MultiByteToWideChar(CP_UTF8, 0, _file, -1, nullptr, 0);
        wchar_t* file = new wchar_t[size];
        MultiByteToWideChar(CP_UTF8, 0, _file, -1, file, size);
        if (!PathFileExistsW(file)) {
            delete[] file;
            size = MultiByteToWideChar(CP_ACP, 0, _file, -1, nullptr, 0);
            file = new wchar_t[size];
            MultiByteToWideChar(CP_ACP, 0, _file, -1, file, size);
        }
#else
        std::wstring wfile = utf8_to_wstring(_file);
        const wchar_t* file = wfile.c_str();
#endif

        int charset = int64ToIntS(vsapi->propGetInt(in, "charset", 0, &err));
        if (err) charset = DEFAULT_CHARSET;

        float fps = static_cast<float>(vsapi->propGetFloat(in, "fps", 0, &err));
        if (err) fps = -1.f;
        d.fps = (fps > 0.f || !d.vi->fpsNum) ? fps : static_cast<float>(d.vi->fpsNum) / d.vi->fpsDen;

        const char* vfr = vsapi->propGetData(in, "vfr", 0, &err);
        if (!err) d.vfr = GetVFRTranslator(vfr);

        if (!d.vi->fpsNum && fps <= 0.f && !d.vfr) {
            vsapi->setError(out, (filterName + ": variable framerate clip must have fps or vfr specified").c_str());
            vsapi->freeNode(d.node);
            return;
        }

        if (filterName == "TextSubMod")
            d.textsub = new CTextSubVapourSynthFilter{file, charset, fps, &err};
        else
            d.vobsub = new CVobSubVapourSynthFilter{file, &err};

        if (err) {
            vsapi->setError(out, (filterName + ": can't open " + _file).c_str());
            vsapi->freeNode(d.node);
            delete d.textsub;
            delete d.vobsub;
            return;
        }

        d.accurate16bit = vsapi->propGetInt(in, "accurate", 0, &err) != 0;
        if (err) d.accurate16bit = false;

        vsapi->createFilter(in, out, static_cast<const char*>(userData),
            vsfilterInit, vsfilterGetFrame, vsfilterFree, fmParallelRequests, 0, ud.release(), core);

#ifdef _WIN32
        delete[] file;
#endif
    }

    VS_EXTERNAL_API(void) VapourSynthPluginInit(VSConfigPlugin configFunc, VSRegisterFunction registerFunc, VSPlugin* plugin) {
        configFunc("com.holywu.vsfiltermod", "vsfm", "VSFilterMod", VAPOURSYNTH_API_VERSION, 1, plugin);
        registerFunc("TextSubMod",
            "clip:clip;"
            "file:data;"
            "charset:int:opt;"
            "fps:float:opt;"
            "vfr:data:opt;"
            "accurate:int:opt;",
            vsfilterCreate, const_cast<char*>("TextSubMod"), plugin);
        registerFunc("VobSub",
            "clip:clip;"
            "file:data;"
            "accurate:int:opt;",
            vsfilterCreate, const_cast<char*>("VobSub"), plugin);
    }
}
