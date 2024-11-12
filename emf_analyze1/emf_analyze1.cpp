/*=============================================================================
  Copyright (C) 2024 yumetodo <yume-wikijp@live.jp>

  Distributed under the Boost Software License, Version 1.0.
  (See https://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/
#include <Windows.h>
#include <iostream>
#include <memory>
#include <optional>

std::wostream& operator<<(std::wostream& os, const RECTL& rect)
{
    os << L"[" << rect.left << L',' << rect.right << L':' << rect.top << L',' << rect.bottom << L']';
    return os;
}

// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-emf/929b78e1-b848-44a5-9fac-327cae5c2ae5
// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-emfplus/36d74342-6931-44d5-8ded-d3874f6be5dc
struct alignas(4)  EmrCommentEMFPlus {
    DWORD Type;
    DWORD Size;
    DWORD DataSize;
    char EMFPlusSignature[4];
};

// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-emfplus/fa7c00e7-ef14-4070-b12a-cb047d964ebe
struct alignas(4) EmfPlusRecordHeader
{
    WORD Type;
    WORD Flags;
    DWORD Size;
    DWORD DataSize;
};

// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-emfplus/a60a681e-49ae-4103-a8ce-ef9f65a73fc1
struct EmfPlusGraphicsVersion {
    std::uint32_t GraphicsVersion : 12;
    std::uint32_t MetafileSignature: 20;
};
static_assert(sizeof(EmfPlusGraphicsVersion) == 4);

// https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-emfplus/fa7c00e7-ef14-4070-b12a-cb047d964ebe
struct alignas(4) EmfPlusHeader
{
    EmfPlusRecordHeader Header;
    EmfPlusGraphicsVersion Version;
    DWORD EmfPlusFlags;
    DWORD LogicalDpiX;
    DWORD LogicalDpiY;
};
static_assert(sizeof(EmfPlusHeader) == 28);

namespace {
std::optional<EmfPlusHeader> GetEmfPlusHeader(HENHMETAFILE meta, const ENHMETAHEADER& enmhHeader)
{
    if (enmhHeader.nBytes < sizeof(ENHMETAHEADER) + sizeof(EmfPlusHeader)) return std::nullopt;
    auto metaData = std::make_unique<BYTE[]>(enmhHeader.nBytes);
    if (!GetEnhMetaFileBits(meta, enmhHeader.nBytes, metaData.get())) return std::nullopt;
    if (!metaData) return std::nullopt;
    const size_t metaDataHeadOffset = enmhHeader.offDescription + sizeof(ENHMETAHEADER);
    if (enmhHeader.nBytes < metaDataHeadOffset) std::nullopt;
    EmrCommentEMFPlus metaDataHead{};
    std::memcpy(&metaDataHead, metaData.get() + metaDataHeadOffset, sizeof(EmrCommentEMFPlus));
    if (metaDataHead.Type != 0x46 || std::string_view{ metaDataHead.EMFPlusSignature, 4 } != "EMF+") std::nullopt;
    const size_t headerOffset = sizeof(EmrCommentEMFPlus);
    if (enmhHeader.nBytes < metaDataHeadOffset + headerOffset) return std::nullopt;
    EmfPlusHeader emfPlusHeader{};
    std::memcpy(&emfPlusHeader, metaData.get() + metaDataHeadOffset + headerOffset, sizeof(EmfPlusHeader));
    if (emfPlusHeader.Header.Type != 0x4001 || emfPlusHeader.Header.Size != sizeof(EmfPlusHeader)) return std::nullopt;
    return emfPlusHeader;
}
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc != 2) return 1;
    std::wcout.imbue(std::locale(""));

    const auto meta = ::GetEnhMetaFileW(argv[1]);
    ENHMETAHEADER enmhHeader;
    ::GetEnhMetaFileHeader(meta, sizeof(ENHMETAHEADER), &enmhHeader);
    std::wstring description;
    description.resize(enmhHeader.nDescription);
    ::GetEnhMetaFileDescriptionW(meta, enmhHeader.nDescription, description.data());
    std::wcout
        << L"device units: " << enmhHeader.rclBounds << L'\n'
        << L"Picture Frame: " << enmhHeader.rclFrame << L'\n'
        << L"version: " << enmhHeader.nVersion << L'\n'
        << L"description: " << description << std::endl;
    try {
        if (const auto plusHeader = GetEmfPlusHeader(meta, enmhHeader)) {
            std::wcout
                << L"EMF+ header::\n"
                << L"GraphicsVersion: " << plusHeader->Version.GraphicsVersion << L'\n'
                << L"EmfPlusFlags: " << std::hex << plusHeader->EmfPlusFlags << L'\n'
                << L"LogicalDpi: [" << plusHeader->LogicalDpiX << L',' << plusHeader->LogicalDpiY << L']' << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
