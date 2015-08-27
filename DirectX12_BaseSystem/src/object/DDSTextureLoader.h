//--------------------------------------------------------------------------------------
// File: DDSTextureLoader.h
//
// Functions for loading a DDS texture and creating a Direct3D 11 runtime resource for it
//
// Note these functions are useful as a light-weight runtime loader for DDS files. For
// a full-featured DDS file reader, writer, and texture processing pipeline see
// the 'Texconv' sample and the 'DirectXTex' library.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// http://go.microsoft.com/fwlink/?LinkId=248926
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#ifdef _MSC_VER
#pragma once
#endif

#include <d3d11_1.h>

#pragma warning(push)
#pragma warning(disable : 4005)
#include <stdint.h>
#include <string>
#include <vector>
#pragma warning(pop)

#if defined(_MSC_VER) && (_MSC_VER<1610) && !defined(_In_reads_)
#define _In_reads_(exp)
#define _Out_writes_(exp)
#define _In_reads_bytes_(exp)
#define _In_reads_opt_(exp)
#define _Outptr_opt_
#endif

#ifndef _Use_decl_annotations_
#define _Use_decl_annotations_
#endif

struct DDS_HEADER;

namespace DirectX
{
	void CreateTextureData(const std::string& fileName,std::vector<UINT8>* textureData);
}