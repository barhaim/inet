//
// Copyright (C) 2014 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/mobility/contract/IMobility.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMTransmitter.h"
#include "inet/physicallayer/common/layered/SignalPacketModel.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMPLCPFrame_m.h"
#include "inet/physicallayer/contract/layered/ISignalAnalogModel.h"
#include "inet/physicallayer/analogmodel/layered/ScalarSignalAnalogModel.h"
#include "inet/physicallayer/ieee80211/Ieee80211OFDMModulation.h"
#include "inet/physicallayer/ieee80211/Ieee80211OFDMCode.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211ConvolutionalCode.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMEncoder.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMEncoderModule.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMModulator.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211Serializer.h"
#include "inet/physicallayer/common/layered/LayeredTransmission.h"
#include "inet/common/serializer/headerserializers/ieee80211/Ieee80211PhySerializer.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMDefs.h"
#include "inet/physicallayer/ieee80211/layered/Ieee80211OFDMSymbolModel.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211OFDMTransmitter);

using namespace serializer;

void Ieee80211OFDMTransmitter::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        isCompliant = par("isCompliant").boolValue();
        dataEncoder = dynamic_cast<const IEncoder *>(getSubmodule("encoder"));
        signalEncoder = dynamic_cast<const IEncoder *>(getSubmodule("signalEncoder"));
        dataModulator = dynamic_cast<const IModulator *>(getSubmodule("modulator"));
        signalModulator = dynamic_cast<const IModulator *>(getSubmodule("signalModulator"));
        pulseShaper = dynamic_cast<const IPulseShaper *>(getSubmodule("pulseShaper"));
        digitalAnalogConverter = dynamic_cast<const IDigitalAnalogConverter *>(getSubmodule("digitalAnalogConverter"));
        channelSpacing = Hz(par("channelSpacing"));
        power = W(par("power"));
        Hz bandwidth = Hz(par("bandwidth"));
        carrierFrequency = Hz(par("carrierFrequency"));
        if (isCompliant && (dataEncoder || signalEncoder || dataModulator || signalModulator
                            || pulseShaper || digitalAnalogConverter || !isNaN(channelSpacing.get() || !isNaN(bandwidth.get())))) // TODO: check modulations
        {
            throw cRuntimeError("In compliant mode it is forbidden to set the following parameters: dataEncoder, signalEncoder, modulator, signalModulator, pulseShaper, digitalAnalogConverter, bandwidth, channelSpacing");
        }
        const char *levelOfDetailStr = par("levelOfDetail").stringValue();
        if (strcmp("bit", levelOfDetailStr) == 0)
            levelOfDetail = BIT_DOMAIN;
        else if (strcmp("symbol", levelOfDetailStr) == 0)
            levelOfDetail = SYMBOL_DOMAIN;
        else if (strcmp("sample", levelOfDetailStr) == 0)
            levelOfDetail = SAMPLE_DOMAIN;
        else if (strcmp("packet", levelOfDetailStr) == 0)
            levelOfDetail = PACKET_DOMAIN;
        else
            throw cRuntimeError("Unknown level of detail='%s'", levelOfDetailStr);
        if (!isCompliant)
        {
            // TODO: factor?
            const APSKModulationBase *signalModulation = APSKModulationBase::findModulation(par("signalModulation"));
            const APSKModulationBase *dataModulation = APSKModulationBase::findModulation(par("dataModulation"));
            const Ieee80211OFDMEncoderModule *ofdmSignalEncoderModule = check_and_cast<const Ieee80211OFDMEncoderModule *>(signalEncoder);
            const Ieee80211OFDMEncoderModule *ofdmDataEncoderModule = check_and_cast<const Ieee80211OFDMEncoderModule *>(dataEncoder);
            const Ieee80211OFDMSignalMode *signalMode = nullptr;
            const Ieee80211OFDMDataMode *dataMode = nullptr;
            if (levelOfDetail >= BIT_DOMAIN)
            {
                signalMode = new Ieee80211OFDMSignalMode(ofdmSignalEncoderModule->getCode(), new Ieee80211OFDMModulation(signalModulation), channelSpacing, bandwidth, 0);
                dataMode = new Ieee80211OFDMDataMode(ofdmDataEncoderModule->getCode(), new Ieee80211OFDMModulation(dataModulation), channelSpacing, bandwidth);
            }
            if (levelOfDetail < BIT_DOMAIN)
            {
                signalMode = new Ieee80211OFDMSignalMode(nullptr, new Ieee80211OFDMModulation(signalModulation), channelSpacing, bandwidth, 0);
                dataMode = new Ieee80211OFDMDataMode(nullptr, new Ieee80211OFDMModulation(dataModulation), channelSpacing, bandwidth);
            }
            mode = new Ieee80211OFDMMode(new Ieee80211OFDMPreambleMode(channelSpacing, bandwidth), signalMode, dataMode, channelSpacing, bandwidth);
        }
    }
}

