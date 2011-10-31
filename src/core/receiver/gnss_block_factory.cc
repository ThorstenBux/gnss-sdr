/*!
 * \file gnss_block_factory.cc
 * \brief  This class implements a factory that returns instances of GNSS blocks.
 * \author Carlos Aviles, 2010. carlos.avilesr(at)googlemail.com
 *         Luis Esteve, 2011. luis(at)epsilon-formacion.com
 *
 * This class encapsulates the complexity behind the instantiation
 * of GNSS blocks.
 *
 * -------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2011  (see AUTHORS file for a list of contributors)
 *
 * GNSS-SDR is a software defined Global Navigation
 *          Satellite Systems receiver
 *
 * This file is part of GNSS-SDR.
 *
 * GNSS-SDR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * at your option) any later version.
 *
 * GNSS-SDR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNSS-SDR. If not, see <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------
 */

#include "gnss_block_factory.h"

#include <string>
#include <sstream>
#include <iostream>
#include <boost/lexical_cast.hpp>

#include <glog/log_severity.h>
#include <glog/logging.h>

#include "configuration_interface.h"
#include "gnss_block_interface.h"
#include "pass_through.h"
#include "file_signal_source.h"
#include "null_sink_output_filter.h"
#include "file_output_filter.h"
#include "channel.h"
#include "usrp1_signal_source.h"
#include "direct_resampler_conditioner.h"
#include "gps_l1_ca_gps_sdr_acquisition.h"
#include "gps_l1_ca_pcps_acquisition.h"
#include "gps_l1_ca_tong_pcps_acquisition.h"
#include "gps_l1_ca_dll_pll_tracking.h"
#include "gps_l1_ca_telemetry_decoder.h"
#include "gps_l1_ca_observables.h"

using google::LogMessage;

GNSSBlockFactory::GNSSBlockFactory()
{
}

GNSSBlockFactory::~GNSSBlockFactory()
{
}

GNSSBlockInterface* GNSSBlockFactory::GetSignalSource(
        ConfigurationInterface *configuration, gr_msg_queue_sptr queue)
{

    std::string default_implementation = "File_Signal_Source";
    std::string implementation = configuration->property(
            "SignalSource.implementation", default_implementation);

    DLOG(INFO) << "Getting SignalSource with implementation "
            << implementation;

    return GetBlock(configuration, "SignalSource", implementation, 0, 1,
            queue);
}

GNSSBlockInterface* GNSSBlockFactory::GetSignalConditioner(
        ConfigurationInterface *configuration, gr_msg_queue_sptr queue)
{

    std::string default_implementation = "Pass_Through";
    std::string implementation = configuration->property(
            "SignalConditioner.implementation", default_implementation);

    DLOG(INFO) << "Getting SignalConditioner with implementation "
            << implementation;

    return GetBlock(configuration, "SignalConditioner", implementation, 1, 1,
            queue);
}

GNSSBlockInterface* GNSSBlockFactory::GetObservables(
        ConfigurationInterface *configuration, gr_msg_queue_sptr queue)
{

    std::string default_implementation = "GPS_L1_CA_Observables";
    std::string implementation = configuration->property(
            "Observables.implementation", default_implementation);

    DLOG(INFO) << "Getting Observables with implementation "
            << implementation;

    unsigned int channel_count =
            configuration->property("Channels.count", 12);

    return GetBlock(configuration, "Observables", implementation,
            channel_count, channel_count, queue);
}

GNSSBlockInterface* GNSSBlockFactory::GetPVT(
        ConfigurationInterface *configuration, gr_msg_queue_sptr queue)
{

    std::string default_implementation = "Pass_Through";
    std::string implementation = configuration->property(
            "PVT.implementation", default_implementation);

    DLOG(INFO) << "Getting PVT with implementation " << implementation;

    unsigned int channel_count =
            configuration->property("Channels.count", 12);

    return GetBlock(configuration, "PVT", implementation, channel_count, 1,
            queue);
}

GNSSBlockInterface* GNSSBlockFactory::GetOutputFilter(
        ConfigurationInterface *configuration, gr_msg_queue_sptr queue)
{

    std::string default_implementation = "Null_Sink_Output_Filter";
    std::string implementation = configuration->property(
            "OutputFilter.implementation", default_implementation);

    DLOG(INFO) << "Getting OutputFilter with implementation "
            << implementation;

    return GetBlock(configuration, "OutputFilter", implementation, 1, 0,
            queue);
}

