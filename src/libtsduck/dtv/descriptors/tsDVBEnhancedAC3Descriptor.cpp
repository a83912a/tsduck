//----------------------------------------------------------------------------
//
// TSDuck - The MPEG Transport Stream Toolkit
// Copyright (c) 2005-2020, Thierry Lelegard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------

#include "tsDVBEnhancedAC3Descriptor.h"
#include "tsDescriptor.h"
#include "tsNames.h"
#include "tsTablesDisplay.h"
#include "tsPSIRepository.h"
#include "tsPSIBuffer.h"
#include "tsDuckContext.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"DVB_enhanced_AC3_descriptor"
#define MY_XML_NAME_LEGACY u"enhanced_AC3_descriptor"
#define MY_CLASS ts::DVBEnhancedAC3Descriptor
#define MY_DID ts::DID_ENHANCED_AC3
#define MY_STD ts::Standards::DVB

TS_REGISTER_DESCRIPTOR(MY_CLASS, ts::EDID::Standard(MY_DID), MY_XML_NAME, MY_CLASS::DisplayDescriptor, MY_XML_NAME_LEGACY);


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::DVBEnhancedAC3Descriptor::DVBEnhancedAC3Descriptor() :
    AbstractDescriptor(MY_DID, MY_XML_NAME, MY_STD, 0, MY_XML_NAME_LEGACY),
    component_type(),
    bsid(),
    mainid(),
    asvc(),
    mixinfoexists(false),
    substream1(),
    substream2(),
    substream3(),
    additional_info()
{
}

ts::DVBEnhancedAC3Descriptor::DVBEnhancedAC3Descriptor(DuckContext& duck, const Descriptor& desc) :
    DVBEnhancedAC3Descriptor()
{
    deserialize(duck, desc);
}

void ts::DVBEnhancedAC3Descriptor::clearContent()
{
    component_type.clear();
    bsid.clear();
    mainid.clear();
    asvc.clear();
    mixinfoexists = false;
    substream1.clear();
    substream2.clear();
    substream3.clear();
    additional_info.clear();
}


//----------------------------------------------------------------------------
// Merge inside this object missing information which can be found in other object
//----------------------------------------------------------------------------