BitVector *Ieee80211OFDMTransmitter::serialize(const cPacket* packet) const
{
    Ieee80211PhySerializer phySerializer;
    BitVector *serializedPacket = new BitVector();
    const Ieee80211OFDMPLCPFrame *phyFrame = check_and_cast<const Ieee80211OFDMPLCPFrame*>(packet);
    phySerializer.serialize(phyFrame, serializedPacket);
    unsigned int byteLength = phyFrame->getLength();
    unsigned int rate = phyFrame->getRate();
    int dataBitsLength = 6 + byteLength * 8 + 16;
    padding(serializedPacket, dataBitsLength, rate);
    return serializedPacket;
}

const ITransmissionPacketModel* Ieee80211OFDMTransmitter::createPacketModel(const cPacket* macFrame) const
{
    // The PLCP header is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1),
    // Tail (6) and SERVICE (16) fields.
    int plcpHeaderLength = 4 + 1 + 12 + 1 + 6 + 16;
    Ieee80211OFDMPLCPFrame * phyFrame = new Ieee80211OFDMPLCPFrame();
    phyFrame->setRate(mode->getSignalMode()->getRate());
    phyFrame->setLength(macFrame->getByteLength());
    phyFrame->encapsulate(const_cast<cPacket *>(macFrame));
    phyFrame->setBitLength(phyFrame->getLength()*8 + plcpHeaderLength);
    BitVector *serializedPacket = serialize(phyFrame);
    return new TransmissionPacketModel(phyFrame, serializedPacket, mode->getDataMode()->getNetBitrate());
}

const ITransmissionAnalogModel* Ieee80211OFDMTransmitter::createScalarAnalogModel(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel *bitModel) const
{
    int headerBitLength = -1;
    int dataBitLength = -1;
    if (levelOfDetail > PACKET_DOMAIN)
    {
        headerBitLength = bitModel->getHeaderBitLength();
        dataBitLength = bitModel->getPayloadBitLength();
    }
    else
    {
        if (isCompliant)
        {
            const ConvolutionalCode *convolutionalCode = mode->getDataMode()->getCode()->getConvolutionalCode();
            headerBitLength = ENCODED_SIGNAL_FIELD_LENGTH;
            dataBitLength = convolutionalCode->getEncodedLength((packetModel->getSerializedPacket()->getSize() - DECODED_SIGNAL_FIELD_LENGTH));
        }
        else
        {
            throw cRuntimeError("Unimplemented");
        }
    }
    simtime_t preambleDuration = mode->getPreambleMode()->getDuration();
    simtime_t headerDuration = 0;
    if (!isCompliant)
    {
        unsigned int headerCodeWordSize = mode->getSignalMode()->getModulation()->getModulation()->getCodeWordSize();
        ASSERT(headerBitLength % headerCodeWordSize == 0);
        unsigned int numberOfSignalAPSKSymbols = headerBitLength / headerCodeWordSize;
        unsigned int numberOfSignalOFDMSymbols = numberOfSignalAPSKSymbols / NUMBER_OF_OFDM_DATA_SUBCARRIERS;
        headerDuration = numberOfSignalOFDMSymbols * mode->getSymbolInterval();
    }
    else
        headerDuration = mode->getSignalMode()->getDuration();
    unsigned int dataCodeWordSize = mode->getDataMode()->getModulation()->getModulation()->getCodeWordSize();
    ASSERT(dataBitLength % dataCodeWordSize == 0);
    unsigned int numberOfDataAPSKSymbols = dataBitLength / dataCodeWordSize;
    unsigned int numberOfDataOFDMSymbols = numberOfDataAPSKSymbols / NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    simtime_t dataDuration = numberOfDataOFDMSymbols * mode->getSymbolInterval();
    simtime_t duration = preambleDuration + headerDuration + dataDuration;
    return new ScalarTransmissionSignalAnalogModel(duration, carrierFrequency, mode->getBandwidth(), power);
}