GNSSBlockInterface* GNSSBlockFactory::GetChannel(
        ConfigurationInterface *configuration, std::string acq,
        std::string trk, std::string tlm, int channel,
        gr_msg_queue_sptr queue)
{

    std::stringstream stream;
    stream << channel;
    std::string id = stream.str();

    DLOG(INFO) << "Instantiating channel " << id;

    return new Channel(configuration, channel, GetBlock(configuration,
            "SignalConditioner", "Pass_Through", 1, 1, queue),
            (AcquisitionInterface*)GetBlock(configuration, "Acquisition",
                    acq, 1, 1, queue), (TrackingInterface*)GetBlock(
                    configuration, "Tracking", trk, 1, 1, queue),
            (TelemetryDecoderInterface*)GetBlock(configuration,
                    "TelemetryDecoder", tlm, 1, 1, queue), "Channel",
            "Channel", queue);
}

std::vector<GNSSBlockInterface*>* GNSSBlockFactory::GetChannels(
        ConfigurationInterface *configuration, gr_msg_queue_sptr queue)
{

    std::string default_implementation = "PassThrough";
    unsigned int channel_count =
            configuration->property("Channels.count", 12);
    std::vector<GNSSBlockInterface*>* channels = new std::vector<
            GNSSBlockInterface*>();

    std::string tracking = configuration->property("Tracking.implementation",
            default_implementation);
    std::string telemetry_decoder = configuration->property(
            "TelemetryDecoder.implementation", default_implementation);
    for (unsigned int i = 0; i < channel_count; i++)
    {

        std::string acquisition_implementation_name = "Acquisition"
                + boost::lexical_cast<std::string>(i) + ".implementation";
        std::string acquisition_implementation = configuration->property(
                acquisition_implementation_name, default_implementation);
        channels->push_back(GetChannel(configuration,
                acquisition_implementation, tracking, telemetry_decoder, i,
                queue));
        std::cout << "getchannel_" << i << ", acq_implementation_name: "
                << acquisition_implementation_name << ", implementation: "
                << acquisition_implementation << std::endl;

    }

    DLOG(INFO) << "Getting " << channel_count << " channels";
    return channels;

}

GNSSBlockInterface* GNSSBlockFactory::GetBlock(
        ConfigurationInterface *configuration, std::string role,
        std::string implementation, unsigned int in_streams,
        unsigned int out_streams, gr_msg_queue_sptr queue)
{

    GNSSBlockInterface* block = NULL;

    if (implementation.compare("File_Signal_Source") == 0)
    {
        block = new FileSignalSource(configuration, role, in_streams,
                out_streams, queue);
    }
    else if (implementation.compare("Pass_Through") == 0)
    {
        block = new PassThrough(configuration, role, in_streams, out_streams);
    }
    else if (implementation.compare("Null_Sink_Output_Filter") == 0)
    {
        block = new NullSinkOutputFilter(configuration, role, in_streams,
                out_streams);
    }
    else if (implementation.compare("File_Output_Filter") == 0)
    {
        block = new FileOutputFilter(configuration, role, in_streams,
                out_streams);
    }
    else if (implementation.compare("USRP1_Signal_Source") == 0)
    {
        block = new Usrp1SignalSource(configuration, role, in_streams,
                out_streams, queue);
    }
    else if (implementation.compare("Direct_Resampler") == 0)
    {
        block = new DirectResamplerConditioner(configuration, role,
                in_streams, out_streams);
    }
    else if (implementation.compare("GPS_L1_CA_GPS_SDR_Acquisition") == 0)
    {
        block = new GpsL1CaGpsSdrAcquisition(configuration, role, in_streams,
                out_streams, queue);
    }
    else if (implementation.compare("GPS_L1_CA_PCPS_Acquisition") == 0)
    {
        block = new GpsL1CaPcpsAcquisition(configuration, role, in_streams,
                out_streams, queue);
    }
    else if (implementation.compare("GPS_L1_CA_TONG_PCPS_Acquisition") == 0)
    {
        block = new GpsL1CaTongPcpsAcquisition(configuration, role,
                in_streams, out_streams, queue);
    }
    else if (implementation.compare("GPS_L1_CA_DLL_PLL_Tracking") == 0)
    {
        block = new GpsL1CaDllPllTracking(configuration, role, in_streams,
                out_streams, queue);
    }
    else if (implementation.compare("GPS_L1_CA_Telemetry_Decoder") == 0)
    {
        block = new GpsL1CaTelemetryDecoder(configuration, role, in_streams,
                out_streams, queue);
    }
    else if (implementation.compare("GPS_L1_CA_Observables") == 0)
    {
        block = new GpsL1CaObservables(configuration, role, in_streams,
                out_streams, queue);
    }
    else
    {
        // Log fatal. This causes execution to stop.
        LOG_AT_LEVEL(ERROR) << implementation
                << ": Undefined implementation for block";
    }

    return block;
}