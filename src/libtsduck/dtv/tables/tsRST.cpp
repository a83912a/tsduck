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

#include "tsRST.h"
#include "tsBinaryTable.h"
#include "tsTablesDisplay.h"
#include "tsPSIRepository.h"
#include "tsPSIBuffer.h"
#include "tsDuckContext.h"
#include "tsxmlElement.h"
TSDUCK_SOURCE;

#define MY_XML_NAME u"RST"
#define MY_CLASS ts::RST
#define MY_TID ts::TID_RST
#define MY_PID ts::PID_RST
#define MY_STD ts::Standards::DVB

TS_REGISTER_TABLE(MY_CLASS, {MY_TID}, MY_STD, MY_XML_NAME, MY_CLASS::DisplaySection, nullptr, {MY_PID});


//----------------------------------------------------------------------------
// Definition of names for running status values.
//----------------------------------------------------------------------------

const ts::Enumeration ts::RST::RunningStatusNames({
    {u"undefined",   RS_UNDEFINED},
    {u"not-running", RS_NOT_RUNNING},
    {u"starting",    RS_STARTING},
    {u"pausing",     RS_PAUSING},
    {u"running",     RS_RUNNING},
    {u"off-air",     RS_OFF_AIR},
});


//----------------------------------------------------------------------------
// Constructors
//----------------------------------------------------------------------------

ts::RST::RST() :
    AbstractTable(MY_TID, MY_XML_NAME, MY_STD),
    events()
{
}

ts::RST::RST(DuckContext& duck, const BinaryTable& table) :
    RST()
{
    deserialize(duck, table);
}


//----------------------------------------------------------------------------
// Clear the content of the table.
//----------------------------------------------------------------------------

void ts::RST::clearContent()
{
    events.clear();
}


//----------------------------------------------------------------------------
// Deserialization
//----------------------------------------------------------------------------

void ts::RST::deserializePayload(PSIBuffer& buf, const Section& section)
{
    while (!buf.error() && !buf.endOfRead()) {
        Event event;
        event.transport_stream_id = buf.getUInt16();
        event.original_network_id = buf.getUInt16();
        event.service_id = buf.getUInt16();
        event.event_id = buf.getUInt16();
        buf.skipBits(5);
        event.running_status = buf.getBits<uint8_t>(3);
        events.push_back(event);
    }
}


//----------------------------------------------------------------------------
// Serialization
//----------------------------------------------------------------------------

void ts::RST::serializePayload(BinaryTable& table, PSIBuffer& payload) const
{
    for (auto it = events.begin(); it != events.end(); ++it) {
        payload.putUInt16(it->transport_stream_id);
        payload.putUInt16(it->original_network_id);
        payload.putUInt16(it->service_id);
        payload.putUInt16(it->event_id);
        payload.putBits(0xFF, 5);
        payload.putBits(it->running_status, 3);
    }
}


//----------------------------------------------------------------------------
// A static method to display a RST section.
//----------------------------------------------------------------------------

void ts::RST::DisplaySection(TablesDisplay& display, const ts::Section& section, int indent)
{
    DuckContext& duck(display.duck());
    std::ostream& strm(duck.out());
    const std::string margin(indent, ' ');
    PSIBuffer buf(duck, section.payload(), section.payloadSize());

    while (!buf.error() && buf.remainingReadBytes() >= 9) {
        const uint16_t ts_id = buf.getUInt16();
        const uint16_t onet_id = buf.getUInt16();
        const uint16_t srv_id = buf.getUInt16();
        const uint16_t ev_id = buf.getUInt16();
        buf.skipBits(5);
        const uint8_t rs = buf.getBits<uint8_t>(3);

        strm << margin
             << UString::Format(u"TS: %d (0x%<X), Orig. Netw.: %d (0x%<X), Service: %d (0x%<X), Event: %d (0x%<X), Status: %s",
                                {ts_id, onet_id, srv_id, ev_id, RunningStatusNames.name(rs)})
             << std::endl;
    }

    display.displayExtraData(buf, indent);
}


//----------------------------------------------------------------------------
// XML serialization
//----------------------------------------------------------------------------

void ts::RST::buildXML(DuckContext& duck, xml::Element* root) const
{
    for (EventList::const_iterator it = events.begin(); it != events.end(); ++it) {
        xml::Element* e = root->addElement(u"event");
        e->setIntAttribute(u"transport_stream_id", it->transport_stream_id, true);
        e->setIntAttribute(u"original_network_id", it->original_network_id, true);
        e->setIntAttribute(u"service_id", it->service_id, true);
        e->setIntAttribute(u"event_id", it->event_id, true);
        e->setEnumAttribute(RunningStatusNames, u"running_status", it->running_status);
    }
}


//----------------------------------------------------------------------------
// XML deserialization
//----------------------------------------------------------------------------

bool ts::RST::analyzeXML(DuckContext& duck, const xml::Element* element)
{
    xml::ElementVector children;
    bool ok = element->getChildren(children, u"event");

    for (size_t index = 0; ok && index < children.size(); ++index) {
        Event event;
        ok = children[index]->getIntAttribute<uint16_t>(event.transport_stream_id, u"transport_stream_id", true, 0, 0x0000, 0xFFFF) &&
             children[index]->getIntAttribute<uint16_t>(event.original_network_id, u"original_network_id", true, 0, 0x0000, 0xFFFF) &&
             children[index]->getIntAttribute<uint16_t>(event.service_id, u"service_id", true, 0, 0x0000, 0xFFFF) &&
             children[index]->getIntAttribute<uint16_t>(event.event_id, u"event_id", true, 0, 0x0000, 0xFFFF) &&
             children[index]->getIntEnumAttribute<uint8_t>(event.running_status, RunningStatusNames, u"running_status", true);
        if (ok) {
            events.push_back(event);
        }
    }
    return ok;
}