const ITransmissionPacketModel* Ieee80211OFDMTransmitter::createSignalFieldPacketModel(const ITransmissionPacketModel* completePacketModel) const
{
    // The SIGNAL field is composed of RATE (4), Reserved (1), LENGTH (12), Parity (1), Tail (6),
    // fields, so the SIGNAL field is 24 bits (OFDM_SYMBOL_SIZE / 2) long.
    BitVector *signalField = new BitVector();
    const BitVector *serializedPacket = completePacketModel->getSerializedPacket();
    for (unsigned int i = 0; i < NUMBER_OF_OFDM_DATA_SUBCARRIERS / 2; i++)
        signalField->appendBit(serializedPacket->getBit(i));
    return new TransmissionPacketModel(nullptr, signalField, bps(NaN));
}

const ITransmissionPacketModel* Ieee80211OFDMTransmitter::createDataFieldPacketModel(const ITransmissionPacketModel* completePacketModel) const
{
    BitVector *dataField = new BitVector();
    const BitVector *serializedPacket = completePacketModel->getSerializedPacket();
    for (unsigned int i = NUMBER_OF_OFDM_DATA_SUBCARRIERS / 2; i < serializedPacket->getSize(); i++)
        dataField->appendBit(serializedPacket->getBit(i));
    return new TransmissionPacketModel(nullptr, dataField, bps(NaN));
}

void Ieee80211OFDMTransmitter::encodeAndModulate(const ITransmissionPacketModel* packetModel, const ITransmissionBitModel *&fieldBitModel, const ITransmissionSymbolModel *&fieldSymbolModel, const IEncoder *encoder, const IModulator *modulator, bool isSignalField) const
{
    const ITransmissionPacketModel *fieldPacketModel = nullptr;
    if (isSignalField)
        fieldPacketModel = createSignalFieldPacketModel(packetModel);
    else
        fieldPacketModel = createDataFieldPacketModel(packetModel);
    if (levelOfDetail >= BIT_DOMAIN)
    {
        if (fieldPacketModel)
        {
            if (encoder) // non-compliant mode
                fieldBitModel = encoder->encode(fieldPacketModel);
            else // compliant mode
            {
                const Ieee80211OFDMCode *code = nullptr;
                if (isSignalField)
                    code = mode->getSignalMode()->getCode();
                else // data
                    code = mode->getDataMode()->getCode();
                const Ieee80211OFDMEncoder encoder(code);
                fieldBitModel = encoder.encode(fieldPacketModel);
            }
        }
        else
            throw cRuntimeError("Encoder needs packet representation");
    }
    if (levelOfDetail >= SYMBOL_DOMAIN)
    {
        if (fieldBitModel)
        {
            if (modulator) // non-compliant mode
                fieldSymbolModel = modulator->modulate(fieldBitModel);
            else // compliant mode
            {
                const Ieee80211OFDMModulation *ofdmModulation;
                if (isSignalField)
                    ofdmModulation = mode->getSignalMode()->getModulation();
                else
                    ofdmModulation = mode->getDataMode()->getModulation();
                Ieee80211OFDMModulator modulator(ofdmModulation);
                fieldSymbolModel = modulator.modulate(fieldBitModel);
            }
        }
        else
            throw cRuntimeError("Modulator needs bit representation");
    }
    delete fieldPacketModel;
}

