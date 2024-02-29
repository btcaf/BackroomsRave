#include "util.h"

#include <wincodec.h>
#include <comdef.h>

void hr_check(HRESULT hr) {
	if (FAILED(hr)) {
        PostQuitMessage(1);
	}
}

BYTE* load_bitmap(PCWSTR uri, UINT& width, UINT& height) {
    IWICImagingFactory* wic_factory = nullptr;
    IWICBitmapDecoder* wic_decoder = nullptr;
    IWICBitmapFrameDecode* wic_frame = nullptr;
    IWICFormatConverter* wic_converter = nullptr;

    hr_check(CoCreateInstance(
        CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&wic_factory)
    ));

    hr_check(wic_factory->CreateDecoderFromFilename(
        uri, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
        &wic_decoder
    ));

    hr_check(wic_decoder->GetFrame(0, &wic_frame));

    hr_check(wic_factory->CreateFormatConverter(&wic_converter));

    hr_check(wic_converter->Initialize(
        wic_frame,
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeMedianCut
    ));

    hr_check(wic_converter->GetSize(&width, &height));

    BYTE* res = new BYTE[4 * width * height];
    hr_check(wic_converter->CopyPixels(
        nullptr, 4 * width, 4 * width * height, res
    ));


    if (wic_factory) wic_factory->Release();
    if (wic_decoder) wic_decoder->Release();
    if (wic_frame) wic_frame->Release();
    if (wic_converter) wic_converter->Release();

    return res;
}