void ts::DVBEnhancedAC3Descriptor::merge(const DVBEnhancedAC3Descriptor& other)
{
    if (!component_type.set()) {
        component_type = other.component_type;
    }
    if (!bsid.set()) {
        bsid = other.bsid;
    }
    if (!mainid.set()) {
        mainid = other.mainid;
    }
    if (!asvc.set()) {
        asvc = other.asvc;
    }
    mixinfoexists = mixinfoexists || other.mixinfoexists;
    if (!substream1.set()) {
        substream1 = other.substream1;
    }
    if (!substream2.set()) {
        substream2 = other.substream2;
    }
    if (!substream3.set()) {
        substream3 = other.substream3;
    }
    if (additional_info.empty()) {
        additional_info = other.additional_info;
    }
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::DVBEnhancedAC3Descriptor::serializePayload(PSIBuffer& buf) const
{
    buf.putBit(component_type.set());
    buf.putBit(bsid.set());
    buf.putBit(mainid.set());
    buf.putBit(asvc.set());
    buf.putBit(mixinfoexists);
    buf.putBit(substream1.set());
    buf.putBit(substream2.set());
    buf.putBit(substream3.set());
    if (component_type.set()) {
        buf.putUInt8(component_type.value());
    }
    if (bsid.set()) {
        buf.putUInt8(bsid.value());
    }
    if (mainid.set()) {
        buf.putUInt8(mainid.value());
    }
    if (asvc.set()) {
        buf.putUInt8(asvc.value());
    }
    if (substream1.set()) {
        buf.putUInt8(substream1.value());
    }
    if (substream2.set()) {
        buf.putUInt8(substream2.value());
    }
    if (substream3.set()) {
        buf.putUInt8(substream3.value());
    }
    buf.putBytes(additional_info);
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::DVBEnhancedAC3Descriptor::deserializePayload(PSIBuffer& buf)
{
    const bool component_type_flag = buf.getBit() != 0;
    const bool bsid_flag = buf.getBit() != 0;
    const bool mainid_flag = buf.getBit() != 0;
    const bool asvc_flag = buf.getBit() != 0;
    mixinfoexists = buf.getBit() != 0;
    const bool substream1_flag = buf.getBit() != 0;
    const bool substream2_flag = buf.getBit() != 0;
    const bool substream3_flag = buf.getBit() != 0;

    if (component_type_flag) {
        component_type = buf.getUInt8();
    }
    if (bsid_flag) {
        bsid = buf.getUInt8();
    }
    if (mainid_flag) {
        mainid = buf.getUInt8();
    }
    if (asvc_flag) {
        asvc = buf.getUInt8();
    }
    if (substream1_flag) {
        substream1 = buf.getUInt8();
    }
    if (substream2_flag) {
        substream2 = buf.getUInt8();
    }
    if (substream3_flag) {
        substream3 = buf.getUInt8();
    }
    buf.getByteBlock(additional_info, buf.remainingReadBytes());
}


//----------------------------------------------------------------------------
// Static method to display a descriptor.
//----------------------------------------------------------------------------

void ts::DVBEnhancedAC3Descriptor::DisplayDescriptor(TablesDisplay& disp, DID did, const uint8_t* data, size_t size, int indent, TID tid, PDS pds)
{
    const UString margin(indent, ' ');

    if (size >= 1) {
        uint8_t flags = data[0];
        data++; size--;

        if ((flags & 0x80) && size >= 1) { // component_type
            uint8_t type = data[0];
            data++; size--;
            disp << margin << "Component type: " << names::AC3ComponentType(type, names::FIRST) << std::endl;
        }
        if ((flags & 0x40) && size >= 1) { // bsid
            uint8_t bsid = data[0];
            data++; size--;
            disp << margin << UString::Format(u"AC-3 coding version: %d (0x%X)", {bsid, bsid}) << std::endl;
        }
        if ((flags & 0x20) && size >= 1) { // mainid
            uint8_t mainid = data[0];
            data++; size--;
            disp << margin << UString::Format(u"Main audio service id: %d (0x%X)", {mainid, mainid}) << std::endl;
        }
        if ((flags & 0x10) && size >= 1) { // asvc
            uint8_t asvc = data[0];
            data++; size--;
            disp << margin << UString::Format(u"Associated to: 0x%X", {asvc}) << std::endl;
        }
        if (flags & 0x08) {
            disp << margin << "Substream 0: Mixing control metadata" << std::endl;
        }
        if ((flags & 0x04) && size >= 1) { // substream1
            uint8_t type = data[0];
            data++; size--;
            disp << margin << "Substream 1: " << names::AC3ComponentType(type, names::FIRST) << std::endl;
        }
        if ((flags & 0x02) && size >= 1) { // substream2
            uint8_t type = data[0];
            data++; size--;
            disp << margin << "Substream 2: " << names::AC3ComponentType(type, names::FIRST) << std::endl;
        }
        if ((flags & 0x01) && size >= 1) { // substream3
            uint8_t type = data[0];
            data++; size--;
            disp << margin << "Substream 3: " << names::AC3ComponentType(type, names::FIRST) << std::endl;
        }
        disp.displayPrivateData(u"Additional information", data, size, margin);
    }
    else {
        disp.displayExtraData(data, size, margin);
    }
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::DVBEnhancedAC3Descriptor::buildXML(DuckContext& duck, xml::Element* root) const
{
    root->setBoolAttribute(u"mixinfoexists", mixinfoexists);
    root->setOptionalIntAttribute(u"component_type", component_type, true);
    root->setOptionalIntAttribute(u"bsid", bsid, true);
    root->setOptionalIntAttribute(u"mainid", mainid, true);
    root->setOptionalIntAttribute(u"asvc", asvc, true);
    root->setOptionalIntAttribute(u"substream1", substream1, true);
    root->setOptionalIntAttribute(u"substream2", substream2, true);
    root->setOptionalIntAttribute(u"substream3", substream3, true);
    root->addHexaTextChild(u"additional_info", additional_info, true);
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

bool ts::DVBEnhancedAC3Descriptor::analyzeXML(DuckContext& duck, const xml::Element* element)
{
    return  element->getBoolAttribute(mixinfoexists, u"mixinfoexists", true) &&
            element->getOptionalIntAttribute(component_type, u"component_type") &&
            element->getOptionalIntAttribute(bsid, u"bsid") &&
            element->getOptionalIntAttribute(mainid, u"mainid") &&
            element->getOptionalIntAttribute(asvc, u"asvc") &&
            element->getOptionalIntAttribute(substream1, u"substream1") &&
            element->getOptionalIntAttribute(substream2, u"substream2") &&
            element->getOptionalIntAttribute(substream3, u"substream3") &&
            element->getHexaTextChild(additional_info, u"additional_info", false, 0, MAX_DESCRIPTOR_SIZE - 8);
}