const ITransmissionSymbolModel* Ieee80211OFDMTransmitter::createSymbolModel(const ITransmissionSymbolModel* signalFieldSymbolModel, const ITransmissionSymbolModel* dataFieldSymbolModel) const
{
    if (levelOfDetail >= SYMBOL_DOMAIN)
    {
        const std::vector<const ISymbol *> *signalSymbols = signalFieldSymbolModel->getSymbols();
        std::vector<const ISymbol *> *mergedSymbols = new std::vector<const ISymbol *>();
        const Ieee80211OFDMSymbol *ofdmSymbol = nullptr;
        for (unsigned int i = 0; i < signalSymbols->size(); i++)
        {
            ofdmSymbol = dynamic_cast<const Ieee80211OFDMSymbol *>(signalSymbols->at(i));
            mergedSymbols->push_back(new Ieee80211OFDMSymbol(*ofdmSymbol));
        }
        const std::vector<const ISymbol *> *dataSymbols = dataFieldSymbolModel->getSymbols();
        for (unsigned int i = 0; i < dataSymbols->size(); i++)
        {
            ofdmSymbol = dynamic_cast<const Ieee80211OFDMSymbol *>(dataSymbols->at(i));
            mergedSymbols->push_back(new Ieee80211OFDMSymbol(*ofdmSymbol));
        }
        const Ieee80211OFDMTransmissionSymbolModel *transmissionSymbolModel = new Ieee80211OFDMTransmissionSymbolModel(1, 1.0 / mode->getSignalMode()->getDuration(), mergedSymbols->size() - 1, 1.0 / mode->getSymbolInterval(), mergedSymbols, signalFieldSymbolModel->getHeaderModulation(), dataFieldSymbolModel->getPayloadModulation());
        delete signalFieldSymbolModel;
        delete dataFieldSymbolModel;
        return transmissionSymbolModel;
    }
    return new Ieee80211OFDMTransmissionSymbolModel(-1, NaN, -1, NaN, nullptr, mode->getSignalMode()->getModulation()->getModulation(), mode->getDataMode()->getModulation()->getModulation());
}

const ITransmissionBitModel* Ieee80211OFDMTransmitter::createBitModel(const ITransmissionBitModel* signalFieldBitModel, const ITransmissionBitModel* dataFieldBitModel, const ITransmissionPacketModel *packetModel) const
{
    if (levelOfDetail >= BIT_DOMAIN)
    {
        BitVector *encodedBits = new BitVector(*signalFieldBitModel->getBits());
        unsigned int signalBitLength = signalFieldBitModel->getBits()->getSize();
        const BitVector *dataFieldBits = dataFieldBitModel->getBits();
        unsigned int dataBitLength = dataFieldBits->getSize();
        for (unsigned int i = 0; i < dataFieldBits->getSize(); i++)
            encodedBits->appendBit(dataFieldBits->getBit(i));
        const TransmissionBitModel *transmissionBitModel = new TransmissionBitModel(signalBitLength, mode->getSignalMode()->getGrossBitrate(), dataBitLength, mode->getDataMode()->getGrossBitrate(), encodedBits, dataFieldBitModel->getForwardErrorCorrection(), dataFieldBitModel->getScrambling(), dataFieldBitModel->getInterleaving());
        delete signalFieldBitModel;
        delete dataFieldBitModel;
        return transmissionBitModel;
    }
    return new TransmissionBitModel(-1, mode->getSignalMode()->getGrossBitrate(), -1, mode->getDataMode()->getGrossBitrate(), nullptr, nullptr, nullptr, nullptr);
}

void Ieee80211OFDMTransmitter::padding(BitVector* serializedPacket, unsigned int dataBitsLength, uint8_t rate) const
{
    // TODO: in non-compliant mode: header padding.
    unsigned int codedBitsPerOFDMSymbol = mode->getDataMode()->getModulation()->getModulation()->getCodeWordSize() * NUMBER_OF_OFDM_DATA_SUBCARRIERS;
    const Ieee80211OFDMCode *code = mode->getDataMode()->getCode();
    unsigned int dataBitsPerOFDMSymbol = codedBitsPerOFDMSymbol;
    if (code && code->getConvolutionalCode())
    {
        const ConvolutionalCode *convolutionalCode = code->getConvolutionalCode();
        dataBitsPerOFDMSymbol = convolutionalCode->getDecodedLength(codedBitsPerOFDMSymbol);
    }
    unsigned int appendedBitsLength = dataBitsPerOFDMSymbol - dataBitsLength % dataBitsPerOFDMSymbol;
    serializedPacket->appendBit(0, appendedBitsLength);
}

