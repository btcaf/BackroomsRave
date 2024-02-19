#include "util.h"

#include <wincodec.h>

void hr_check(HRESULT hr) {
	if (FAILED(hr)) {
		exit(1);
	}
}

BYTE* LoadBitmapFromFile(PCWSTR uri, UINT& width, UINT& height) {
    IWICImagingFactory* pIWICFactory = NULL;
    IWICBitmapDecoder* pDecoder = NULL;
    IWICBitmapFrameDecode* pSource = NULL;
    IWICFormatConverter* pConverter = NULL;

    hr_check(CoCreateInstance(
        CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pIWICFactory)
    ));

    hr_check(pIWICFactory->CreateDecoderFromFilename(
        uri, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad,
        &pDecoder
    ));

    hr_check(pDecoder->GetFrame(0, &pSource));

    hr_check(pIWICFactory->CreateFormatConverter(&pConverter));

    hr_check(pConverter->Initialize(
        pSource,
        GUID_WICPixelFormat32bppRGBA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeMedianCut
    ));

    hr_check(pConverter->GetSize(&width, &height));

    BYTE* res = new BYTE[4 * width * height];
    hr_check(pConverter->CopyPixels(
        nullptr, 4 * width, 4 * width * height, res
    ));


    if (pIWICFactory) pIWICFactory->Release();
    if (pDecoder) pDecoder->Release();
    if (pSource) pSource->Release();
    if (pConverter) pConverter->Release();

    return res;
}