const ITransmissionSampleModel* Ieee80211OFDMTransmitter::createSampleModel(const ITransmissionSymbolModel *symbolModel) const
{
    if (levelOfDetail >= SAMPLE_DOMAIN)
    {
        throw cRuntimeError("This level of detail is unimplemented.");
//        if (symbolModel)
//        {
//            if (pulseShaper) // non-compliant mode
//                sampleModel = pulseShaper->shape(symbolModel);
//            else // compliant mode
//            {
//            }
//        }
//        else
//            throw cRuntimeError("Pulse shaper needs symbol representation");
    }
    else
        return nullptr;
}

const ITransmissionAnalogModel* Ieee80211OFDMTransmitter::createAnalogModel(const ITransmissionPacketModel *packetModel, const ITransmissionBitModel* bitModel, const ITransmissionSymbolModel* symbolModel, const ITransmissionSampleModel* sampleModel) const
{
    const ITransmissionAnalogModel *analogModel = nullptr;
    if (digitalAnalogConverter)
    {
        if (!sampleModel)
            analogModel = digitalAnalogConverter->convertDigitalToAnalog(sampleModel);
        else
            throw cRuntimeError("Digital/analog converter needs sample representation");
    }
    else // TODO: Analog model is obligatory, currently we use scalar analog model as default analog model
        analogModel = createScalarAnalogModel(packetModel, bitModel);
    return analogModel;
}

const ITransmission *Ieee80211OFDMTransmitter::createTransmission(const IRadio *transmitter, const cPacket *macFrame, const simtime_t startTime) const
{
    if (isCompliant)
    {
        // TODO: get mode from the MAC
        // Kludge
        mode = &Ieee80211OFDMCompliantModes::getCompliantMode(13, MHz(20));
    }
    const ITransmissionBitModel *bitModel = nullptr;
    const ITransmissionBitModel *signalFieldBitModel = nullptr;
    const ITransmissionBitModel *dataFieldBitModel = nullptr;
    const ITransmissionSymbolModel *symbolModel = nullptr;
    const ITransmissionSymbolModel *signalFieldSymbolModel = nullptr;
    const ITransmissionSymbolModel *dataFieldSymbolModel = nullptr;
    const ITransmissionSampleModel *sampleModel = nullptr;
    const ITransmissionAnalogModel *analogModel = nullptr;
    const ITransmissionPacketModel *packetModel = createPacketModel(macFrame);
    encodeAndModulate(packetModel, signalFieldBitModel, signalFieldSymbolModel, signalEncoder, signalModulator, true);
    encodeAndModulate(packetModel, dataFieldBitModel, dataFieldSymbolModel, dataEncoder, dataModulator, false);
    bitModel = createBitModel(signalFieldBitModel, dataFieldBitModel, packetModel);
    symbolModel = createSymbolModel(signalFieldSymbolModel, dataFieldSymbolModel);
    sampleModel = createSampleModel(symbolModel);
    analogModel = createAnalogModel(packetModel, bitModel, symbolModel, sampleModel);
    IMobility *mobility = transmitter->getAntenna()->getMobility();
    // assuming movement and rotation during transmission is negligible
    const simtime_t endTime = startTime + analogModel->getDuration();
    const Coord startPosition = mobility->getCurrentPosition();
    const Coord endPosition = mobility->getCurrentPosition();
    const EulerAngles startOrientation = mobility->getCurrentAngularPosition();
    const EulerAngles endOrientation = mobility->getCurrentAngularPosition();
    return new LayeredTransmission(packetModel, bitModel, symbolModel, sampleModel, analogModel, transmitter, macFrame, startTime, endTime, startPosition, endPosition, startOrientation, endOrientation);
}

Ieee80211OFDMTransmitter::~Ieee80211OFDMTransmitter()
{
    if (!isCompliant)
    {
        delete mode->getDataMode()->getModulation();
        delete mode->getDataMode()->getCode();
        delete mode->getSignalMode()->getModulation();
        delete mode->getSignalMode()->getCode();
        delete mode->getPreambleMode();
        delete mode->getSignalMode();
        delete mode->getDataMode();
        delete mode;
    }
}

} // namespace physicallayer

} // namespace inet